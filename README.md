# GLFont
This is an archived version of an old text rendering engine. The application takes a ttf file, creates a texture atlas, and manages OpenGL buffers to render the text, aligned on pixel boundaries. The project depends on FreeType to render per-glyph bitmaps and FreeImage to output debug images, as well as my [OpenGL wrapper](https://github.com/Airducky/GLWrap).

Strings to be displayed are stored in a list, and the OpenGL buffers are only changed when needed. The engine stores the earliest position in the list that requires modification, and updates to the vertex buffer only occur beginning from that position to avoid re-formatting vertex data for every string.

Every vertex is interpreted as a single character in the graphics pipleline, identified by an unsigned integer, and expanded by a geometry shader into a quad (triangle strip with four vertices). The "origin" used to place a character (and the entire string) refers to the font's origin for the glyph. The geometry shader then calculates a lower left corner for the rendered quad using the glyph's bearings and bitmap dimensions.

A sample render can be found in sample_render.png of a test application as it was captured in TestFrame.cpp. Two sample debug outputs font atlases are included for [Sax Mono](https://www.dafont.com/saxmono.font) and [Mecha](https://www.dafont.com/mecha-cf.font).
