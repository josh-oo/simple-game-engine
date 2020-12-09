#pragma once
#include <string>
#ifndef ConfigParser_H
#define ConfigParser_H
class ConfigParser
{
public:
	struct Color {
		float r;
		float g;
		float b;
	};
	float getSpinning() { return spinning; };
	float getSpinSpeed() { return spinSpeed; };
	Color getBackgroundColor() { return backgroundColor; };
	std::string getTerrainPath() { return terrainPath; };
	float getTerrainWidth() { return terrainWidth; };
	float getTerrainDepth() { return terrainDepth; }
	float getTerrainHeight() { return terrainHeight; };
	void load(std::string);
	ConfigParser();
	~ConfigParser();
private:
	float spinning;
	float spinSpeed;
	Color backgroundColor;
	std::string terrainPath;
	float terrainWidth;
	float terrainDepth;
	float terrainHeight;
};
#endif ConfigParser_H

