"""
Crawl ff7speedruns.com via MediaWiki API up to a given depth, save parsed HTML,
clean noisy elements and convert to Markdown.

Usage:
  python tools/ff7speedruns_crawl_convert.py --depth 4 --max-pages 1000

Outputs:
  - docs/FF7_speedruns_html/<title>.html  (parsed HTML from API)
  - docs/FF7_speedruns_md/<title>.md     (cleaned Markdown)

Requires: requests, beautifulsoup4, html2text
"""
import os
import time
import argparse
from collections import deque
import requests
from bs4 import BeautifulSoup
import html2text

API = 'https://ff7speedruns.com/api.php'
USER_AGENT = 'ff7-decomp-api-crawler/1.0'

BAD_CLASSES = [
    'toc', 'navbox', 'metadata', 'reference', 'references', 'mw-references-wrap',
    'mw-editsection', 'infobox', 'navbox', 'sidebar', 'stub', 'hatnote'
]

OUT_HTML = os.path.join(os.getcwd(), 'docs', 'FF7_speedruns_html')
OUT_MD = os.path.join(os.getcwd(), 'docs', 'FF7_speedruns_md')
os.makedirs(OUT_HTML, exist_ok=True)
os.makedirs(OUT_MD, exist_ok=True)

session = requests.Session()
session.headers.update({'User-Agent': USER_AGENT})


def sanitize_title(title):
    return title.replace('/', '_')


def save_file(path, content):
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)


def fetch_parsed_html(title):
    params = {
        'action': 'parse',
        'page': title,
        'prop': 'text',
        'format': 'json'
    }
    r = session.get(API, params=params, timeout=30)
    r.raise_for_status()
    j = r.json()
    if 'error' in j:
        raise RuntimeError(j['error'])
    return j.get('parse', {}).get('text', {}).get('*', '')


def get_links(title):
    links = []
    params = {
        'action': 'query',
        'prop': 'links',
        'titles': title,
        'pllimit': 'max',
        'format': 'json'
    }
    while True:
        r = session.get(API, params=params, timeout=30)
        r.raise_for_status()
        j = r.json()
        pages = j.get('query', {}).get('pages', {})
        for pid, page in pages.items():
            for l in page.get('links', []) if page.get('links') else []:
                links.append(l.get('title'))
        if 'continue' in j:
            params.update(j['continue'])
            continue
        break
    return links


def clean_html(html):
    soup = BeautifulSoup(html, 'html.parser')
    # remove unwanted classes and elements
    for cls in BAD_CLASSES:
        for el in soup.select('.' + cls):
            el.decompose()
    # remove edit section anchors
    for el in soup.select('.mw-editsection'):
        el.decompose()
    # remove tables used for navigation (navbox-like)
    for tbl in soup.find_all('table'):
        if 'navbox' in (tbl.get('class') or []):
            tbl.decompose()
    # remove comments
    for comment in soup.find_all(string=lambda text: isinstance(text, type(soup.Comment))):
        comment.extract()
    return str(soup)


def html_to_markdown(html):
    h = html2text.HTML2Text()
    h.ignore_links = False
    h.body_width = 0
    md = h.handle(html)
    # basic cleanup: remove multiple blank lines
    md = '\n'.join([line.rstrip() for line in md.splitlines()])
    while '\n\n\n' in md:
        md = md.replace('\n\n\n', '\n\n')
    return md


def crawl_and_convert(start_titles, depth, max_pages, sleep):
    seen = set()
    q = deque()
    for t in start_titles:
        q.append((t, 0))
    saved = 0
    while q and saved < max_pages:
        title, d = q.popleft()
        if title in seen:
            continue
        seen.add(title)
        try:
            html = fetch_parsed_html(title)
        except Exception as e:
            print('Parse error', title, e)
            continue
        cleaned = clean_html(html)
        # save HTML
        fn_html = sanitize_title(title) + '.html'
        path_html = os.path.join(OUT_HTML, fn_html)
        save_file(path_html, cleaned)
        # convert to markdown
        md = html_to_markdown(cleaned)
        fn_md = sanitize_title(title) + '.md'
        path_md = os.path.join(OUT_MD, fn_md)
        save_file(path_md, md)
        print('Saved', path_html, 'and', path_md)
        saved += 1
        if d < depth:
            try:
                links = get_links(title)
            except Exception as e:
                print('Links error', title, e)
                links = []
            for l in links:
                if l not in seen:
                    q.append((l, d+1))
        time.sleep(sleep)
    print('Done. saved=', saved)


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--depth', type=int, default=4)
    p.add_argument('--max-pages', type=int, default=1000)
    p.add_argument('--sleep', type=float, default=0.5)
    p.add_argument('--start', nargs='*', default=['Main_Page'])
    args = p.parse_args()
    crawl_and_convert(args.start, args.depth, args.max_pages, args.sleep)
