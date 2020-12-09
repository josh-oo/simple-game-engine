#include "stdafx.h"
#include "Texture.h"

#include <iostream>

GEDUtils::SimpleImage* img;

Texture::Texture(const wstring path)
{
	try{
		img = new GEDUtils::SimpleImage(path.c_str());
		wcout << "Image from " << path.c_str() << " loaded with dimensions: " << img->getWidth() << "x" << img->getHeight() << endl;
	}
	catch (exception e) {
		img = NULL;
		wcout << "Image from " << path.c_str() << " could not be loaded! "  << endl;
	}
}

void Texture::getPixelTiled(int x, int y, ColorVec *color) {
	if (img == NULL) { return; }
	float r, g, b;
	int width = (*img).getWidth();
	int height = (*img).getHeight();

	x = x % width;//Tile x Position
	y = y % height;//Tile y Position

	(*img).getPixel(x,y,r,g,b);
	color->r = r;
	color->g = g;
	color->b = b;
}

Texture::~Texture()
{
	if (img == NULL) { return; }
	delete img;
}
