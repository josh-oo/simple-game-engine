#pragma once

#include <string>
#include <vector>

#include <DXUT.h>
#include <DXUTcamera.h>

#include <d3dx11effect.h>

// Convenience macros for safe effect variable retrieval
#define SAFE_GET_PASS(Technique, name, var)   {assert(Technique!=NULL); var = Technique->GetPassByName( name );						assert(var->IsValid());}
#define SAFE_GET_TECHNIQUE(effect, name, var) {assert(effect!=NULL); var = effect->GetTechniqueByName( name );						assert(var->IsValid());}
#define SAFE_GET_SCALAR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsScalar();			assert(var->IsValid());}
#define SAFE_GET_VECTOR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsVector();			assert(var->IsValid());}
#define SAFE_GET_MATRIX(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsMatrix();			assert(var->IsValid());}
#define SAFE_GET_SAMPLER(effect, name, var)   {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsSampler();			assert(var->IsValid());}
#define SAFE_GET_RESOURCE(effect, name, var)  {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsShaderResource();	assert(var->IsValid());}



struct SpriteVertex
{
	DirectX::XMFLOAT3 position;     // world-space position (sprite center)
	float radius;                   // world-space radius (= half side length of the sprite quad)
	float lifetime;
	float alpha;
	int textureIndex;               // which texture to use (out of SpriteRenderer::m_spriteSRV)
	float distanceToCamera;
	SpriteVertex(DirectX::XMFLOAT3 pos, float rad, int texInd) { position = pos; radius = rad; textureIndex = texInd; };
	SpriteVertex() {};
};

class SpriteRenderer
{
public:
	// Constructor: Create a SpriteRenderer with the given list of textures.
	// The textures are *not* be created immediately, but only when create is called!
	SpriteRenderer(const std::vector<std::wstring>& textureFilenames);
	// Destructor does nothing. Destroy and ReleaseShader must be called first!
	~SpriteRenderer();

	// Load/reload the effect. Must be called once before create!
	HRESULT reloadShader(ID3D11Device* pDevice);
	// Release the effect again.
	void releaseShader();

	// Create all required D3D resources (textures, buffers, ...).
	// reloadShader must be called first!
	HRESULT create(ID3D11Device* pDevice);
	// Release D3D resources again.
	void destroy();

	// Render the given sprites. They must already be sorted into back-to-front order.
	void renderSprites(ID3D11DeviceContext* context, const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera& camera);

private:
	std::vector<std::wstring> m_textureFilenames;

	// Rendering effect (shaders and related GPU state). Created/released in Reload/ReleaseShader.
	ID3DX11Effect* m_pEffect;

	// Sprite textures and corresponding shader resource views.
	//std::vector<ID3D11Texture2D*>          m_spriteTex;       // You may not need this if you use CreateDDSTExtureFromFile!
	std::vector<ID3D11ShaderResourceView*> m_spriteSRV;

	// Maximum number of allowed sprites, i.e. size of the vertex buffer.
	size_t m_spriteCountMax;
	// Vertex buffer for sprite vertices, and corresponding input layout.
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11InputLayout* m_pInputLayout;

	//ID3D11Texture2D*                        diffuseTexture;
	//ID3D11ShaderResourceView*               diffuseTextureSRV;
};
