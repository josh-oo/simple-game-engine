#include "stdafx.h"
#include "ConfigParser.h"
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

ConfigParser::ConfigParser()
{
}


ConfigParser::~ConfigParser()
{
}

string getPrefValue(vector<string> vec, string value)
{
	auto it = find(vec.begin(), vec.end(), value);
	if (it != vec.end())
	{
		return *it;
	}
	else
	{
		cout << "Not Found Exception" << endl;
		return "";
	}
}

void ConfigParser::load(std::string file)
{
	vector<string> Prefs{"spinning","spinSpeed","backgroundColor","terrainWidth","terrainDepth","terrainHeight","terrainPath"};

	ifstream ConfPars;
	ConfPars.open(file, ifstream::in);
	while(!ConfPars.is_open()){}
	cout << "Parse Opened" << endl;

	string prefSet;
	while (ConfPars >> prefSet)
	{
		if (getPrefValue(Prefs, prefSet) == "")
		{
			cout << "Incorrect identifier - ERROR ABORT" << endl;
			abort();
		}
		if (prefSet == "backgroundColor")
		{
			ConfPars >> ConfigParser::backgroundColor.r;
			ConfPars >> ConfigParser::backgroundColor.g;
			ConfPars >> ConfigParser::backgroundColor.b;
		}
		else if (prefSet == "spinning")
		{
			ConfPars >> ConfigParser::spinning;
		}
		else if (prefSet == "spinSpeed")
		{
			ConfPars >> ConfigParser::spinSpeed;
		}
		else if (prefSet == "terrainWidth")
		{
			ConfPars >> ConfigParser::terrainWidth;
		}
		else if (prefSet == "terrainDepth")
		{
			ConfPars >> ConfigParser::terrainDepth;
		}
		else if (prefSet == "terrainHeight")
		{
			ConfPars >> ConfigParser::terrainHeight;
		}
		else if (prefSet == "terrainPath")
		{
			ConfPars >> ConfigParser::terrainPath;
		}
	}
	ConfPars.close();
}

