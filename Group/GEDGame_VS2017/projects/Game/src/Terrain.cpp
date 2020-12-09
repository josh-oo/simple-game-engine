#include "Terrain.h"

#include "GameEffect.h"
#include "SimpleImage.h"
#include <DDSTextureLoader.h>
#include "DirectXTex.h"
#include <DirectXMath.h>

// You can use this macro to access your height field
#define IDX(X,Y,WIDTH) ((X) + (Y) * (WIDTH))

float imgWidth = 0;
float imgHeight = 0;

Terrain::Terrain(void) :
	indexBuffer(nullptr),
	//vertexBuffer(nullptr),
	diffuseTexture(nullptr),
	diffuseTextureSRV(nullptr),
	debugSRV(nullptr),
	normalTexture(nullptr),
	normalTextureSRV(nullptr)
{
}


Terrain::~Terrain(void)
{
}

HRESULT Terrain::create(ID3D11Device* device, ConfigParser configParser)
{
	HRESULT hr;

	// This buffer contains positions, normals and texture coordinates for one triangle
    /*float triangle[] = {
        // Vertex 0
           -400.0f, 0.0f, -400.0f,  1.0f, // Position
           0.0f,    1.0f,    0.0f,  0.0f, // Normal
           0.0f,    0.0f,                 // Texcoords

        // Vertex 1
           400.0f,   0.0f, -400.0f, 1.0f, // Position
           0.0f,     1.0f,    0.0f, 0.0f, // Normal
           0.0f,     1.0f,                // Texcoords

        // Vertex 2
           -400.0f, 0.0f,  400.0f,  1.0f, // Position
           0.0f,    1.0f,    0.0f,  0.0f, // Normal
           1.0f,    0.0f,                 // Texcoords

		// Vertex 2
			400.0f, 0.0f,  400.0f,  1.0f, // Position
			0.0f,    1.0f,    0.0f,  0.0f, // Normal
			1.0f,    1.0f,                 // Texcoords
    };*/

	// Note 1: The normal map that you created last week will not be used
	// in this assignment (Assignment 4). It will be of use in later assignments

	// Note 2: For each vertex 10 floats are stored. Do not use another
	// layout.

	// Note 3: In the coordinate system of the vertex buffer (output):
	// x = east,    y = up,    z = south,          x,y,z in [0,1] (float)

	vector<float> *heightValues = new vector<float>;
	getBufferFromHeightMap(heightValues, configParser.getTerrainPath().heightPath);	


    D3D11_SUBRESOURCE_DATA id;
	id.pSysMem = &((*heightValues)[0]);//First element of vector
    id.SysMemPitch = sizeof(float); // Stride
    id.SysMemSlicePitch = 0;

    D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.ByteWidth = imgHeight * imgWidth * sizeof(float); //The size in bytes of the triangle array
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;


	//Creating the Shader Resource View description
    V(device->CreateBuffer(&bd, &id, &heightMap)); // http://msdn.microsoft.com/en-us/library/ff476899%28v=vs.85%29.aspx

	D3D11_SHADER_RESOURCE_VIEW_DESC sd;
	sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	sd.Format = DXGI_FORMAT_R32_FLOAT;
	sd.Buffer.FirstElement = 0;
	sd.Buffer.NumElements = imgHeight * imgWidth;

	//Create resource view
	V(device->CreateShaderResourceView(heightMap, &sd, &heightMapSRV));

	int indicesSize = (int)(((imgHeight-1)*(imgWidth-1))*2*3);
	//A heightmap of size x * y generates squares of number (x-1) * (y-1). 
	//Each square consists of two triangles. Each triangle consists of 3 vertices.

	// Create index buffer
	int *indices = new int[indicesSize];

	for (int y = 0; y < imgHeight - 1; y++) {
		for (int x = 0; x < imgWidth - 1; x++) {//Iterate on every quad
			int currentIndex = IDX(x,y,imgWidth-1) * 6;
			indices[currentIndex + 0] = IDX(x, y, imgWidth);//First triangle of the quad
			indices[currentIndex + 1] = IDX(x + 1, y, imgWidth);
			indices[currentIndex + 2] = IDX(x, y + 1, imgWidth);
			indices[currentIndex + 3] = IDX(x, y + 1, imgWidth);//Second triangle of the quad
			indices[currentIndex + 4] = IDX(x + 1, y, imgWidth);
			indices[currentIndex + 5] = IDX(x + 1, y+1, imgWidth);
		}
	}

	// Create and fill description
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(unsigned int) * indicesSize;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	// Define initial data
	ZeroMemory(&id, sizeof(id));
	id.pSysMem = indices;
	// Create Buffer
	V(device->CreateBuffer(&bd, &id, &indexBuffer));


	// Load color texture (color map)
	wstring col = L"resources\\" + configParser.getTerrainPath().colorPath;
	V(DirectX::CreateDDSTextureFromFile(device, col.c_str(),nullptr, &diffuseTextureSRV));
	if (hr != S_OK) {
		MessageBoxA(NULL, "Could not load texture \"resources\\terrain_color.dds\"", "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}
	
	// Load normal texture (normal map)
	wstring norm = L"resources\\" + configParser.getTerrainPath().normalPath;
	V(DirectX::CreateDDSTextureFromFile(device, norm.c_str(),nullptr, &normalTextureSRV));
	if (hr != S_OK) {
		MessageBoxA(NULL, "Could not load texture \"resources\\terrain_normal.dds\"", "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}

	delete[] indices;// heightFloats;
	delete heightValues;

	return hr;
}

void Terrain::getBufferFromHeightMap(vector<float>* values, wstring heightPath) {
	heightPath = L"resources\\" + heightPath;
	GEDUtils::SimpleImage img(heightPath.c_str());
	imgHeight = img.getHeight();
	imgWidth = img.getWidth();

	for (int y = 0; y < imgHeight; y++) {
		for (int x = 0; x < imgWidth; x++) {
			float currHeightValue = img.getPixel(x, y);
			values->push_back(currHeightValue);
		}
	}
}

/*void Terrain::getVertexPosFromHeightMap(float terrainWidth, float terrainDepth,float terrainHeight, vector<SimpleVertex> *vertices) {
	wstring path = L"resources\\terrain_height.tiff";
	GEDUtils::SimpleImage *img = new GEDUtils::SimpleImage(path.c_str());
	imgHeight = img->getHeight();
	imgWidth = img->getWidth();

	float relWidth = terrainWidth / (imgWidth-1);
	float relDepth = terrainDepth / (imgHeight-1);

	float shiftWidth = terrainWidth / 2.0f;
	float shiftDepth = terrainDepth / 2.0f;

	for (int y = 0; y < imgHeight; y++) {
		for (int x = 0; x < imgWidth; x++) {
			float currHeightValue = img->getPixel(x, y);
			float currY = currHeightValue * terrainHeight;
			float currZ = y * relDepth -shiftDepth;
			float currX = x * relWidth -shiftWidth;

			float u = x / (imgWidth - 1);
			float v = y / (imgHeight - 1);

			float valXMinOne;
			float valXPlusOne;
			float valYMinOne;
			float valYPlusOne;

			if (x == 0)
			{
				valXMinOne = 0.5;
				valXPlusOne = img->getPixel(x + 1, y);
			}
			else if (x == imgWidth - 1)
			{
				valXMinOne = img->getPixel(x - 1, y);
				valXPlusOne = 0.5;
			}
			else
			{
				valXMinOne = img->getPixel(x - 1, y);
				valXPlusOne = img->getPixel(x + 1, y);
			}

			if (y == 0)
			{
				valYMinOne = 0.5;
				valYPlusOne = img->getPixel(x, y + 1);
			}
			else if (y == imgHeight - 1)
			{
				valYMinOne = img->getPixel(x, y - 1);
				valYPlusOne = 0.5;
			}
			else
			{
				valYMinOne = img->getPixel(x, y - 1);
				valYPlusOne = img->getPixel(x, y + 1);
			}

			float normalX = ((-(valXPlusOne - valXMinOne) / 2)*imgWidth);
			float normalY = ((-(valYPlusOne - valYMinOne) / 2)*imgHeight);
			float normalZ = 1.0f;

			DirectX::XMMATRIX normalUnscaled = DirectX::XMMatrixScaling(terrainWidth, terrainWidth, terrainWidth);
			normalUnscaled = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, normalUnscaled));
			DirectX::XMVECTOR vNormal = DirectX::XMVectorSet(normalX, normalZ, normalY, 1);
			vNormal = DirectX::XMVector4Transform(vNormal, normalUnscaled);
			vNormal = DirectX::XMVector3Normalize(vNormal);

			/*float magnitude = sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ);

			normalX /= magnitude;
			normalY /= magnitude;
			normalZ /= magnitude;

			normalX = (normalX + 1) / 2;
			normalY = (normalY + 1) / 2;
			normalZ = (normalZ + 1) / 2;


			//SimpleVertex *vert = new SimpleVertex(currX,currY,currZ, normalX, normalY, normalZ,u,v);
			SimpleVertex *vert = new SimpleVertex(currX, currY, currZ, vNormal, u, v);
			vertices->push_back(*vert);
			delete vert;
		}
	}
	delete img;

}
*/

void Terrain::destroy()
{
	SAFE_RELEASE(heightMap);
	SAFE_RELEASE(heightMapSRV);
	SAFE_RELEASE(normalTextureSRV);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(debugSRV);

	SAFE_RELEASE(diffuseTextureSRV);
}


void Terrain::render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass)
{
	HRESULT hr;

	// Bind the terrain vertex buffer to the input assembler stage 
    ID3D11Buffer* vbs[] = { nullptr, };
    //unsigned int strides[] = { 10 * sizeof(float), }, offsets[] = { 0, };
	unsigned int strides[] = { 0, }, offsets[] = { 0, };
    context->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Bind the terrain index buffer to the input assembler stage
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Tell the input assembler stage which primitive topology to use
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);    

    //Bind the SRV of the textures to the effect variable
	V(g_gameEffect.diffuseEV->SetResource(diffuseTextureSRV));
	V(g_gameEffect.normalMap->SetResource(normalTextureSRV));
	V(g_gameEffect.heightMap->SetResource(heightMapSRV));

	V(g_gameEffect.Resolution->SetInt(imgWidth));

    // Apply the rendering pass in order to submit the necessary render state changes to the device
    V(pass->Apply(0, context));

    // DrawIndexed
	int indicesSize = (int)(((imgHeight - 1)*(imgWidth - 1)) * 2 * 3);
	context->DrawIndexed(indicesSize,0,0);
}
