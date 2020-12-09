#pragma once
#include <string>
#include <vector>
#include <map>
#include <DirectXMath.h>
#ifndef ConfigParser_H
#define ConfigParser_H
class ConfigParser
{
public:
	struct TerrainPath
	{
		std::wstring heightPath;
		std::wstring colorPath;
		std::wstring normalPath;
	};
	struct Object
	{
		std::wstring type;
		std::wstring name;
		float scale;
		float rotationX;
		float rotationY;
		float rotationZ;
		float posX;
		float posY;
		float posZ;
	};
	struct MeshData
	{
		std::wstring name;
		std::wstring objPath;
		std::wstring diffusePath;
		std::wstring specularPath;
		std::wstring glowPath;
	};
	struct EnemyType
	{
		std::wstring enemyTypeName;
		float hitpoints;
		float unitSize;
		float unitSpeed;
		std::wstring mesh;
		float scale;
		float rotationX;
		float rotationY;
		float rotationZ;
		float positionX;
		float positionY;
		float positionZ;
	};

	struct ProjectileSettings {
		float projectileSpeed;
		signed int damage;
		float particleMass;
		float spawnPosX;
		float spawnPosY;
		float spawnPosZ;
		signed int spriteIndex;
		float spriteRadius;
		
	};

	struct Gun {
		std::wstring name;
		ProjectileSettings ProjectileSettings;
		float cooldown;
		bool enabled;
		bool isShooting;
		/*(std::wstring instanceName, float projSpeed, signed int _cooldown, signed int dam, float particleMs) 
		{
			name = instanceName;
			ProjectileSettings.projectileSpeed = projSpeed;
			cooldown = _cooldown;
			ProjectileSettings.damage = dam;
			ProjectileSettings.particleMass = particleMs;
			enabled = false;
			isShooting = false;
		};*/
		Gun() { enabled = false; isShooting = false; };
	};

	struct SpawnConfig
	{
		float interval;
		float minHeight;
		float maxHeight;
	};

	struct EnemyExplosion
	{
		std::wstring name;
		int textureIndex;
		float duration;
	};

	struct ParticleSystem
	{
		int particleNumber;
		float lifeTime;
		float minSpeed;
		float maxSpeed;
		float gravityY;
		signed int textureIndex;
	};

	Gun getGatlingGun() { return Gatling; };
	Gun getPlasmaGun() { return Plasma; };
	ParticleSystem getParticleSystem() { return particleSystem; };
	EnemyExplosion GetExplosion() { return explosion; };
	SpawnConfig getSpawner() { return Spawner; };
	TerrainPath getTerrainPath() { return Paths; };
	std::vector<MeshData> getMeshes() { return Meshes; };
	std::vector<Object> getObjects() { return Objects; };
	std::vector<std::wstring> getTextures() { return textureFiles; };
	std::map<std::wstring, EnemyType> getEnemyTypes() { return EnemyTypes; };
	float getTerrainWidth() { return TerrainWidth; };
	float getTerrainDepth() { return TerrainDepth; };
	float getTerrainHeight() { return TerrainHeight; };
	void loadNewType(std::string cfgFile);
	ConfigParser();
	~ConfigParser();
private:
	SpawnConfig Spawner;
	ParticleSystem particleSystem;
	Gun Gatling;
	Gun Plasma;
	EnemyExplosion explosion;
	std::vector<MeshData> Meshes;
	std::vector<Object> Objects;
	std::vector<std::wstring> textureFiles;
	std::map<std::wstring, EnemyType> EnemyTypes;
	TerrainPath Paths;
	float TerrainWidth;
	float TerrainDepth;
	float TerrainHeight;
};
#endif ConfigParser_H

extern ConfigParser configParser;

