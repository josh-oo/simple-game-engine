Texture2DArray     g_Sprites[4];

cbuffer cbChangesEveryFrame
{
    matrix g_ViewProjection;
    float3 camera_Right;
    float3 camera_Up;
};

struct SpriteVertex
{
    float3 position : POSITION;
    float radius : RADIUS;
	float t : LIFETIME;
    float a : ALPHA;
    int textureIndex : TEXTUREINDEX;
};

struct PSVertex
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD;
	float t : LIFETIME;
    float a : ALPHA;
	int textureIndex : TEXTUREINDEX;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

BlendState BSBlendOver {
   // AlphaToCoverageEnable = TRUE;
	BlendEnable[0] = TRUE;
	SrcBlend[0] = SRC_ALPHA;
	SrcBlendAlpha[0] = ONE;
	DestBlend[0] = INV_SRC_ALPHA;
	DestBlendAlpha[0] = INV_SRC_ALPHA;
};

RasterizerState rsCullNone
{
    CullMode = None;
};

//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------
DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};


SpriteVertex SpriteVS(in SpriteVertex vertex)
{
    return vertex;
}

[maxvertexcount(4)]
void SpriteGS(point SpriteVertex vertex[1], inout TriangleStream<PSVertex> stream)
{
	SpriteVertex vIn = vertex[0];
	float radius = vIn.radius;
    PSVertex v;
	v.t = vIn.t;
    v.a = vIn.a;
	v.textureIndex = vIn.textureIndex;
	v.position = float4((vIn.position - camera_Right * radius + camera_Up * radius).xyz,1);//= float4(vIn.position.x - vIn.radius, vIn.position.y, vIn.position.z + vIn.radius, 1);
	v.position = mul(v.position, g_ViewProjection);
	v.tex = float2(0,0);
	stream.Append(v);
	v.position = float4((vIn.position + camera_Right * radius + camera_Up * radius).xyz,1);
	v.position = mul(v.position, g_ViewProjection);
	v.tex = float2(1, 0);
	stream.Append(v);
	v.position = float4((vIn.position - camera_Right * radius - camera_Up * radius).xyz,1);
	v.position = mul(v.position, g_ViewProjection);
	v.tex = float2(0, 1);
	stream.Append(v);
	v.position = float4((vIn.position + camera_Right * radius - camera_Up * radius).xyz,1);
	v.position = mul(v.position, g_ViewProjection);
	v.tex = float2(1, 1);
	stream.Append(v);

}

float4 SpritePS(PSVertex psVert) : SV_Target0
{
    switch (psVert.textureIndex)
    {
        case 0:
            return psVert.a * g_Sprites[0].Sample(samAnisotropic, float3(psVert.tex.xy, 0));
        case 1:
            return psVert.a * g_Sprites[1].Sample(samAnisotropic, float3(psVert.tex.xy, 0));
        case 2:
            return psVert.a * g_Sprites[2].Sample(samAnisotropic, float3(psVert.tex.xy, psVert.t * 79));
        case 3:
            return psVert.a * g_Sprites[3].Sample(samAnisotropic, float3(psVert.tex.xy, 0));
    }

	return float4(0,0,0,0);
}

technique11 Sprite_Render
{
    pass P_Sprite
    {
        SetVertexShader(CompileShader(vs_4_0, SpriteVS()));
        SetGeometryShader(CompileShader(gs_4_0, SpriteGS()));
        SetPixelShader(CompileShader(ps_4_0, SpritePS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(BSBlendOver, float4(1.0f, 1.0f, 1.0f, 1.0f), 0xFFFFFFFF);
    }
}