#pragma once
#include <string>
#include "TexturesGenerator.h"
#include "SimpleImage.h"
using namespace std;

class Texture
{
public:
	Texture(const wstring path);
	~Texture();

	void getPixelTiled(int x, int y, ColorVec *color);
private:
	GEDUtils::SimpleImage* img;
};

