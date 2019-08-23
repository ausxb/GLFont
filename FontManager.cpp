#include "FontManager.h"
#include <algorithm>
#include <memory>
#include <iostream>

FontManager::FontManager(FT_Library ftlib, const char *fontpath, unsigned fontWidth, unsigned fontHeight, unsigned charbase, unsigned charpast)
	: charinf{ nullptr }, map{ nullptr }, rangeBegin{ charbase }, rangeEnd{ charpast }, bitmap{nullptr}
{
	if(FT_New_Face(ftlib, fontpath, 0, &face)) {
		//ERROR
	}
	
	FT_Set_Pixel_Sizes(face, fontWidth, fontHeight);
}

FontManager::~FontManager()
{
	FT_Done_Face(face);
	if(charinf) delete[] charinf;
	if(map) delete[] map;
	if(bitmap) delete[] bitmap;
}

void FontManager::bakeTextureAtlas()
{
	int atlas_width = 128;
	int atlas_height = 128;
	
	charinf = new CharInfo[rangeEnd - rangeBegin];
	map = new AtlasMap[rangeEnd - rangeBegin];
	
	Metric *metrics = new Metric[rangeEnd - rangeBegin];

	generateMetrics(metrics);
	
	//Reversed comparison for descending order
	std::sort(metrics, &metrics[rangeEnd - rangeBegin], [](const Metric &a, const Metric &b){ return a.area > b.area; });
	
	/*std::cout << "Sorted order: ";
	for(Metric *m = metrics; m < metrics + (rangeEnd - rangeBegin); m++)
		std::cout << static_cast<char>(m->code) << ' ';
	std::cout << std::endl;*/
	
	bool fit = false;
	
	while(!fit)
	{
		bitmap = new unsigned char[atlas_width * atlas_height];
		std::uninitialized_fill(bitmap, &bitmap[atlas_width * atlas_height], 0);

		fit = pack(metrics, atlas_width, atlas_height);
		
		if(!fit)
		{
			delete[] bitmap;
			atlas_width *= 2;
			atlas_height *= 2;
		}
		else
		{
			//Keep bitmap
			width = atlas_width;
			height = atlas_height;
			
			delete[] metrics;
			
			/*textureObject = new glwrap::Texture{GL_TEXTURE_2D, 1, GL_R8UI, width, height, 0};
			textureObject->store(0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, bitmap);*/
		}
	}
}

unsigned FontManager::charbase() const
{
	return rangeBegin;
}

unsigned FontManager::charpast() const
{
	return rangeEnd;
}

const unsigned char* FontManager::raw() const
{
	return bitmap;
}

const FontManager::CharInfo* FontManager::characterInfo() const
{
	return charinf;
}

const FontManager::AtlasMap* FontManager::atlasMap() const
{
	return map;
}

int FontManager::mapWidth() const
{
	return width;
}

int FontManager::mapHeight() const
{
	return height;
}

void FontManager::generateMetrics(Metric *metrics)
{
	FT_GlyphSlot g = face->glyph;
	
	for(int fc = rangeBegin; fc < rangeEnd; fc++) {
		int index = fc - rangeBegin;
		if(FT_Load_Char(face, fc, FT_LOAD_RENDER))
		{
			std::cerr << "Glyph metric load error" << std::endl;
			continue; //Exception instead
		}
		
		CharInfo *record = (charinf + index);
		
		record->ax = g->advance.x;
		record->ay = g->advance.y;
		record->bw = g->bitmap.width;
		record->bh = g->bitmap.rows;
		record->lb = g->bitmap_left;
		record->tb = g->bitmap_top;

		metrics[index].area = record->bw * record->bh;
		metrics[index].code = fc;
		
		/*std::cout << '\'' << static_cast<char>(fc) << '\'' << ':' << std::endl;
		std::cout << "\tAdvance x: " << record->ax << " Advance y: " << record->ay << std::endl
					<< "\tBitmap width: " << record->bw << " Bitmap height: " << record->bh << std::endl
					<< "\tBitmap left: " << record->lb << " Bitmap top: " << record->tb << std::endl
					<< "\tArea: " << metrics[index].area << std::endl;*/
	}
}

bool FontManager::pack(Metric *metrics, int atlas_width, int atlas_height)
{
	FT_GlyphSlot g = face->glyph;
	unsigned writeX = 0; //Current horizontal position
	unsigned writeY = 0; //Current vertical position
	unsigned rowHeight = charinf[metrics[0].code - rangeBegin].bh; 
	int m = 0;
	
	for(m = 0; m < (rangeEnd - rangeBegin); m++)
	{
		int index = metrics[m].code - rangeBegin;
		
		if(FT_Load_Char(face, metrics[m].code, FT_LOAD_RENDER))
		{
			std::cerr << "Glyph copy load error" << std::endl;
			continue; //Exception instead
		}
		
		if(charinf[index].bh > rowHeight) rowHeight = charinf[index].bh; //To find maximum height of row
		
		if(charinf[index].bw > atlas_width - writeX) //Wrap around, if possible
		{
			writeY += rowHeight; //Go to the next row of space
			rowHeight = charinf[index].bh; //Reset local max to first character of new row
			writeX = 0;
		}
		
		if(charinf[index].bh > atlas_height - writeY) //Need more space
		{
			return false;
		}
		
		/*std::cout << "Working on: " << static_cast<char>(metrics[m].code) << " Write X: " << writeX << " Write Y: " << writeY << std::endl;*/
		for(int scanline = 0; scanline < charinf[index].bh; scanline++)
		{
			unsigned char *from = g->bitmap.buffer + (scanline * g->bitmap.width);
			unsigned char *until = from + g->bitmap.width;
			unsigned char *into = bitmap + (atlas_width * (writeY + scanline)) + writeX;
			
			//std::copy(from, until, into);
			memcpy_s(into, g->bitmap.width, from, g->bitmap.width);
		}
		
		map[index].lx = writeX;
		map[index].ly = writeY;
		map[index].hx = writeX + charinf[index].bw - 1; //Inclusive
		map[index].hy = writeY + charinf[index].bh - 1; //Inclusive
		
		writeX += charinf[index].bw;
	}
	
	if(m == (rangeEnd - rangeBegin))
	{
		//std::cout << "Atlas size: " << atlas_width << "x" << atlas_height << std::endl;
		return true;
	}
	else
	{
		return false;
	}
}