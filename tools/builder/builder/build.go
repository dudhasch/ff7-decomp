package builder

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/goccy/go-yaml"
	"github.com/xeeynamo/ff7-decomp/tools/builder/deps"
)

// Build compiles and sha1-checks the overlays named in `only`, or every overlay
// when `only` is empty. A partial build still writes a splat config for every
// overlay -- tools/ninja/gen.py reads all of them -- but check.sha1 lists only
// the selected ones, so ninja's `check` target pulls in nothing else and the
// overlays left out are never split, compiled or linked.
func Build(version string, only []string) error {
	data, _ := os.ReadFile(ConfigPath(version))
	var b BuildConfig
	if err := yaml.Unmarshal(data, &b); err != nil {
		panic(err)
	}
	selected, err := selectOverlays(b, only)
	if err != nil {
		return err
	}
	if err := writeObjdiffConfig(b); err != nil {
		return err
	}
	if err := os.MkdirAll(b.BuildPath, 0755); err != nil {
		return err
	}
	if err := writeSplatConfigs(b); err != nil {
		return err
	}
	if err := writeSha1Check(b, selected); err != nil {
		return err
	}
	if err := deps.GenNinja(b.BuildPath); err != nil {
		return err
	}
	if selected == nil {
		if err := deps.Ninja(); err != nil {
			return err
		}
	} else {
		// ninja's default target is "every output nothing else consumes",
		// which includes the .exe of an overlay dropped from check.sha1. Ask
		// for the check explicitly instead.
		if err := deps.Ninja("build/check.dummy"); err != nil {
			return err
		}
	}
	if err := generateExpected(); err != nil {
		return err
	}
	return nil
}

// selectOverlays turns the --overlays list into a set, or nil for "all".
// Selecting `main` degrades to a full build: main links against
// config/sym_ovl_export.us.txt, which is regenerated from the ELF of every
// other overlay, so there would be nothing left to skip.
func selectOverlays(b BuildConfig, only []string) (map[string]bool, error) {
	if len(only) == 0 {
		return nil, nil
	}
	known := map[string]bool{}
	names := make([]string, 0, len(b.Overlays))
	for _, o := range b.Overlays {
		known[o.Name] = true
		names = append(names, o.Name)
	}
	selected := map[string]bool{}
	for _, name := range only {
		name = strings.TrimSpace(name)
		if name == "" {
			continue
		}
		if !known[name] {
			return nil, fmt.Errorf("unknown overlay %q, known overlays are: %s",
				name, strings.Join(names, ", "))
		}
		if name == "main" {
			return nil, nil
		}
		selected[name] = true
	}
	if len(selected) == 0 {
		return nil, nil
	}
	return selected, nil
}

func writeSplatConfigs(b BuildConfig) error {
	for _, o := range b.Overlays {
		expectedFingerprint := o.Fingerprint()
		actualFingerprint, _ := os.ReadFile(fmt.Sprintf("%s/%s.fingerprint", b.BuildPath, o.Name))
		if actualFingerprint != nil && bytes.Equal(expectedFingerprint, actualFingerprint) {
			continue
		}
		splatConfig, err := makeSplatConfig(b, o)
		if err != nil {
			return err
		}
		data, err := yaml.Marshal(&splatConfig)
		if err != nil {
			return err
		}
		if err := os.WriteFile(fmt.Sprintf("%s/%s.yaml", b.BuildPath, o.Name), data, 0644); err != nil {
			return err
		}
		if err := os.WriteFile(fmt.Sprintf("%s/%s.fingerprint", b.BuildPath, o.Name), expectedFingerprint, 0644); err != nil {
			return err
		}
	}
	return nil
}

func writeSha1Check(b BuildConfig, selected map[string]bool) error {
	var sb strings.Builder
	for _, o := range b.Overlays {
		if selected != nil && !selected[o.Name] {
			continue
		}
		sb.WriteString(fmt.Sprintf("%s  %s\n", o.Sha1, filepath.Join(b.BuildPath, o.Name+".exe")))
	}
	return os.WriteFile(filepath.Join(b.BuildPath, "check.sha1"), []byte(sb.String()), 0644)
}

func generateExpected() error {
	if err := os.MkdirAll("expected/build", 0o755); err != nil {
		return fmt.Errorf("mkdir: %w", err)
	}
	if err := os.RemoveAll("expected/build"); err != nil {
		return fmt.Errorf("remove: %w", err)
	}
	cmd := exec.Command("cp", "-r", "build", "expected/")
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("%s: %w", strings.Join(cmd.Args, " "), err)
	}
	return nil
}
