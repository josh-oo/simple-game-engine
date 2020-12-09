#pragma once
#include "DXUT.h"
#include "d3dx11effect.h"
#include <vector>
#include "ConfigParser.h"

using namespace std;

struct SimpleVertex
{
	DirectX::XMFLOAT4 Pos;
	DirectX::XMFLOAT4 Normal;
	DirectX::XMFLOAT2 UV;
	/*float x;
	float y;
	float z;*/
	SimpleVertex(float vx, float vy,float vz, float nx, float ny, float nz, float u, float v) : Pos(vx,vy,vz,1.0f), Normal(nx,ny,nz,0.0f), UV(u,v) {}
	SimpleVertex(float vx, float vy, float vz, DirectX::XMVECTOR normal, float u, float v) : Pos(vx, vy, vz, 1.0f), UV(u, v) { DirectX::XMStoreFloat4(&Normal, normal); }
	SimpleVertex(float vx, float vy, float vz, float u, float v) : Pos(vx, vy, vz, 1.0f), Normal(0.0f, 1.0f, 0.0f, 0.0f), UV(u, v) {}
	SimpleVertex() : Pos(), Normal(), UV() {}
};

/*struct TexCoord
{
	float x;
	float y;
	TexCoord(float x, float y) : x(x), y(y) {}
	TexCoord() : x(0.0f), y(0.0f) {}
};*/

class Terrain
{
public:
	Terrain(void);
	~Terrain(void);

	HRESULT create(ID3D11Device* device, ConfigParser configParser);

	void destroy();

	void render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass);


private:
	Terrain(const Terrain&);
	Terrain(const Terrain&&);
	void operator=(const Terrain&);

	// Terrain rendering resources
	//ID3D11Buffer*                           vertexBuffer;	// The terrain's vertices
	ID3D11Buffer*                           indexBuffer;	// The terrain's triangulation
	ID3D11Texture2D*                        diffuseTexture; // The terrain's material color for diffuse lighting
	ID3D11ShaderResourceView*               diffuseTextureSRV; // Describes the structure of the diffuse texture to the shader stages

	ID3D11Texture2D*                        normalTexture; // The terrain's material color for diffuse lighting
	ID3D11ShaderResourceView*               normalTextureSRV; // Describes the structure of the diffuse texture to the shader stages

	ID3D11Buffer*							heightMap;
	ID3D11ShaderResourceView*				heightMapSRV;

	// General resources
	ID3D11ShaderResourceView*               debugSRV;

	//void getVertexPosFromHeightMap(float terrainWidth, float terrainDepth, float terrainHeight, vector<SimpleVertex>* vertices);
	void getBufferFromHeightMap(vector<float> *values, wstring heightPath);
};

