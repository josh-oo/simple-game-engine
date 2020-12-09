//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------
Buffer<float> g_HeightMap;
Texture2D     g_Diffuse; // Material albedo for diffuse lighting
Texture2D	  g_NormalMap;

Texture2D	g_specular;
Texture2D	g_glow;

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
    float4  g_LightDir; // Object space
	int g_TerrainRes;
};

cbuffer cbChangesEveryFrame
{
	matrix  g_WorldNormals; //Normal map
    matrix  g_World;
    matrix  g_WorldViewProjection;
    float   g_Time;

	float4	g_cameraPosWorld;
	//matrix	g_meshpass;
};

cbuffer cbUserChanges
{
};


//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct T3dVertexVSIn {
	 float3 Pos : POSITION; //Position in object space 
	 float2 Tex : TEXCOORD; //Texture coordinate 
	 float3 Nor : NORMAL; //Normal in object space 
     float3 Tan : TANGENT; //Tangent in object space (not used in Ass. 5)
};

struct T3dVertexPSIn {
	  float4 Pos : SV_POSITION; //Position in clip space  
      float2 Tex : TEXCOORD; //Texture coordinate  
 float3 PosWorld : POSITION; //Position in world space 
 float3 NorWorld : NORMAL; //Normal in world space  
 float3 TanWorld : TANGENT; //Tangent in world space (not used in Ass. 5) 
};

struct PosNorTex
{
    float4 Pos : SV_POSITION;
    float4 Nor : NORMAL;
    float2 Tex : TEXCOORD;
};

struct PosTexLi
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
    float   Li : LIGHT_INTENSITY;
	float3 normal: NORMAL;
};

struct PosTex
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState rsDefault {
};

RasterizerState rsCullFront {
    CullMode = Front;
};

RasterizerState rsCullBack {
    CullMode = Back;
};

RasterizerState rsCullNone {
	CullMode = None; 
};

RasterizerState rsLineAA {
	CullMode = None; 
	AntialiasedLineEnable = true;
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

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};


//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

PosTexLi SimpleVS(PosNorTex Input) {
    PosTexLi output = (PosTexLi) 0;

    // Transform position from object space to homogenious clip space
    output.Pos = mul(Input.Pos, g_WorldViewProjection);

    // Pass trough normal and texture coordinates
    output.Tex = Input.Tex;

    // Calculate light intensity
    output.normal = normalize(mul(Input.Nor, g_World).xyz); // Assume orthogonal world matrix
    output.Li = saturate(dot(output.normal, g_LightDir.xyz));
        
    return output;
}

float4 SimplePS(PosTexLi Input) : SV_Target0 {
    // Perform lighting in object space, so that we can use the input normal "as it is"
    //float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
    float4 matDiffuse = g_Diffuse.Sample(samLinearClamp, Input.Tex);
    return float4(matDiffuse.rgb * Input.Li, 1);
	//return float4(Input.normal, 1);
}

PosTex TerrainVS(in uint VertexID : SV_VertexID) {
	PosTex output = (PosTex)0;
	int y = VertexID / g_TerrainRes;// y value of indexed vertex
	int x = VertexID - (y*g_TerrainRes);// x value of indexed vertex

	output.Tex.x = x / (float)(g_TerrainRes - 1);//x texure coordinate between 0 and 1
	output.Tex.y = y / (float)(g_TerrainRes - 1);//y texure coordinate between 0 and 1

	output.Pos.y = g_HeightMap[VertexID];
	//x and y values between 0 and 1 later scaled by units given in game.cfg
	//x and y values shift by 0.5 to center origin
	output.Pos.x = output.Tex.x - 0.5f;
	output.Pos.z = output.Tex.y - 0.5f;
	output.Pos.w = 1.0f;

	output.Pos = mul(output.Pos, g_WorldViewProjection);//Apply camera change

	return output;
}

float4 TerrainPS(in PosTex Input) : SV_Target0 {
	float3 n = (float3)0;
	n.xz = g_NormalMap.Sample(samAnisotropic,Input.Tex);
	n.xz = (n.xz * 2) - 1;
	n.y = sqrt(1 - n.x*n.x - n.z * n.z);

	n = normalize(mul(n, g_World).xyz);

	float4 matDiffuse = g_Diffuse.Sample(samLinearClamp, Input.Tex);

	float i = saturate(dot(n, g_LightDir.xyz));
	if (i < 0.05) {
		i = 0.05;
	}
	return float4(matDiffuse.rgb*i, 1);
}

float3 dehom_1(float4 vec)
{
    return 1 / vec.w * float3(vec.x, vec.y, vec.z);
}

float3 dehom_0(float4 vec)
{
    return float3(vec.x, vec.y, vec.z);
}

T3dVertexPSIn MeshVS (in T3dVertexVSIn VSInput)
{
	/*PosNTex*/
    T3dVertexPSIn output = (T3dVertexPSIn) 0;
    float4 inputW = float4(VSInput.Pos.xyz, 1.0);

    output.Pos = mul(inputW, g_WorldViewProjection);
    output.Tex = VSInput.Tex;

	/*positionToWorld*/
    float4 transformedAccWorld = mul(inputW, g_World);
    output.PosWorld = dehom_1(transformedAccWorld);

	/*normalsToWorld*/
    inputW = float4(VSInput.Nor.xyz, 0);
    
    float4 transformedAccWorldNormals = mul(inputW, g_WorldNormals);
    float3 cutNorm = dehom_0(transformedAccWorldNormals);
    output.NorWorld = normalize(cutNorm);

	/*tangentsToWorld*/
    inputW = float4(VSInput.Tan.xyz, 0);
    float4 transformedAccTangents = mul(inputW, g_World);
    float3 cutTan = dehom_0(transformedAccTangents);
    output.TanWorld = normalize(cutTan);

    return output;
}

float4 MeshPS(in T3dVertexPSIn PSInput) : SV_Target0{

    
    //declaration of c's
    float cd = 0.4;
    float cs = 0.4;
    float ca = 0.3;
    float cg = 0.1;


	float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, PSInput.Tex);
    float4 matSpecular = g_specular.Sample(samAnisotropic, PSInput.Tex);
    float4 matGlow = g_glow.Sample(samAnisotropic, PSInput.Tex);
    float4 colLight = { 1, 1, 1, 1 };
    float4 colLightAmbient = { 1, 1, 1, 1 };


    //float4 n = normalize(float4(PSInput.NorWorld.x, PSInput.NorWorld.y, PSInput.NorWorld.z, 1));
    float3 n = normalize(PSInput.NorWorld);
    float3 l = g_LightDir.xyz;
    float3 r = reflect(-l, n);
    float3 v = normalize(g_cameraPosWorld.xyz - PSInput.PosWorld);

    int specExp = 5;
    float4 result =   (cd * matDiffuse * saturate(dot(n, l)) * colLight)
                    + (cs * matSpecular * pow(saturate(dot(r, v)), specExp) * colLight)
                    + (ca * matDiffuse * colLightAmbient)
                    + (cg * matGlow);
    return result;
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, TerrainPS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }

	pass P1_Mesh
	{
		SetVertexShader(CompileShader(vs_4_0, MeshVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, MeshPS()));

		SetRasterizerState(rsCullBack);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}
