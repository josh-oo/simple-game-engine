// 2DArraySmoothing.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <time.h> 
#include <cstdlib>

// Access a 2D array of width w at position x / y
#define IDX(x, y, w) ((x) + (y) * (w))

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

void smoothArray(float* array, int width, int height) {
	float* eArray = new float[(width+2)*(height+2)];
	float* sArray = new float[width*height];

	// Clone corners to extended array's corners
	eArray[IDX(0, 0, width + 2)] = array[IDX(0, 0, width)]; // top left
	eArray[IDX(0, height+ 1, width + 2)] = array[IDX(0, height - 1, width)]; // top right
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

	printArray(eArray, width + 2, height + 2);

	// Fill sArray with computed averages
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			sArray[IDX(x, y, width)] = (
				eArray[IDX(x, y, width + 2)]	 + eArray[IDX(x + 1, y, width + 2)]		+ eArray[IDX(x + 2, y, width + 2)] +
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

int main()
{

	// Define 2Darray dimensions here
	const int width = 5;
	const int height = 10;

	// Instantiate array and print chosen size
	float* vArray = new float[width*height];
	std::cout << "Array Size: " << width*height << "\n\n";


	// Set random seed
	srand(time(nullptr));

	// Fill generated array with random float values between 0.0f and 1.0f
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			vArray[IDX(i, j, width)] = (float)rand() / RAND_MAX;;

		}
	}

	// Print randomly generated Array
	std::cout << "Randomly generated Array:\n\n";
	printArray(vArray, width, height);

	// Smooth genereated Array
	smoothArray(vArray, width, height);

	// Print now smoothed Array
	std::cout << "Smoothed Array:\n\n";
	printArray(vArray, width, height);

	system("pause");
	return 0;
}

