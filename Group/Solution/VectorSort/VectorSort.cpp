// VectorSort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <iostream>
#include <algorithm>

bool f(int, int);

int main()
{
	std::vector<int> vector;
	int input = 2; int i = 0;

	while (input != 0) {
		std::cout << "Please enter your desired Integer to add: ";
		std::cin >> input;
		if (input != 0) {
			vector.push_back(1);
			vector[i] = input;
			i++;
		}
	}
	std::sort(vector.begin(), vector.end(), [](int a, int b) {
		return a > b;
	});
	
	for (auto a : vector) {
		std::cout << a << " ";
	}
	std::cout << "\n";

    return 0;
}

