#pragma once
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_OUTLINE_H

/*!
 * \class FontManager FontManager.h
 * \brief Handles loading fonts and renders them in OpenGL.
 *
 */
class FontManager
{
public:
	///Constructor that loads a font. Construction fails if loading fails.
	/*!
	 * Constructs a FontManager type which must have an associated font.
	 * If the requested font cannot be loaded, then the intended FontManager is invalid.
	 * \param[in] ftlib The FreeType library handle to use.
	 * \param[in] fontpath A file path to the font about to be loaded.
	 * \param[in] fontWidth The font size's width for this manager.
	 * \param[in] fontHeight The font size's height for this manager.
	 * \param[in] charbase the code point at which to start the atlas, inclusive
	 * \param[in] charpast the code point at which to end the atlas, exclusive
	 */
	FontManager(FT_Library ftlib, const char *fontpath, unsigned fontWidth, unsigned fontHeight, unsigned charbase, unsigned charpast);
	
	~FontManager();
	
	///Creates the texture atlas from characters in rangeBegin to rangeEnd
	/*!
	 * Greedy packing, begins by attempting a 128x128, and doubles size until every character fits.
	 */
	void bakeTextureAtlas();
	
	///Get the first code point included in the atlas
	/*!
	 * 
	 * \return The first code point included in the atlas
	 */
	unsigned charbase() const;
	
	///Get one past the last included code point
	/*!
	 *
	 * \return One past the last included code point
	 */
	unsigned charpast() const;
	
	///Get a pointer to the raw bitmap data
	/*!
	*
	* \return Const unsigned char pointer to bitmap data
	*/
	const unsigned char* raw() const;
	
	/*!
	 * \struct FontManager::CharInfo FontManager.h
	 * \brief Encapsulates information necessary to render each glyph
	 */
	struct CharInfo
	{
		int ax; ///< X advance
		int ay; ///< Y advance

		unsigned bw; ///< Bitmap width
		unsigned bh; ///< Bitmap rows

		int lb; ///< Left bearing
		int tb; ///< Top bearing
	};
	
	/*!
	 * \struct FontManager::AtlasMap FontManager.h
	 * \brief Stores the texture coordinates (in pixels) of the character in the baked texture atlas.
	 */
	struct AtlasMap
	{
		int lx; ///< Low X
		int ly; ///< Low Y
		int hx; ///< High X
		int hy; ///< High Y
	};
	
	const CharInfo* characterInfo() const;
	const AtlasMap* atlasMap() const;
	
	int mapWidth() const;
	int mapHeight() const;
	
private:
	FT_Face face; ///< FreeType handle for the font
	CharInfo *charinf;
	AtlasMap *map;
	unsigned rangeBegin, rangeEnd;
	unsigned char *bitmap;
	int width;
	int height;
	
	/*!
	 * \struct FontManager::metric FontManager.handle
	 * \breif Stores the total pixels taken up by the code point.
	 * Used to sort each glyph by the total number of pixels it takes in a bitmap.
	 * This is used as part of creating the texture atlas.
	 */
	struct Metric
	{
		unsigned area;
		unsigned code;
	};
	
	void generateMetrics(Metric *metrics);
	bool pack(Metric *metrics, int atlas_width, int atlas_height);
};