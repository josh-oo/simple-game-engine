#pragma once

#include <string>
#include <vector>
#include "SimpleImage.h"

using namespace std;

#define IDX(x, y, w) ((x) + (y) * (w))
struct Vec
{
	float x;
	float y;
	Vec(float x, float y) : x(x), y(y){}
};

struct NormalVec
{
	/*Normal Vec consisting of x, y and z coord*/
	float x;
	float y;
	float z;

	NormalVec(float x, float y, float z) : x(x), y(y), z(z) {}
	NormalVec() : x(0), y(0), z(0) {}
	void setX(float x) { this->x = x; };
	void setY(float y) { this->y = y; };
	void setZ(float z) { this->z = z; };
	void normalize(NormalVec *normal);
	static void mapValuesToPositives(vector<NormalVec>* normals, int res);
};

struct ColorVec
{
	/*Color Vec consisting of rgb values as well as a*/
	float r;
	float g;
	float b;
	float a;

	ColorVec(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
	ColorVec() : r(0), g(0), b(0), a(1) {}
};

class TexturesGenerator
{
public:
	TexturesGenerator(const wstring txtLowFlat,const wstring txtLowSteep,const wstring txtHighFlat,const wstring txtHighSteep);
	~TexturesGenerator();
	
	/*Create list of normals vec out of the heightFIeld and the resolution*/
	void createNormals(const vector<float> heightField, int res, vector<NormalVec> *normals);

	/*Create list of colors out of the heightField, the resolution and the normal vectors*/
	void createColors(const vector<float> heightField, int res, vector<NormalVec> *normals, vector<ColorVec> *colors);

	void generateAndStoreImages(const std::vector<float>& heightfield, int resolution,
		const std::wstring& colorFilename, const std::wstring& normalsFilename);
private:
	void calcAlphas(float height, float slope, float& alpha1, float& alpha2, float& alpha3);
};

