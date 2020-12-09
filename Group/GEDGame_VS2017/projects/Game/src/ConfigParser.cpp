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

wstring getPrefValue(vector<wstring> vec, wstring value)
{
	if (value[0] == '#')
	{
		return L"comment";
	}
	auto it = find(vec.begin(), vec.end(), value);
	if (it != vec.end())
	{
		return *it;
	}
	else
	{
		cout << "Not Found Exception" << endl;
		return L"";
	}
}

void ConfigParser::loadNewType(std::string file)
{
	vector<wstring> Prefs{ L"TerrainPath",L"TerrainWidth",L"TerrainDepth",L"TerrainHeight",L"Mesh",
		L"CockpitObject", L"GroundObject", L"EnemyType", L"Spawn", L"Textures", L"GatlingGun", L"PlasmaGun",
		L"Explosion", L"ParticleSystem"};
	wifstream ConfPars;
	ConfPars.open(file, wifstream::in);
	while (!ConfPars.is_open()) {}
	cout << "Parse Opened" << endl;

	wstring prefSet;

	while (ConfPars >> prefSet)
	{
		wstring existing = getPrefValue(Prefs, prefSet);
		if (existing == L"")
		{
			cout << "Incorrect identifier - ERROR ABORT" << endl;
			abort();
		}
		else if (existing == L"comment")
		{
			bool nLine = false;
			wchar_t oneChar;
			wstring wholeLine = prefSet;
			ConfPars.get(oneChar);
			while (oneChar != '\n')
			{
				wholeLine = wholeLine + oneChar;
				ConfPars.get(oneChar);
			}
			wcout << "Comment: " << wholeLine << endl;
		}
		else if (prefSet == L"TerrainWidth")
		{
			ConfPars >> ConfigParser::TerrainWidth;
		}
		else if (prefSet == L"TerrainDepth")
		{
			ConfPars >> ConfigParser::TerrainDepth;
		}
		else if (prefSet == L"TerrainHeight")
		{
			ConfPars >> ConfigParser::TerrainHeight;
		}
		else if (prefSet == L"TerrainPath")
		{
			ConfPars >> ConfigParser::Paths.heightPath;
			ConfPars >> ConfigParser::Paths.colorPath;
			ConfPars >> ConfigParser::Paths.normalPath;
		}
		else if (prefSet == L"Mesh")
		{
			MeshData localMesh;
			ConfPars >> localMesh.name;
			ConfPars >> localMesh.objPath;
			ConfPars >> localMesh.diffusePath;
			ConfPars >> localMesh.specularPath;
			ConfPars >> localMesh.glowPath;
			configParser.Meshes.push_back(localMesh);
		}
		else if ( prefSet == L"CockpitObject" || prefSet == L"GroundObject")
		{
			Object localObject;
			if (prefSet == L"CockpitObject")
			{
				localObject.type = L"CockpitObject";
			}
			else if (prefSet == L"GroundObject")
			{
				localObject.type = L"GroundObject";
			}
			ConfPars >> localObject.name;
			ConfPars >> localObject.scale;
			ConfPars >> localObject.rotationX;
			ConfPars >> localObject.rotationY;
			ConfPars >> localObject.rotationZ;
			ConfPars >> localObject.posX;
			ConfPars >> localObject.posY;
			ConfPars >> localObject.posZ;
			configParser.Objects.push_back(localObject);
		}
		else if (prefSet == L"EnemyType")
		{
			EnemyType localType;
			ConfPars >> localType.enemyTypeName;
			ConfPars >> localType.hitpoints;
			ConfPars >> localType.unitSize;
			ConfPars >> localType.unitSpeed;
			ConfPars >> localType.mesh;
			ConfPars >> localType.scale;
			ConfPars >> localType.rotationX;
			ConfPars >> localType.rotationY;
			ConfPars >> localType.rotationZ;
			ConfPars >> localType.positionX;
			ConfPars >> localType.positionY;
			ConfPars >> localType.positionZ;
			configParser.EnemyTypes.insert(std::pair<wstring, EnemyType>(localType.enemyTypeName, localType));
		}
		else if (prefSet == L"Spawn")
		{
			ConfPars >> configParser.Spawner.interval;
			ConfPars >> configParser.Spawner.minHeight;
			ConfPars >> configParser.Spawner.maxHeight;
			
		}
		else if (prefSet == L"Textures")
		{
			wstring line;
			std::getline(ConfPars, line);
			wcout << "Line: " << line << endl;
			const size_t newLinePos = line.find('\n', 1);
			size_t before = 1;
			size_t after = 1;
			while ((after=line.find(' ', before)) != wstring::npos)
			{
				wstring texturePath = line.substr(before, (after-before));
				wcout << "texturePath: " << texturePath << " loaded!" << endl;
				configParser.textureFiles.push_back(texturePath);
				before = after + 1;
			}
			wstring lastTexturePath = line.substr(before, (newLinePos-1-before));
			wcout << "LatexturePath: " << lastTexturePath  << " loaded!"<< endl;
			configParser.textureFiles.push_back(lastTexturePath);
		}
		else if (prefSet == L"GatlingGun")
		{
			ConfPars >> configParser.Gatling.name;
			ConfPars >> configParser.Gatling.ProjectileSettings.projectileSpeed;
			ConfPars >> configParser.Gatling.cooldown;
			ConfPars >> configParser.Gatling.ProjectileSettings.damage;
			ConfPars >> configParser.Gatling.ProjectileSettings.particleMass;
			ConfPars >> configParser.Gatling.ProjectileSettings.spawnPosX;
			ConfPars >> configParser.Gatling.ProjectileSettings.spawnPosY;
			ConfPars >> configParser.Gatling.ProjectileSettings.spawnPosZ;
			ConfPars >> configParser.Gatling.ProjectileSettings.spriteIndex;
			ConfPars >> configParser.Gatling.ProjectileSettings.spriteRadius;
		}
		else if (prefSet == L"PlasmaGun")
		{
			ConfPars >> configParser.Plasma.name;
			ConfPars >> configParser.Plasma.ProjectileSettings.projectileSpeed;
			ConfPars >> configParser.Plasma.cooldown;
			ConfPars >> configParser.Plasma.ProjectileSettings.damage;
			ConfPars >> configParser.Plasma.ProjectileSettings.particleMass;
			ConfPars >> configParser.Plasma.ProjectileSettings.spawnPosX;
			ConfPars >> configParser.Plasma.ProjectileSettings.spawnPosY;
			ConfPars >> configParser.Plasma.ProjectileSettings.spawnPosZ;
			ConfPars >> configParser.Plasma.ProjectileSettings.spriteIndex;
			ConfPars >> configParser.Plasma.ProjectileSettings.spriteRadius;
		}
		else if (prefSet == L"Explosion")
		{
			ConfPars >> configParser.explosion.name;
			ConfPars >> configParser.explosion.textureIndex;
			ConfPars >> configParser.explosion.duration;
		}
		else if (prefSet == L"ParticleSystem")
		{
			ConfPars >> configParser.particleSystem.particleNumber;
			ConfPars >> configParser.particleSystem.lifeTime;
			ConfPars >> configParser.particleSystem.minSpeed;
			ConfPars >> configParser.particleSystem.maxSpeed;
			ConfPars >> configParser.particleSystem.gravityY;
			ConfPars >> configParser.particleSystem.textureIndex;
		}
	}
	cout << "Parse finished" << endl;
	ConfPars.close();
}

