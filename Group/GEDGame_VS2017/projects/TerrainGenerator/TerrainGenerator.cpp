// TerrainGenerator.cpp: Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <random>
#include "TerrainGenerator.h"
#include <ctime>
#include <fstream>
#include "SimpleImage.h"
#include "TextureGenerator.h"
#include "TexturesGenerator.h"

using namespace std;

int resolution = -1;

int _tmain(int argc, _TCHAR *argv[])
{
	srand(24932);

	int *resPointer = &resolution;
	wstring output_height;
	wstring *heightPointer = &output_height;
	wstring output_color;
	wstring *colorPointer = &output_color;
	wstring output_normal;
	wstring *normalPointer = &output_normal;

	/*Parsing the parameters into the variables*/
	fillParams(argc, argv, resPointer, heightPointer, colorPointer, normalPointer);

	if (!isPowerOfTwo(resolution))
	{
		cout << "ERROR - No resolution power of two specified" << endl;
		return -1;
	}

	/*HeightField Creation*/
	int size = resolution * resolution;
	float *heightField = new float[size];

	/*fillField(heightField, resolution*resolution) fills random floats into the array*/
	//fillField(heightField, resolution*resolution);
	/*fillWithDiamondSquareAlg(heightField, resolution*resolution) fills with the Diamond Square Algorithm*/
	fillWithDiamondSquareAlg(heightField, resolution);

	const int smoothRepeater = 10;
	cout << "\n========================================= Smoothing terrain ========================================== \n[";

	int smoothStep = smoothRepeater / 100;
	for (int i = 0; i < smoothRepeater; i++){
		//smoothStep++;
		/*if (i % smoothStep == 0) 
		{
			cout << "#";
		}*/
		smoothArray(heightField, resolution, resolution);
	}
	cout << "]\n" << endl;

	/*Provide the four essential textures for the TextureGenerator*/
	wstring textureLowFlat = L"../../../../external/textures/gras15.jpg";
	wstring& lowFlatRef = textureLowFlat;
	wstring textureLowSteep = L"../../../../external/textures/pebble01.jpg";
	wstring& lowSteepRef = textureLowSteep;
	wstring textureHighFlat = L"../../../../external/textures/rock5.jpg";
	wstring& highFlatRef = textureHighFlat;
	wstring textureHighSteep = L"../../../../external/textures/concretefloor012a.png";
	wstring& highSteepRef = textureHighSteep;

	vector<float> heightFieldVec;
	vector<float>* heightFieldVecPointer = &heightFieldVec; // Pointer for downsizing

	int index = 0;
	//cout << "Copy into vector..." << endl;
	while (index < size)
	{
		heightFieldVec.push_back(heightField[index]);
		index++;
	}

	const vector<float>& fieldRef = heightFieldVec;
	
	//cout << "Copying successful" << endl;
	//cout << "VectorPrint: " << endl;

	int i = 0;
	
	wstring colorPath = output_color;
	const wstring& colorRef = colorPath;
	wstring normalPath = output_normal;
	const wstring& normalRef = normalPath;

	/*Generate Textures*/
	//GEDUtils::TextureGenerator textGen(lowFlatRef, lowSteepRef, highFlatRef, highSteepRef);
	TexturesGenerator textGen(lowFlatRef, lowSteepRef, highFlatRef, highSteepRef);
	try
	{
		textGen.generateAndStoreImages(heightFieldVec, resolution, colorPath, normalPath);
	}
	catch (exception e)
	{
		cout << "Error: " << e.what() << endl;
	}
	// Downsizing and saving newly downsized image
	downSize(heightFieldVecPointer);
	GEDUtils::SimpleImage heightFieldDownsized(resolution, resolution);
	GEDUtils::SimpleImage* hFpointer = &heightFieldDownsized;

	for (int i = 0; i < resolution; i++) {
		for (int k = 0; k < resolution; k++) {
			(*hFpointer).setPixel(i, k, heightFieldVec[IDX(i, k, resolution)]);
		}
	}

	const wchar_t* heightChar = &(*heightPointer->c_str());
	wcout << ">> Saving under : " << heightChar << "..." << endl;

	heightFieldDownsized.save(heightChar);
	// End of downsizing

	delete[] heightField;

	system("pause");
    return 0;
}

bool isPowerOfTwo(int var)
{
	if ((var & (var - 1)) == 0)
	{
		return true;
	}
	return false;
}

void smoothArray(float* array, int width, int height) {
	float* eArray = new float[(width + 2)*(height + 2)];
	float* sArray = new float[width*height];

	// Clone corners to extended array's corners
	eArray[IDX(0, 0, width + 2)] = array[IDX(0, 0, width)]; // top left
	eArray[IDX(0, height + 1, width + 2)] = array[IDX(0, height - 1, width)]; // top right
	eArray[IDX(width + 1, 0, width + 2)] = array[IDX(width - 1, 0, width)]; // bottom left
	eArray[IDX(width + 1, height + 1, width + 2)] = array[IDX(width - 1, height - 1, width)]; // bottom right

	// Clone top and bottom row to extended array
	for (int x = 0; x < width; x++) {
		eArray[IDX(x + 1, 0, width + 2)] = array[IDX(x, 0, width)]; // top row
		eArray[IDX(x + 1, height + 1, width + 2)] = array[IDX(x, height - 1, width)]; // bottom row
	}

	// Clone left and right column to extended array
	for (int y = 0; y < height; y++) {
		eArray[IDX(0, y + 1, width + 2)] = array[IDX(0, y, width)]; // far left column
		eArray[IDX(width + 1, y + 1, width + 2)] = array[(IDX(width - 1, y, width))]; // far right column
	}

	// Clone remaining 2D array to extended array
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			eArray[IDX(x + 1, y + 1, width + 2)] = array[IDX(x, y, width)];
		}
	}

	// Fill sArray with computed averages
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			sArray[IDX(x, y, width)] = (
				eArray[IDX(x, y, width + 2)] + eArray[IDX(x + 1, y, width + 2)] + eArray[IDX(x + 2, y, width + 2)] +
				eArray[IDX(x, y + 1, width + 2)] + eArray[IDX(x + 1, y + 1, width + 2)] + eArray[IDX(x + 2, y + 1, width + 2)] +
				eArray[IDX(x, y + 2, width + 2)] + eArray[IDX(x + 1, y + 2, width + 2)] + eArray[IDX(x + 2, y + 2, width + 2)]) / 9;

			// O O O
			// O X O	// X + all O's divided by 9
			// O O O
		}
	}

	// Switch original array to smoothed array and delete all remaining arrays
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			array[IDX(x, y, width)] = sArray[IDX(x, y, width)];
		}
	}
	delete[] eArray;
	delete[] sArray;
}

void printArray(float* array, int width, int height)
{
	std::cout << "[";
	for (int i = 0; i < width*height; i++) {
		if ((i + 1) % width == 0) {
			if (i == (width*height) - 1) {
				std::cout << array[i] << "]\n\n";
			}
			else {
				std::cout << array[i] << "]\n[";
			}

		}
		else {
			std::cout << array[i] << "\t" << ", ";
		}
	}
}

wstring toWstr(string oldStr)
{
	/*'Wides' a normal string into a wstring type*/
	wstring transf;
	transf.assign(oldStr.begin(), oldStr.end());
	return transf;
}

unsigned int Strlength(wchar_t const* s) {
	/*detects length of string represented as char pointer*/
	unsigned int i = 0;
	while (*s++ != '\0')
		++i;

	return i;
}

bool validIntegerVal(wchar_t* s)
{
	/*checks if the string is a valid Integer number*/
	unsigned int size = Strlength(s);
	bool validInteger = true;
	for (int k = 0; k < size; k++)
	{
		if (!isdigit(*(s + k)))
		{
			validInteger = false;
			break;
		}
	}
	return validInteger;
}

void fillParams(int argc, _TCHAR *argv[], int *resPointer, wstring *heightPointer, wstring *colorPointer, wstring *normalPointer)
{
	//cout << "Anzahl Befehle: " << argc << endl;
	wcout << "Program: " << argv[0] << endl;
	/*argc: the number of (CLI-)params; argv: the params*/
	wcout << "argv[0] = " << argv[0] << endl;
	for (int i = 1; i < argc; i++)
	{
		/*Output arguments*/
		wcout << "argv[" << i << "] = " << argv[i] << "\t";
		if (i == argc/2) {
			cout << "\n";
		}
	}
	cout << "\n\n";

	for (int i = 1; i < argc; i += 2)
	{
		if (_tcscmp(argv[i], TEXT("-r")) == 0)
		{
			/*resolution parameter*/
			char minus = '-';
			cout << "argc: " << argc << "\t i = " << i << endl;
			if (argc <= i + 1)
			{
				cout << "ERROR - No resolution parameter" << endl;
				return;
			}
			if (!isdigit(*argv[i + 1]))
			{
				/*No valid number as parameter*/
				if (*argv[i + 1] == minus)
				{
					cout << "ERROR - No negative resolution parameter allowed" << endl;
					return;
				}
				else
				{
					cout << "ERROR - No resolution parameter" << endl;
					return;
				}
			}
			/*Fill resolution var*/
			bool validInteger = validIntegerVal(argv[i + 1]);
			if (!validInteger)
			{
				cout << "ERROR - No resolution parameter" << endl;
				return;
			}
			*resPointer = _tstoi(argv[i + 1]);
		}
		else if (_tcscmp(argv[i], TEXT("-o_height")) == 0)
		{
			/*Output height parameter*/
			if (argc < i + 1)
			{
				cout << "ERROR - No height output parameter" << endl;
				return;
			}
			*heightPointer = argv[i + 1];
		}
		else if (_tcscmp(argv[i], TEXT("-o_color")) == 0)
		{
			/*Output color parameter*/
			if (argc < i + 1)
			{
				cout << "ERROR - No color output parameter" << endl;
				return;
			}
			*colorPointer = argv[i + 1];
		}
		else if (_tcscmp(argv[i], TEXT("-o_normal")) == 0)
		{
			/*Output color parameter*/
			if (argc < i + 1)
			{
				cout << "ERROR - No normal output parameter" << endl;
				return;
			}
			*normalPointer = argv[i + 1];
		}
	}

	wcout << "resolution is " << *resPointer << endl;
	wcout << "output_height is " << *heightPointer << endl;
	wcout << "output_color is " << *colorPointer << endl;
	wcout << "output_normal is " << *normalPointer << endl;
}

void fillField(float* heightField, int size, float roughness)
{
	for (int i = 0; i < size; i++) {
		heightField[i] = randomVal(0.0f, 1.0f, roughness);
	}
}

void fillWithDiamondSquareAlg(float *heightField, int resolution)
{
	int size = (resolution + 1)*(resolution + 1);
	float roughness = 1.0f;
	float *tempHeightField = new float[size];

	//Initialize the corners with randomValues
	setHeight(0, 0, tempHeightField, randomVal(0.0f, 1.0f, roughness)); //Set random value to top / left corner
	setHeight(0, resolution, tempHeightField, randomVal(0.0f, 1.0f, roughness)); //Set random value to top / left corner
	setHeight(resolution, 0, tempHeightField, randomVal(0.0f, 1.0f, roughness)); //Set random value to top / left corner
	setHeight(resolution, resolution, tempHeightField, randomVal(0.0f, 1.0f, roughness)); //Set random value to top / left corner

	int margin = resolution / 2;//Distance from the border to the first point to be edited in the current step
	int distance = resolution;//Distance of the individual points to be processed in the current step

							  //Calculation of the first diamond
	diamondStep(margin, margin, margin, tempHeightField, roughness);
	squareStep(margin, margin, margin, tempHeightField, roughness);

	margin = margin / 2; //Distance and margin is halved after each step
	distance = distance / 2;


	int amount = 4; //Amount of new diamond midpoints in this step
	int counter = 1;

	float doneSteps = 0.0f;
	float currentProgress = 0.0f;
	float totalSteps = 0.0f;

	// Evaluate expected amount of steps for progress bar
	for (int i = 4; i <= resolution; i = i*2) {
		totalSteps += i * (i/2);
	}
	cout << "\n========================================= Generating terrain ========================================= \n[";

	roughness = 0.3f;
	while (distance >= 2)
	{
		int currXpos = margin, currYpos = margin;
		for (int a = 0; a < amount; a++) {//process all diamondSteps
			diamondStep(currXpos, currYpos, margin, tempHeightField, roughness);
			
			doneSteps++;
			//cout << doneSteps / totalSteps << endl;
			if ((doneSteps / totalSteps) > (currentProgress + 0.01f)) {
				currentProgress = doneSteps / totalSteps;
				cout << "#";
			}

			//cout << "processDiamondSteps counter:" << counter << " amount:" << amount <<  endl;
			if (currXpos + distance > resolution) {
				currXpos = margin;
				currYpos += distance;
			}
			else { currXpos += distance; }
		}
		currXpos = margin, currYpos = margin;//reseting margin and distance values
		for (int a = 0; a < amount; a++) {//process all squareSteps
			squareStep(currXpos, currYpos, margin, tempHeightField, roughness);

			doneSteps++;
			//cout << doneSteps / totalSteps << endl;
			if ((doneSteps / totalSteps) >(currentProgress + 0.01f)) {
				currentProgress = doneSteps / totalSteps;
				cout << "#";
			}

			if (currXpos + distance > resolution) {
				currXpos = margin;
				currYpos += distance;
			}
			else { currXpos += distance; }
		}
		roughness = roughness / 2;//Roughness decreases on every iteration
		counter++;
		amount *= 4;
		margin = margin / 2;//Distance and margin is halved after each step
		distance = distance / 2;
	}
	cout << "#]" << endl;

	//heightField = tempHeightField;
	for (int x = 0; x < resolution; x++) {//Cut the last column and the last row to fit the original format
		for (int y = 0; y < resolution; y++) {
			heightField[IDX(x, y, resolution)] = tempHeightField[IDX(x, y, resolution + 1)];
		}
	}

	delete[] tempHeightField;

}

float randomVal(float min, float max, float sigma) {
	default_random_engine *gen = new default_random_engine(rand());
	normal_distribution<float> floatDistributor((max + min) / 2, sigma);
	
	float rand = floatDistributor(*gen);
	while (!(rand >= min && rand <= max)) {
		rand = floatDistributor(*gen);
	}
	delete gen;

	return rand;
}

void  diamondStep(int x, int y, int delta, float *map, float roughness) {

	float *values = new float[4];
	values[0] = getHeight(x - delta, y - delta, map);//Value of top left corner
	values[1] = getHeight(x + delta, y - delta, map);//Value of top right corner
	values[2] = getHeight(x - delta, y + delta, map);//Value of bottom left corner
	values[3] = getHeight(x + delta, y + delta, map);//Value of bottom right corner


	float avg = 0;//Average of the four corners
	for (int i = 0; i < 4; i++) {
		avg += 0.25f * values[i];
	}

	float min = avg - roughness;
	float max = avg + roughness;
	if (min < 0) { min = 0; }
	if (max > 1) { max = 1; }
	float newVal = randomVal(min, max, roughness); 

	setHeight(x, y, map,newVal);
	delete[] values;
}

void  squareStep(int x, int y, int delta, float *map, float roughness) {
	int *posX = new int[4];
	int *posY = new int[4];

	posX[0] = x - delta; posY[0] = y; //new left point
	posX[1] = x + delta; posY[1] = y; //new right point
	posX[2] = x; posY[2] = y - delta; //new top point
	posX[3] = x; posY[3] = y + delta; //new bottom point

	for (int i = 0; i < 4; i++) {
		float sum = 0;
		int counter = 0;
		int x = posX[i];
		int y = posY[i];
		if (x - delta >= 0) {//Left Neighbour
							 //If this node exists in our grid
			float add = getHeight(x - delta, y, map);
			sum += add;
			counter++;
		}
		if (x + delta <= resolution) {//Right Neighbour
									  //Todod Check
			float add = getHeight(x + delta, y, map);
			sum += add;
			counter++;
		}
		if (y - delta >= 0) {//Top Neighbour
			float add = getHeight(x, y - delta, map);
			sum += add;
			counter++;
		}
		if (y + delta <= resolution) {//Bottom Neighbour
			float add = getHeight(x, y + delta, map);
			sum += add;
			counter++;
		}

		float avg = (sum / counter);

		float min = avg - roughness;
		float max = avg + roughness;
		if (min < 0) { min = 0; }
		if (max > 1) { max = 1; }
		float newVal = randomVal(min, max, roughness);

		setHeight(x, y, map, newVal);
	}
	delete[] posX;
	delete[] posY;
}

float getHeight(int x, int y, float *map) {
	int index = IDX(x, y, resolution + 1);
	if (index >= (resolution + 1)*(resolution + 1)) {
		cout << endl << "Index:" << index << " x:" << x << " y:" << y << " res:" << resolution << endl;
		system("pause");
	}
	return map[index];
}

void setHeight(int x, int y, float *map, float value) {
	int index = IDX(x, y, resolution + 1);
	map[index] = value;
}

void downSize(vector<float>* heightField)
{
	vector<float> copy;
	for (int y = 0; y < resolution; y = y + 2) {
		for (int x = 0; x < resolution; x = x + 2) {
			auto aData = new float;
			*aData = ((*heightField)[IDX(x, y, resolution)] + (*heightField)[IDX(x + 1, y, resolution)]
				+ (*heightField)[IDX(x + 1, y + 1, resolution)] + (*heightField)[IDX(x, y + 1, resolution)]) / 4;
			// Computes average of 4 entries and saves the average
			// X O
			// O O

			copy.push_back(*aData);
			delete aData;
		}
	}

	resolution = resolution / 2;

	// Switch orignal vector with newly created smaller vector
	(*heightField).clear();
	for (int i = 0; i < resolution*resolution; i++) {
		(*heightField).push_back(copy[i]);
	}
	std::cout << "Finished downsizing..." << endl;
}
