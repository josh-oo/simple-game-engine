#include "SpriteRenderer.h"
#include "DXUT.h"
#include "d3dx11effect.h"
#include "SDKmisc.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <DDSTextureLoader.h>
#include "DirectXTex.h"

ID3DX11EffectTechnique*                 technique; // One technique to render the effect
ID3DX11EffectPass*                      passSprite; // One rendering pass of the technique

ID3DX11EffectMatrixVariable*			g_ViewProjection;
ID3DX11EffectVectorVariable*			g_right;
ID3DX11EffectVectorVariable*			g_up;

ID3DX11EffectShaderResourceVariable*    diffuseEV;

SpriteRenderer::SpriteRenderer(const std::vector<std::wstring>& textureFilenames)
{
	m_textureFilenames = textureFilenames;
	m_pEffect = nullptr;
	m_spriteSRV;
	m_spriteCountMax = 1024;
	m_pVertexBuffer = nullptr;
	m_pInputLayout = nullptr;
	technique = nullptr;
	passSprite = nullptr;
	g_ViewProjection = nullptr;
}

SpriteRenderer::~SpriteRenderer()
{
	delete m_pEffect;
	delete m_pVertexBuffer;
	delete m_pInputLayout;
}

HRESULT SpriteRenderer::reloadShader(ID3D11Device * pDevice)
{
	HRESULT hr;
	WCHAR path[MAX_PATH];

	V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"shader\\SpriteRenderer.fxo"));
	std::ifstream is(path, std::ios_base::binary);
	is.seekg(0, std::ios_base::end);
	std::streampos pos = is.tellg();
	is.seekg(0, std::ios_base::beg);
	std::vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();

	V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, pDevice, &m_pEffect));
	assert(m_pEffect->IsValid());

	// Obtain the effect technique
	SAFE_GET_TECHNIQUE(m_pEffect, "Sprite_Render", technique);

	SAFE_GET_TECHNIQUE(m_pEffect, "Sprite_Render", technique);
	SAFE_GET_PASS(technique, "P_Sprite", passSprite);

	SAFE_GET_MATRIX(m_pEffect, "g_ViewProjection", g_ViewProjection);
	SAFE_GET_VECTOR(m_pEffect, "camera_Right", g_right);
	SAFE_GET_VECTOR(m_pEffect, "camera_Up", g_up);

	SAFE_GET_RESOURCE(m_pEffect, "g_Sprites", diffuseEV);

	//Get the Pass from fx file
	SAFE_GET_PASS(technique, "P_Sprite", passSprite);

	return S_OK;
}

void SpriteRenderer::releaseShader()
{
	SAFE_RELEASE(m_pEffect);
}

HRESULT SpriteRenderer::create(ID3D11Device * pDevice)
{
	HRESULT hr;

	//Sprite Vertex buffer
	D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = m_spriteCountMax * sizeof(SpriteVertex); //The size in bytes of the triangle array
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	V(pDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer));

	//Sprite Vertex input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"RADIUS", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ALPHA", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXTUREINDEX", 0, DXGI_FORMAT_R32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);
	std::cout << "numElements: " << numElements << std::endl;
	D3DX11_PASS_DESC pd;


	V_RETURN(passSprite->GetDesc(&pd));

	V_RETURN(pDevice->CreateInputLayout(layout, numElements, pd.pIAInputSignature,
		pd.IAInputSignatureSize, &m_pInputLayout));

	// Load color texture (color map)
	for (int i = 0; i < m_textureFilenames.size(); i++) {
		std::wstring col = m_textureFilenames.at(i);
		ID3D11ShaderResourceView* diffuseTextureSRV;
		V(DirectX::CreateDDSTextureFromFile(pDevice, col.c_str(), nullptr, &diffuseTextureSRV));
		m_spriteSRV.push_back(diffuseTextureSRV);
		if (hr != S_OK) {
			MessageBoxA(NULL, "Could not load texture \"resources\\sprite.dds\"", "Invalid texture", MB_ICONERROR | MB_OK);
			return hr;
		}
	}

	return S_OK;
}

void SpriteRenderer::destroy()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);

	SAFE_RELEASE(technique);
	SAFE_RELEASE(passSprite);

	for (int i = 0; i < m_spriteSRV.size(); i++) {
		SAFE_RELEASE(m_spriteSRV.at(i));
	}
}

void SpriteRenderer::renderSprites(ID3D11DeviceContext * context, const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera & camera)
{
	HRESULT hr;
	if (sprites.size() > 0)
	{
		D3D11_BOX box;
		ZeroMemory(&box, sizeof(D3D11_BOX));
		box.left = 0; box.right = sprites.size() * sizeof(SpriteVertex);
		box.top = 0; box.bottom = 1;
		box.front = 0; box.back = 1;
		context->UpdateSubresource(m_pVertexBuffer, 0, &box, sprites.data(), 0, 0);
		unsigned int strides[] = { sizeof(SpriteVertex), }, offsets[] = { 0, };

		context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, strides, offsets);
		context->IASetInputLayout(m_pInputLayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		DirectX::XMMATRIX const view = camera.GetViewMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
		DirectX::XMMATRIX const proj = camera.GetProjMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb147302%28v=vs.85%29.aspx
		DirectX::XMMATRIX worldViewProj = view * proj;

		DirectX::XMVECTOR cameraRight = camera.GetWorldRight();
		DirectX::XMVECTOR cameraUp = camera.GetWorldUp();


		V(g_ViewProjection->SetMatrix((float*)&worldViewProj));
		V(g_right->SetFloatVector((float*)&cameraRight));
		V(g_up->SetFloatVector((float*)&cameraUp));
		V(diffuseEV->SetResourceArray(m_spriteSRV.data(),0,m_spriteSRV.size()));


		passSprite->Apply(0, context);

		context->Draw(sprites.size(), 0);
	}
}
