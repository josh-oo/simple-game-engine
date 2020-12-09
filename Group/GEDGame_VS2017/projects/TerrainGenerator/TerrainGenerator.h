
#include <iostream>
#include <string>
#include "windows.h"
#include <vector>

#define IDX(x, y, w) ((x) + (y) * (w))
void smoothArray(float* array, int width, int height);
void printArray(float* array, int width, int height);
unsigned int Strlength(wchar_t const* s);
bool validIntegerVal(wchar_t const* s);
void fillParams(int argc, _TCHAR *argv[], int *resPointer, std::wstring *heightPointer, std::wstring *colorPointer, std::wstring *normalPointer);
void fillField(float *heightField, int size, float roughness);
void fillWithDiamondSquareAlg(float *heightField, int size);
float randomVal(float min, float max, float sigma);
void setHeight(int x, int y, float *map, float value);
float getHeight(int x, int y, float *map);
void  diamondStep(int x, int y, int delta, float *map, float roughness);
void squareStep(int x, int y, int delta, float *map, float roughness);
bool isPowerOfTwo(int var);
void downSize(std::vector<float>* heightField);
