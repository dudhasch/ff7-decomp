A Degenerate Triangle is a triangle that has zero internal area, possibly only when projected onto a specific plane. They can result in glitchy effects and edge-cases in collision detection.

Currently, there are only known to be useful or interesting instances of degenerate triangles in the World Map module.

## Types of Degenerate Triangle

### Line Triangle

A line triangle has three collinear vertices.

On the World Map, the collision detection function results in Line Triangles having collision that extends infinitely past the vertices of the triangle along the line the vertices inhabit, only stopping at the chunk boundary.

### Point Triangle

## Uses of Degenerate Triangles in the World Map

### Point Triangles for Cloudsurfing

Triangle data that corresponds to a point triangle can be created and used for collision detection using [Stale Stored Triangle Abuse](/index.php/Stale_Stored_Triangle_Abuse "Stale Stored Triangle Abuse"), which allows the player to enter a Cloudsurf state. The two main ways to do so are to have the triangle's vertices have the same index (usually index zero) or such that the triangle's vertex indexes are higher than the number of vertices in the actual chunk, leading to the resolved vertex coordinates reading zeroes out-of-bounds.

### Line Triangle Loading Zones

Oftentimes, all of the structural polygons of an entrance area model are flagged as a loading zone and to run the entrance script on collision, including vertical walls of buildings or structures. On the X/Z collision plane, the triangles of these vertical walls resolve to line triangles whose collision detection extends past their vertices, so they can be triggered remotely, though doing so requires some precision and good height values.

It is known to be possible to hit the entrance to the Chocobo Ranch using a line triangle loading zone, but attempting to do so is only practical in a tool-assisted setting as it requires a perfect X-coordinate or Z-coordinate.

Load video

YouTube

YouTube might collect personal data. [Privacy Policy](https://www.youtube.com/howyoutubeworks/user-settings/privacy/)

ContinueDismiss

Despite the potential for large skips that the idea of being able to hit loading zones remotely seems to suggest, there are no known major uses for this technique. The City of the Ancients model has no degenerate triangles, for instance.