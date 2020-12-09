#include "stdafx.h"
#include <iostream>
#include <math.h>
#include "TexturesGenerator.h"
#include "SimpleImage.h"
#include <fstream>
#include "Texture.h"
using namespace std;

Texture *lowFlat;
Texture *lowSteep;
Texture *highFlat;
Texture *highSteep;

TexturesGenerator::TexturesGenerator(const wstring txtLowFlat, const wstring txtLowSteep, const wstring txtHighFlat, const wstring txtHighSteep)
{
	lowFlat = new Texture(txtLowFlat);
	lowSteep = new Texture(txtLowSteep);
	highFlat = new Texture(txtHighFlat);
	highSteep = new Texture(txtHighSteep);
}


TexturesGenerator::~TexturesGenerator()
{
	delete lowFlat;
	delete lowSteep;
	delete highFlat;
	delete highSteep;
}

void TexturesGenerator::createNormals(const vector<float> heightField, int res, vector<NormalVec> *normals)
{
	if (heightField.size() != res * res)
	{
		cout << "size(): " << heightField.size() << " and res * res = " << res * res << endl;

		return;
	}
	/*Calculate Normal for every heightPoint*/
	for (int i = 0; i < res; i++)
	{
		for (int k = 0; k < res; k++)
		{
			NormalVec *cur = new NormalVec(0, 0, 0);
			/*Calculate Vector with
				*	two tangents, representing the slope in x-direction and in y-direction
				*	the cross-product between these two tangents--> normal vector
			*/
			/*Creating the two tangents*/
			float valXMinOne;
			float valXPlusOne;
			float valYMinOne;
			float valYPlusOne;

			if (i == 0)
			{
				valXMinOne = 0.5;
				valXPlusOne = heightField.at(IDX(i + 1, k, res));
			}
			else if (i == res-1)
			{
				valXMinOne = heightField.at(IDX(i - 1, k, res));
				valXPlusOne = 0.5;
			}
			else
			{
				valXMinOne = heightField.at(IDX(i - 1, k, res));
				valXPlusOne = heightField.at(IDX(i + 1, k, res));
			}

			if (k == 0)
			{
				valYMinOne = 0.5;
				valYPlusOne = heightField.at(IDX(i, k + 1, res));
			}
			else if (k == res-1)
			{
				valYMinOne = heightField.at(IDX(i, k - 1, res));
				valYPlusOne = 0.5;
			}
			else
			{
				valYMinOne = heightField.at(IDX(i, k - 1, res));
				valYPlusOne = heightField.at(IDX(i, k + 1, res));
			}

			/*Calculating the cross product of the two tangents*/
			cur->setX(-((valXPlusOne-valXMinOne)/2)*res);
			cur->setY(-((valYPlusOne-valYMinOne)/2)*res);
			cur->setZ(1);

			/*Normalize*/
			cur->normalize(cur);

			/*Insert in NormalVector List*/
			(*normals).push_back(*cur);
			delete cur;
		}
	}
	for (int i = 0; i < res; i++)
	{
		for (int k = 0; k < res; k++)
		{
			NormalVec cur = (*normals).at(IDX(i, k, res));
			if (cur.x < -1 || cur.y < -1 || cur.z < -1 || cur.x > 1 || cur.y > 1 || cur.z > 1)
			{
				cout << "ERROR - Vector component out of range" << endl;
				return;
			}
		}
	}

	NormalVec::mapValuesToPositives(normals, res);

	for (int i = 0; i < res; i++)
	{
		for (int k = 0; k < res; k++)
		{
			NormalVec cur = (*normals).at(IDX(i, k, res));
			if (cur.x < 0 || cur.y < 0 || cur.z < 0 || cur.x > 1 || cur.y > 1 || cur.z > 1)
			{
				cout << "ERROR - Vector component out of range" << endl;
				return;
			}
		}
	}
}

/*Create list of colors out of the heightField, the resolution and the normal vectors*/
void TexturesGenerator::createColors(const vector<float> heightField, int res, vector<NormalVec> *normals, vector<ColorVec> *colors)
{
	if (heightField.size() != res * res)
	{
		cout << "size(): " << heightField.size() << " and res * res = " << res * res << endl;
		return;
	}

	for (int x = 0; x < res; x++) {
		for (int y = 0; y < res; y++) {
			float a1, a2, a3;//Alpha Values for current Pixel
			float currHeight = heightField.at(IDX(x, y, res));
			float slope = 1.0f - normals->at(IDX(x,y,res)).z;
			calcAlphas(currHeight,slope,a1,a2,a3);

			//cout << "calculated alpha values: a1:" << a1 << " a2:" << a2 << " a3:" << a3 << endl;

			ColorVec *c0,*c1,*c2,*c3;
			c0 = new ColorVec();
			c1 = new ColorVec();
			c2 = new ColorVec();
			c3 = new ColorVec();

			//Get the color of all layers
			lowFlat->getPixelTiled(x,y,c0);
			lowSteep->getPixelTiled(x,y,c1);
			highFlat->getPixelTiled(x,y,c2);
			highSteep->getPixelTiled(x, y, c3);

			//𝐶3=𝛼3∗𝑐3+(1−𝛼3)∗(𝛼2∗𝑐2+1−𝛼2∗(𝛼1∗𝑐1+1−𝛼1∗𝑐0))
			ColorVec *newColor = new ColorVec();
			//Blend the pixel colors
			newColor->b = (a3*c3->b) + (1 - a3)*(a2*c2->b + (1 - a2) * (a1*c1->b + (1 - a1) * c0->b));
			newColor->g = (a3*c3->g) + (1 - a3)*(a2*c2->g + (1 - a2) * (a1*c1->g + (1 - a1) * c0->g));
			newColor->r = (a3*c3->r) + (1 - a3)*(a2*c2->r + (1 - a2) * (a1*c1->r + (1 - a1) * c0->r));

			(*colors).push_back(*newColor);

			delete c0, c1, c2, c3;
			delete newColor;
		}
	}
}

void TexturesGenerator::calcAlphas(float height, float slope, float& alpha1, float& alpha2, float& alpha3) {

	alpha1 = ((1 - height) * slope)/2; //Sharper blendings
	alpha2 = height*2;
	alpha3 = (height * slope)/2;

	if (alpha1 > 1) { alpha1 = 1; }
	if (alpha2 > 1) { alpha2 = 1; }
	if (alpha3 > 1) { alpha3 = 1; }
}

void TexturesGenerator::generateAndStoreImages(const std::vector<float>& heightfield, int resolution,
	const std::wstring& colorFilename, const std::wstring& normalsFilename) {

	vector<NormalVec> *normals = new vector<NormalVec>;
	vector<ColorVec> *colors = new vector<ColorVec>;


	createNormals(heightfield,resolution,normals);
	cout << "Normal created!" << endl;
	createColors(heightfield, resolution, normals, colors);
	cout << "Colors created!" << endl;

	GEDUtils::SimpleImage normalImage(resolution, resolution);
	GEDUtils::SimpleImage colorImage (resolution, resolution);
	for (int i = 0; i < resolution; i++)
	{
		for (int k = 0; k < resolution; k++)
		{
			NormalVec nrm = (*normals).at(IDX(i, k, resolution));
			ColorVec clr = (*colors).at(IDX(i, k, resolution));
			(normalImage).setPixel(k, i, nrm.x, nrm.y, nrm.z);
			(colorImage).setPixel(k, i, clr.r, clr.g, clr.b);
		}
	}

	const wchar_t *normalFile = normalsFilename.c_str();
	const wchar_t *colorFile = colorFilename.c_str();
	(normalImage).save(normalFile); 
	(colorImage).save(colorFile);

	delete normals, colors;
}

void NormalVec::normalize(NormalVec *normal)
{
	float magnitude = sqrt(normal->x * normal->x + normal->y * normal->y + normal->z * normal->z);

	normal->x = (*normal).x / magnitude;
	normal->y = (*normal).y / magnitude;
	normal->z = (*normal).z / magnitude;
}

void NormalVec::mapValuesToPositives(vector<NormalVec>* normals, int res)
{
	/*maps values [-1;1] to [0;1]*/
	for (int i = 0; i < res * res; i++)
	{
		NormalVec cur = (*normals)[i];
		cur.x = (cur.x + 1) / 2;
		cur.y = (cur.y + 1) / 2;
		cur.z = (cur.z + 1) / 2;
		(*normals)[i] = cur;
	}
}

