#define _USE_MATH_DEFINES

#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
#include <map>
#include <list>
#include <time.h>
#include <ctime>
#include <cstdlib>
#include <math.h>

#include "dxut.h"
#include "DXUTmisc.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "SDKmisc.h"

#include "d3dx11effect.h"
#include "Terrain.h"
#include "GameEffect.h"
#include "Mesh.h"
#include "ConfigParser.h"
#include "SpriteRenderer.h"
#include "debug.h"



// Help macros
#define DEG2RAD( a ) ( (a) * XM_PI / 180.f )

using namespace std;
using namespace DirectX;
//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

float									pi = M_PI;
// Camera
struct CAMERAPARAMS {
	float   fovy;
	float   aspect;
	float   nearPlane;
	float   farPlane;
}                                       g_cameraParams;
float                                   g_cameraMoveScaler = 1000.f;
float                                   g_cameraRotateScaler = 0.01f;
CFirstPersonCamera                      g_camera;               // A first person camera

// User Interface
CDXUTDialogResourceManager              g_dialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg                         g_settingsDlg;          // Device settings dialog
CDXUTTextHelper*                        g_txtHelper = NULL;
CDXUTDialog                             g_hud;                  // dialog for standard controls
CDXUTDialog                             g_sampleUI;             // dialog for sample specific controls

//ID3D11InputLayout*                      g_terrainVertexLayout; // Describes the structure of the vertex buffer to the input assembler stage


bool                                    g_terrainSpinning = false;
bool									noclip = false;
XMMATRIX                                g_terrainWorld; // object- to world-space transformation


// Scene information
XMVECTOR                                g_lightDir;
Terrain									g_terrain;

GameEffect								g_gameEffect; // CPU part of Shader

SpriteRenderer*							g_SpriteRenderer;

ConfigParser							configParser;

float timer;
float coolDownTimerGatling;
float coolDownTimerPlasma;

struct GameObject {
	wstring mesh;
	wstring type;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	float scale;
};

struct EnemyInstance {
	wstring type;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	float remainingHitpoints;
	float distanceTraveled = 0;
};


typedef ConfigParser::EnemyType EnemyType;
typedef ConfigParser::Gun Gun;
typedef ConfigParser::ProjectileSettings ProjectileSettings;

struct Projectile {
	ProjectileSettings Settings;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT3 position;
	float traveledDistance;
	Projectile() { traveledDistance = 0; };
};

struct ExplosionInstance {
	XMFLOAT3 position;
	
	int textureIndex;
	float passedTime;
	float timeStepSize;
};

struct Particle {
	XMFLOAT3 position;
	XMFLOAT3 direction;
	float duration;
	signed int spriteIndex;
	float passedTime;
	float timeStepSize;
	Particle() {};
	Particle(XMFLOAT3 _position, XMFLOAT3 _direction, float _duration, signed int _spriteIndex) {
		position = _position;
		direction = _direction;
		duration = _duration;
		spriteIndex = _spriteIndex;
		passedTime = 0;
	}
};

list<EnemyInstance> enemyInstances;
list<Projectile> projectiles;
list<ExplosionInstance> explosions;
list<Particle> particles;
map<wstring, Mesh*> g_Meshes;
map<wstring, EnemyType*> g_Enemies;
vector<GameObject> gameObjects;

Gun Gatling;
Gun Plasma;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLESPIN          4
#define IDC_RELOAD_SHADERS		101

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *, UINT , const CD3D11EnumDeviceInfo *,
                                       DXGI_FORMAT, bool, void* );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext );

void InitApp();
void DeinitApp();
void RenderText();

bool sortSprites(const SpriteVertex& i, const SpriteVertex& j) { return (i.distanceToCamera < j.distanceToCamera); }

void ReleaseShader();
HRESULT ReloadShader(ID3D11Device* pd3dDevice);

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Old Direct3D Documentation:
    // Start > All Programs > Microsoft DirectX SDK (June 2010) > Windows DirectX Graphics Documentation

    // DXUT Documentaion:
    // Start > All Programs > Microsoft DirectX SDK (June 2010) > DirectX Documentation for C++ : The DirectX Software Development Kit > Programming Guide > DXUT
	
    // New Direct3D Documentaion (just for reference, use old documentation to find explanations):
    // http://msdn.microsoft.com/en-us/library/windows/desktop/hh309466%28v=vs.85%29.aspx


    // Initialize COM library for windows imaging components
    /*HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (hr != S_OK)
    {
        MessageBox(0, L"Error calling CoInitializeEx", L"Error", MB_OK | MB_ICONERROR);
        exit(-1);
    }*/


    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    //DXUTSetIsInGammaCorrectMode(false);

    InitApp();
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"All bugs belong to Joshua" ); // You may change the title

    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1280, 720 );

    DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown();

	DeinitApp();

    return DXUTGetExitCode();
}

float getDistanceToCamera(XMFLOAT3 position)
{
	float vecLength = sqrt(position.x*position.x + position.y*position.y + position.z*position.z);
	float xCam = DirectX::XMVectorGetX(g_camera.GetWorldAhead());
	float yCam = DirectX::XMVectorGetY(g_camera.GetWorldAhead());
	float zCam = DirectX::XMVectorGetZ(g_camera.GetWorldAhead());
	XMFLOAT3 camera = XMFLOAT3(xCam, yCam, zCam);
	XMFLOAT3 camToPos = XMFLOAT3(position.x-camera.x, position.y - camera.y, position.z - camera.z);
	float dist = sqrt(camToPos.x*camToPos.x + camToPos.y*camToPos.y + camToPos.z*camToPos.z);/*vecLength * camLength *
			(atan2(XMVectorGetY(g_camera.GetWorldAhead()), XMVectorGetX(g_camera.GetWorldAhead())) - atan2(position.y, position.x));*/
	return dist;
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    HRESULT hr;
    WCHAR path[MAX_PATH];

    // Parse the config file

    V(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"game.cfg"));
	char pathA[MAX_PATH];
	size_t size;
	wcstombs_s(&size, pathA, path, MAX_PATH);

	/*Loading values form game.cfg*/
	configParser.loadNewType(pathA);
	timer = configParser.getSpawner().interval;
	/*Values stored in configParser*/

	for (std::pair<wstring, ConfigParser::EnemyType> element : configParser.getEnemyTypes()) {
		EnemyType *enemyType = new EnemyType();
		enemyType->hitpoints = element.second.hitpoints;
		enemyType->mesh = element.second.mesh;
		enemyType->positionX = element.second.positionX;
		enemyType->positionY = element.second.positionY;
		enemyType->positionZ = element.second.positionZ;
		enemyType->rotationX = element.second.rotationX;
		enemyType->rotationY = element.second.rotationY;
		enemyType->rotationZ = element.second.rotationZ;
		enemyType->scale = element.second.scale;
		enemyType->unitSize = element.second.unitSize;
		enemyType->unitSpeed = element.second.unitSpeed;
		enemyType->enemyTypeName = element.second.enemyTypeName;

		g_Enemies.insert(pair<wstring, EnemyType*>(element.second.enemyTypeName, enemyType));

		wcout << element.first << " loaded in configParser" <<  " mesh:" << element.second.mesh <<endl;
	}

	wstring r = L"resources\\";
	for (int i = 0; i < configParser.getMeshes().size(); i++) {
		wstring name = configParser.getMeshes().at(i).name;

		wstring objPath =  configParser.getMeshes().at(i).objPath;
		wstring diffusePath = configParser.getMeshes().at(i).diffusePath;
		wstring specularPath = configParser.getMeshes().at(i).specularPath;
		wstring glowPath = configParser.getMeshes().at(i).glowPath;

		if (objPath != L"-") objPath = r + objPath;
		if (diffusePath != L"-") diffusePath = r + diffusePath;
		if (specularPath != L"-") specularPath = r + specularPath;
		if (glowPath != L"-") glowPath = r + glowPath;

		wcout << " path read: " << r + configParser.getMeshes().at(i).objPath << endl;
		Mesh *mesh = new Mesh(objPath,diffusePath,specularPath,glowPath);

		g_Meshes.insert(pair<wstring, Mesh*>(name, mesh));
	}
	g_Meshes;
	for (int i = 0; i < configParser.getObjects().size(); i++) {
		//Load all GameObjects
		GameObject obj;
		obj.mesh = configParser.getObjects().at(i).name;
		obj.type = configParser.getObjects().at(i).type;
		obj.position.x = configParser.getObjects().at(i).posX;
		obj.position.y = configParser.getObjects().at(i).posY;
		obj.position.z = configParser.getObjects().at(i).posZ;
		obj.rotation.x = configParser.getObjects().at(i).rotationX;
		obj.rotation.y = configParser.getObjects().at(i).rotationY;
		obj.rotation.z = configParser.getObjects().at(i).rotationZ;
		obj.scale = configParser.getObjects().at(i).scale;
		gameObjects.push_back(obj);
	}

    // Intialize the user interface

    g_settingsDlg.Init( &g_dialogResourceManager );
    g_hud.Init( &g_dialogResourceManager );
    g_sampleUI.Init( &g_dialogResourceManager );

    g_hud.SetCallback( OnGUIEvent );
    int iY = 30;
    int iYo = 26;
    g_hud.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22 );
    g_hud.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3 );
    g_hud.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2 );

	g_hud.AddButton (IDC_RELOAD_SHADERS, L"Reload shaders (F5)", 0, iY += 24, 170, 22, VK_F5);
    
    g_sampleUI.SetCallback( OnGUIEvent ); iY = 10;
    iY += 24;
    g_sampleUI.AddCheckBox( IDC_TOGGLESPIN, L"Toggle Spinning", 0, iY += 24, 125, 22, g_terrainSpinning );   

	//Sprites
	//std::vector<std::wstring> texFiles;
	//texFiles.push_back(L"resources\\parTrailGatlingDiffuse.DDS");
	//texFiles.push_back(L"resources\\parTrailPlasmaDiffuse.DDS");
	g_SpriteRenderer = new SpriteRenderer(configParser.getTextures());

	Gatling = configParser.getGatlingGun();
	Plasma = configParser.getPlasmaGun();
	cout << "Initial Gatling CoolDown: " << Gatling.cooldown;
	coolDownTimerGatling = Gatling.cooldown;
	cout << "Initial Plasma CoolDown: " << Plasma.cooldown;
	coolDownTimerPlasma = Plasma.cooldown;

	//ready to fire at beginning
	coolDownTimerGatling = 0;
	coolDownTimerPlasma = 0;

}

void DeinitApp()
{
	std::map<wstring, Mesh*>::iterator it = g_Meshes.begin();
	for (std::pair<wstring, Mesh*> element : g_Meshes) {
		// Accessing KEY from element
		std::wstring name = element.first;
		// Accessing VALUE from element.
		Mesh* mesh = element.second;

		SAFE_DELETE(mesh);
		wcout << name << " deleted!" << endl;
	}
	for (std::pair<wstring, EnemyType*> element : g_Enemies) {
		// Accessing KEY from element
		std::wstring name = element.first;
		// Accessing VALUE from element.
		EnemyType* enemy = element.second;

		SAFE_DELETE(enemy);
		wcout << name << " deleted!" << endl;
	}

	delete g_SpriteRenderer;
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_txtHelper->Begin();
    g_txtHelper->SetInsertionPos( 5, 5 );
    g_txtHelper->SetForegroundColor(DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
    g_txtHelper->DrawTextLine( DXUTGetFrameStats(true)); //DXUTIsVsyncEnabled() ) );
    g_txtHelper->DrawTextLine( DXUTGetDeviceStats() );
    g_txtHelper->End();
}



//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *, UINT, const CD3D11EnumDeviceInfo *,
        DXGI_FORMAT, bool, void* )
{
    return true;
}

//--------------------------------------------------------------------------------------
// Specify the initial device settings
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	UNREFERENCED_PARAMETER(pDeviceSettings);
	UNREFERENCED_PARAMETER(pUserContext);

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if (pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE)
        {
            DXUTDisplaySwitchingToREFWarning();
        }
    }
    //// Enable anti aliasing
    pDeviceSettings->d3d11.sd.SampleDesc.Count = 4;
    pDeviceSettings->d3d11.sd.SampleDesc.Quality = 1;

    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	UNREFERENCED_PARAMETER(pBackBufferSurfaceDesc);
	UNREFERENCED_PARAMETER(pUserContext);

    HRESULT hr;


    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext(); // http://msdn.microsoft.com/en-us/library/ff476891%28v=vs.85%29
    V_RETURN( g_dialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_settingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_txtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_dialogResourceManager, 15 );

    V_RETURN( ReloadShader(pd3dDevice) );
    
    
    // Initialize the camera
	XMVECTOR vEye = DirectX::XMVectorSet(0.0f, configParser.getTerrainHeight()*2, -500.0f, 0.0f);   // Camera eye is here
    XMVECTOR vAt = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);               // ... facing at this position
    g_camera.SetViewParams(vEye, vAt); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
	g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);

	// Define the input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] = // http://msdn.microsoft.com/en-us/library/bb205117%28v=vs.85%29.aspx
	{
		{ "SV_POSITION",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",         0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",       0, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );
	// Create the input layout
    D3DX11_PASS_DESC pd;
	V_RETURN(g_gameEffect.pass0->GetDesc(&pd));

	Mesh::createInputLayout(pd3dDevice, g_gameEffect.meshPass1);//Just call it once because of static type
	for (std::pair<wstring, Mesh*> element : g_Meshes) {
		// Accessing KEY from element
		std::wstring name = element.first;
		// Accessing VALUE from element.

		element.second->create(pd3dDevice);
		wcout << name << " created!" << endl;
	}


	// Create the terrain with the configParser
	V_RETURN(g_terrain.create(pd3dDevice, configParser));
	
	//sprites
	V_RETURN(g_SpriteRenderer->create(pd3dDevice));


    return S_OK;
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	UNREFERENCED_PARAMETER(pUserContext);

    g_dialogResourceManager.OnD3D11DestroyDevice();
    g_settingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    //SAFE_RELEASE( g_terrainVertexLayout );
    
	// Destroy the terrain
	g_terrain.destroy();

	//Destroy the meshes
	Mesh::destroyInputLayout();
	for (std::pair<wstring, Mesh*> element : g_Meshes) {
		// Accessing KEY from element
		std::wstring name = element.first;
		// Accessing VALUE from element.
		element.second->destroy();

		wcout << name << " destroyed!" << endl;
	}

	//sprites
	g_SpriteRenderer->destroy();

	

    SAFE_DELETE( g_txtHelper );
    ReleaseShader();

}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	UNREFERENCED_PARAMETER(pSwapChain);
	UNREFERENCED_PARAMETER(pUserContext);

    HRESULT hr;
    
    // Intialize the user interface

    V_RETURN( g_dialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_settingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    g_hud.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_hud.SetSize( 170, 170 );
    g_sampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
    g_sampleUI.SetSize( 170, 300 );

    // Initialize the camera

    g_cameraParams.aspect = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_cameraParams.fovy = 0.785398f;
    g_cameraParams.nearPlane = 1.f;
    g_cameraParams.farPlane = 5000.f;

    g_camera.SetProjParams(g_cameraParams.fovy, g_cameraParams.aspect, g_cameraParams.nearPlane, g_cameraParams.farPlane);
	g_camera.SetEnablePositionMovement(true);
	g_camera.SetRotateButtons(true, false, false);
	g_camera.SetScalers( g_cameraRotateScaler, g_cameraMoveScaler );
	g_camera.SetDrag( true );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	UNREFERENCED_PARAMETER(pUserContext);
    g_dialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Loads the effect from file
// and retrieves all dependent variables
//--------------------------------------------------------------------------------------
HRESULT ReloadShader(ID3D11Device* pd3dDevice)
{
    assert(pd3dDevice != NULL);

    HRESULT hr;

    ReleaseShader();
	V_RETURN(g_gameEffect.create(pd3dDevice));
	V_RETURN(g_SpriteRenderer->reloadShader(pd3dDevice));

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Release resources created in ReloadShader
//--------------------------------------------------------------------------------------
void ReleaseShader()
{
	g_gameEffect.destroy();
	g_SpriteRenderer->releaseShader();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
	UNREFERENCED_PARAMETER(pUserContext);

    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_dialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_settingsDlg.IsActive() )
    {
        g_settingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_hud.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_sampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
        
    // Use the mouse weel to control the movement speed
    if(uMsg == WM_MOUSEWHEEL) {
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        g_cameraMoveScaler *= (1 + zDelta / 500.0f);
        if (g_cameraMoveScaler < 0.1f)
          g_cameraMoveScaler = 0.1f;
        g_camera.SetScalers(g_cameraRotateScaler, g_cameraMoveScaler);
    }

    // Pass all remaining windows messages to camera so it can respond to user input
	// only do so if noclip is enabled
	if (noclip) {
		g_camera.HandleMessages(hWnd, uMsg, wParam, lParam);
	}

    return 0;
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	UNREFERENCED_PARAMETER(nChar);
	UNREFERENCED_PARAMETER(bKeyDown);
	UNREFERENCED_PARAMETER(bAltDown);
	UNREFERENCED_PARAMETER(pUserContext);

	//Enable noclip when the C-key is pressed
	if (nChar == 'C' && bKeyDown) { noclip = !noclip; }

	if (nChar == '3' && Plasma.enabled && bKeyDown)
	{
		Plasma.isShooting = true;
	}
	if (nChar == '1' && Gatling.enabled && bKeyDown)
	{
		Gatling.isShooting = true;
	}

}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	UNREFERENCED_PARAMETER(nEvent);
	UNREFERENCED_PARAMETER(pControl);
	UNREFERENCED_PARAMETER(pUserContext);

    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_settingsDlg.SetActive( !g_settingsDlg.IsActive() ); break;
        case IDC_TOGGLESPIN:
            g_terrainSpinning = g_sampleUI.GetCheckBox( IDC_TOGGLESPIN )->GetChecked();
            break;
		case IDC_RELOAD_SHADERS:
			ReloadShader(DXUTGetD3D11Device ());
			break;
    }
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	UNREFERENCED_PARAMETER(pUserContext);
    // Update the camera's position based on user input 
    g_camera.FrameMove( fElapsedTime );
    
    // Initialize the terrain world matrix
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206365%28v=vs.85%29.aspx
    
	// Start with identity matrix
    g_terrainWorld = XMMatrixIdentity();

	//Scale to input values from cfg file
	g_terrainWorld *= XMMatrixScaling(configParser.getTerrainWidth(), configParser.getTerrainHeight(), configParser.getTerrainDepth());

    
    if( g_terrainSpinning ) 
    {
		// If spinng enabled, rotate the world matrix around the y-axis
        g_terrainWorld *= XMMatrixRotationY(30.0f * DEG2RAD((float)fTime)); // Rotate around world-space "up" axis
    }

	// Set the light vector
    g_lightDir = DirectX::XMVectorSet(1, 1, 1, 0); // Direction to the directional light in world space    
    g_lightDir = DirectX::XMVector3Normalize(g_lightDir);


	timer = timer - fElapsedTime;
	if(!Gatling.enabled)
		coolDownTimerGatling = coolDownTimerGatling - fElapsedTime;
	if(!Plasma.enabled)
		coolDownTimerPlasma = coolDownTimerPlasma - fElapsedTime;

	if (timer < 0.0f)
	{
		srand(time(nullptr));
		std::map<wstring, EnemyType*>::iterator mapBegin = g_Enemies.begin();
		std::map<wstring, EnemyType*>::iterator mapEnd = g_Enemies.end();
		int distance = std::distance(mapBegin, mapEnd);
		int randEnemyTypeSelection = rand() % distance;
		std::map<wstring, EnemyType*>::iterator selected = mapBegin;
		while (randEnemyTypeSelection > 0)
		{
			++selected;
			--randEnemyTypeSelection;
		}

		//SpawnEnemies
		EnemyType randomType = *(selected->second);

		srand(time(nullptr));
		float random = (float)rand() / (float)RAND_MAX;

		float dist = configParser.getSpawner().maxHeight*configParser.getTerrainHeight()
					- configParser.getSpawner().minHeight*configParser.getTerrainHeight();
		float yPos = random * dist + configParser.getSpawner().minHeight*configParser.getTerrainHeight();
		EnemyInstance newEnemy;

		float rInner = 100;
		float rOuter = 400;

		// Randomize both Circles
		// 3.14 (pi) * 100 * 2 = 628; Value between 0 and 2Pi is demanded
		srand(time(nullptr));
		float randomAlphaInner = ((float) (rand() % 628)) / 100.0f;
		float randomAlphaOuter = ((float) (rand() % 628)) / 100.0f;

		XMFLOAT2 innerPos(rInner * std::sin(randomAlphaInner), rInner * std::cos(randomAlphaInner));
		XMFLOAT2 outerPos(rOuter * std::sin(randomAlphaOuter), rOuter * std::cos(randomAlphaOuter));

		newEnemy.position.x = outerPos.x;
		newEnemy.position.y = yPos;
		newEnemy.position.z = outerPos.y;

		newEnemy.rotation.x = randomType.rotationX;
		newEnemy.rotation.y = randomType.rotationY;
		newEnemy.rotation.z = randomType.rotationZ;

		newEnemy.remainingHitpoints = randomType.hitpoints;
		
		XMFLOAT2 v(innerPos.x - outerPos.x, innerPos.y - outerPos.y);
		XMVECTOR vVec = { v.x, v.y };
		vVec = DirectX::XMVector2Normalize(vVec);
		
		v.x = DirectX::XMVectorGetX(vVec);
		v.y = DirectX::XMVectorGetY(vVec);

		float angleInRadians = atan2(v.x, v.y);
		newEnemy.rotation.y = angleInRadians;

		newEnemy.velocity.x = v.x* randomType.unitSpeed;
		newEnemy.velocity.y = 0;
		newEnemy.velocity.z = v.y * randomType.unitSpeed;


		newEnemy.type = randomType.enemyTypeName;

		enemyInstances.push_back(newEnemy);
		
		timer = configParser.getSpawner().interval;		
	}


	if (coolDownTimerGatling < 0.0f)
	{
		//Gatling ready to fire
		Gatling.enabled = true;
	}

	if (coolDownTimerPlasma < 0.0f)
	{
		//Plasma ready to fire
		Plasma.enabled = true;
	}

	if (Gatling.isShooting)
	{
		//spawn Gatling Projectile
		Projectile projectile;
		projectile.traveledDistance = 0.0f;
		projectile.Settings = Gatling.ProjectileSettings;
		projectile.position.x = Gatling.ProjectileSettings.spawnPosX;
		projectile.position.y = Gatling.ProjectileSettings.spawnPosY;
		projectile.position.z = Gatling.ProjectileSettings.spawnPosZ;

		XMVECTOR pos = { projectile.position.x , projectile.position.y, projectile.position.z };

		XMVECTOR transfPos = DirectX::XMVector3Transform(pos, g_camera.GetWorldMatrix());
		projectile.position = XMFLOAT3(DirectX::XMVectorGetX(transfPos),
			DirectX::XMVectorGetY(transfPos),
			DirectX::XMVectorGetZ(transfPos));

		projectile.velocity = DirectX::XMFLOAT3(DirectX::XMVectorGetX(g_camera.GetWorldAhead()),
			DirectX::XMVectorGetY(g_camera.GetWorldAhead()),
			DirectX::XMVectorGetZ(g_camera.GetWorldAhead()));

		projectile.velocity.x *= Gatling.ProjectileSettings.projectileSpeed;
		projectile.velocity.y *= Gatling.ProjectileSettings.projectileSpeed;
		projectile.velocity.z *= Gatling.ProjectileSettings.projectileSpeed;

		projectiles.push_back(projectile);
		Gatling.isShooting = false;
		Gatling.enabled = false;
		coolDownTimerGatling = Gatling.cooldown;
	}

	if (Plasma.isShooting)
	{
		for (list<Projectile>::iterator iter = projectiles.begin(); iter != projectiles.end(); iter++) {};
		//spawn Plasma 
		Projectile projectile;
		projectile.traveledDistance = 0.0f;

		projectile.Settings = Plasma.ProjectileSettings;

		projectile.position.x = Plasma.ProjectileSettings.spawnPosX;
		projectile.position.y = Plasma.ProjectileSettings.spawnPosY;
		projectile.position.z = Plasma.ProjectileSettings.spawnPosZ;

		XMVECTOR pos = { projectile.position.x , projectile.position.y, projectile.position.z };

		XMVECTOR transfPos = DirectX::XMVector3Transform(pos, g_camera.GetWorldMatrix());
		projectile.position = XMFLOAT3(DirectX::XMVectorGetX(transfPos),
			DirectX::XMVectorGetY(transfPos),
			DirectX::XMVectorGetZ(transfPos));

		projectile.velocity = DirectX::XMFLOAT3(DirectX::XMVectorGetX(g_camera.GetWorldAhead()),
			DirectX::XMVectorGetY(g_camera.GetWorldAhead()),
			DirectX::XMVectorGetZ(g_camera.GetWorldAhead()));

		projectile.velocity.x *= Plasma.ProjectileSettings.projectileSpeed;
		projectile.velocity.y *= Plasma.ProjectileSettings.projectileSpeed;
		projectile.velocity.z *= Plasma.ProjectileSettings.projectileSpeed;

		projectiles.push_back(projectile);
		Plasma.isShooting = false;
		Plasma.enabled = false;
		coolDownTimerPlasma = Plasma.cooldown;
	}


	float rBorder = 800;//Destroy enemies, traveled longer than this value

	for (auto it0 = enemyInstances.begin(); it0 != enemyInstances.end();) {//Animation loop
		float x = fElapsedTime * (*it0).velocity.x;
		float y = fElapsedTime * (*it0).velocity.y;
		float z = fElapsedTime * (*it0).velocity.z;
		if ((*it0).distanceTraveled >= rBorder) {//check for 'out of borders'
			auto it_remove = it0;
			it0++;
			enemyInstances.erase(it_remove);
			continue;
		}

		(*it0).position.x += x;
		(*it0).position.y += y;
		(*it0).position.z += z;
		float length = std::sqrt(x*x + y * y + z * z);//Length of added distance
		(*it0).distanceTraveled += length;

		//Collision detection
		int enemySize = g_Enemies.at((*it0).type)->unitSize;
		XMFLOAT3 posEnemy = (*it0).position;
		for (auto it1 = projectiles.begin(); it1 != projectiles.end();) {
			XMFLOAT3 pos = (*it1).position;

			XMVECTOR c1 = {XMLoadFloat3(&posEnemy)};
			XMVECTOR c2 = { XMLoadFloat3(&pos) };
			
			XMVECTOR c = (c1 - c2);
			float sol = DirectX::XMVectorGetX(c) * DirectX::XMVectorGetX(c) + DirectX::XMVectorGetY(c) * DirectX::XMVectorGetY(c) + DirectX::XMVectorGetZ(c) * DirectX::XMVectorGetZ(c);

			if( sol < (enemySize + 1) * (enemySize + 1)){
				cout << "Collision occured" << endl;
				(*it0).remainingHitpoints -= (*it1).Settings.damage;

				auto it_remove = it1;
				it1++;
				projectiles.erase(it_remove);
				continue;
			}
			it1++;
		}

		if ((*it0).remainingHitpoints <= 0.0f) {
			auto it_remove = it0;
			it0++;
			ExplosionInstance newExplosion;
			newExplosion.passedTime = 0;
			newExplosion.position = it_remove->position;
			newExplosion.textureIndex = configParser.GetExplosion().textureIndex;
			newExplosion.timeStepSize = 1 / (configParser.GetExplosion().duration*55);
			srand(time(NULL));
			int numberofParticles = ((int)rand()) % configParser.getParticleSystem().particleNumber;
			for (int i = 0; i < numberofParticles; i++)
			{
				double theta = rand() % 180;
				double phi = rand() % 360;
				XMFLOAT3 randDir = XMFLOAT3( sin(theta) * cos(phi), sin(theta) * cos(phi), cos(phi));
				randDir.x = abs(randDir.x);
				randDir.y = abs(randDir.y);
				randDir.z = abs(randDir.z);
				randDir.x *= (rand()%5) * 5;
				randDir.y *= (rand() % ((int)((configParser.getParticleSystem().maxSpeed) - (configParser.getParticleSystem().minSpeed)))) + configParser.getParticleSystem().minSpeed;
				randDir.z *= (rand() % 5) * 5;
				Particle P(newExplosion.position, randDir, configParser.getParticleSystem().lifeTime, configParser.getParticleSystem().textureIndex);
				P.timeStepSize = newExplosion.timeStepSize;
				particles.push_back(P);
			}
			enemyInstances.erase(it_remove);
			explosions.push_back(newExplosion);
			//Awesome, explosive, incredible particle Animation
			continue;
		}
		it0++;
	}

	for (std::list<ExplosionInstance>::iterator iter = explosions.begin(); iter != explosions.end(); ) {
		//Update passed explosion in each frame
		iter->passedTime += iter->timeStepSize;
		if (iter->passedTime >= 1.0f)
		{
			//Removal after explosion ended
			std::list<ExplosionInstance>::iterator itRem = iter;
			iter++;
			explosions.erase(itRem);
			continue;
		}
		iter++;
	}

	for (std::list<Particle>::iterator iter = particles.begin(); iter != particles.end(); ) {
		//Update passed explosion in each frame
		iter->passedTime += iter->timeStepSize;

		if (iter->passedTime >= 1.0f)
		{
			//Removal after explosion ended
			std::list<Particle>::iterator itRem = iter;
			iter++;
			particles.erase(itRem);
			continue;
		}

		iter->direction.y -= configParser.getParticleSystem().gravityY;
		XMFLOAT3 veloc_dt = XMFLOAT3((*iter).direction.x * fElapsedTime, (*iter).direction.y * fElapsedTime, (*iter).direction.z * fElapsedTime);

		iter->position = XMFLOAT3((*iter).position.x + veloc_dt.x, (*iter).position.y + veloc_dt.y, (*iter).position.z + veloc_dt.z);;

		iter++;
	}
	
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
        float fElapsedTime, void* pUserContext )
{
	UNREFERENCED_PARAMETER(pd3dDevice);
	UNREFERENCED_PARAMETER(fTime);
	UNREFERENCED_PARAMETER(pUserContext);

    HRESULT hr;

    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_settingsDlg.IsActive() )
    {
        g_settingsDlg.OnRender( fElapsedTime );
        return;
    }     

    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	float clearColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
    pd3dImmediateContext->ClearRenderTargetView( pRTV, clearColor );
        
	if(g_gameEffect.effect == NULL) {
        g_txtHelper->Begin();
        g_txtHelper->SetInsertionPos( 5, 5 );
        g_txtHelper->SetForegroundColor( DirectX::XMVectorSet( 1.0f, 1.0f, 0.0f, 1.0f ) );
        g_txtHelper->DrawTextLine( L"SHADER ERROR" );
        g_txtHelper->End();
        return;
    }

    // Clear the depth stencil
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
    
    // Update variables that change once per frame
    XMMATRIX const view = g_camera.GetViewMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb206342%28v=vs.85%29.aspx
    XMMATRIX const proj = g_camera.GetProjMatrix(); // http://msdn.microsoft.com/en-us/library/windows/desktop/bb147302%28v=vs.85%29.aspx
    XMMATRIX worldViewProj = g_terrainWorld * view * proj;
	V(g_gameEffect.worldEV->SetMatrix( ( float* )&g_terrainWorld ));
	V(g_gameEffect.worldViewProjectionEV->SetMatrix( ( float* )&worldViewProj ));
	V(g_gameEffect.lightDirEV->SetFloatVector( ( float* )&g_lightDir ));
	V(g_gameEffect.worldNormalsMatrix->SetMatrixTranspose((float*)&g_terrainWorld));

    // Set input layout
    pd3dImmediateContext->IASetInputLayout( nullptr );

	g_terrain.render(pd3dImmediateContext, g_gameEffect.pass0);

	//Mesh stuff

	for (int i = 0; i < gameObjects.size(); i++) {
		GameObject obj = gameObjects.at(i);

		XMMATRIX mTrans, mScale, mRot;
		mRot = XMMatrixRotationX((obj.rotation.x*pi)/180);//Degreees converted to radians
		mRot *= XMMatrixRotationY((obj.rotation.y*pi) / 180);
		mRot *= XMMatrixRotationZ((obj.rotation.z*pi) / 180);
		mTrans = XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);
		mScale = XMMatrixScaling(obj.scale, obj.scale, obj.scale);

		XMMATRIX mObj = mScale * mRot * mTrans;

		XMMATRIX mWorld = mObj;
		if (obj.type == L"CockpitObject") mWorld *= g_camera.GetWorldMatrix();
		XMMATRIX mWorldViewProj = mWorld * view * proj;
		

		V(g_gameEffect.cameraPosWorldEV->SetFloatVector((float*)&g_camera.GetEyePt()));
		V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&mWorldViewProj));
		V(g_gameEffect.worldEV->SetMatrix((float*)&mWorld));
		V(g_gameEffect.worldNormalsMatrix->SetMatrixTranspose((float*)&mWorld));

		Mesh *mesh = g_Meshes.at(obj.mesh);

		mesh->render(pd3dImmediateContext, g_gameEffect.meshPass1,
			g_gameEffect.diffuseEV,
			g_gameEffect.specularEV,
			g_gameEffect.glowEV);
	}

	for (auto const& obj : enemyInstances) {
		EnemyType *type = g_Enemies.at(obj.type);

		XMMATRIX mTrans, mScale, mRot;
		mRot = XMMatrixRotationX((type->rotationX*pi) / 180);//Degreees converted to radians
		mRot *= XMMatrixRotationY((type->rotationY*pi) / 180);
		mRot *= XMMatrixRotationZ((type->rotationZ*pi) / 180);
		
		mTrans = XMMatrixTranslation(type->positionX, type->positionY,type->positionZ);
		mScale = XMMatrixScaling(type->scale, type->scale, type->scale);

		XMMATRIX anim;
		anim = XMMatrixRotationX(obj.rotation.x);
		anim *= XMMatrixRotationY(obj.rotation.y);
		anim *= XMMatrixRotationZ(obj.rotation.z);
		anim *= XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);

		XMMATRIX mObj = mScale * mRot * mTrans;

		XMMATRIX mWorld = mObj * anim;
		XMMATRIX mWorldViewProj = mWorld * view * proj;

		V(g_gameEffect.cameraPosWorldEV->SetFloatVector((float*)&g_camera.GetEyePt()));
		V(g_gameEffect.worldViewProjectionEV->SetMatrix((float*)&mWorldViewProj));
		V(g_gameEffect.worldEV->SetMatrix((float*)&mWorld));
		V(g_gameEffect.worldNormalsMatrix->SetMatrixTranspose((float*)&mWorld));

		Mesh *mesh = g_Meshes.at(type->mesh);

		mesh->render(pd3dImmediateContext, g_gameEffect.meshPass1,
			g_gameEffect.diffuseEV,
			g_gameEffect.specularEV,
			g_gameEffect.glowEV);
	}


	std::vector<SpriteVertex> projectileSprites;

	for (list<Projectile>::iterator it = projectiles.begin(); it != projectiles.end();) // Projectile P : projectiles)
	{
		XMFLOAT3 gravity = XMFLOAT3(0, fElapsedTime * (*it).Settings.particleMass, 0);
		(*it).velocity.x -= gravity.x;
		(*it).velocity.y -= gravity.y;
		(*it).velocity.z -= gravity.z;

		XMFLOAT3 veloc_dt = XMFLOAT3((*it).velocity.x * fElapsedTime, (*it).velocity.y * fElapsedTime, (*it).velocity.z * fElapsedTime);

		XMFLOAT3 newPos = XMFLOAT3((*it).position.x + veloc_dt.x, (*it).position.y + veloc_dt.y, (*it).position.z + veloc_dt.z);

		// Calculates distance of projectile from its origin
		XMFLOAT3 projVec = XMFLOAT3();
		projVec.x = newPos.x - (*it).position.x; // Calculate vector between old position and new one
		projVec.y = newPos.y - (*it).position.y;
		projVec.z = newPos.z - (*it).position.z;
		(*it).traveledDistance += sqrtf(projVec.x*projVec.x + projVec.y*projVec.y + projVec.z*projVec.z); // Length of said vector added to total traveled distance

		if ((*it).traveledDistance >= 300) {
			auto it_remove = it;
			it++;
			projectiles.erase(it_remove);
			continue;
		}
		(*it).position = newPos;
		
		SpriteVertex sv;
		sv.distanceToCamera = getDistanceToCamera(it->position);

		sv.position.x = (*it).position.x;
		sv.position.y = (*it).position.y;
		sv.position.z = (*it).position.z;
		sv.radius = (*it).Settings.spriteRadius;
		sv.textureIndex = (*it).Settings.spriteIndex;
		sv.alpha = 0.9f;

		projectileSprites.push_back(sv);

		it++;
	}

	for (list<ExplosionInstance>::iterator it = explosions.begin(); it != explosions.end();) {
		SpriteVertex explosion;
		explosion.lifetime = (*it).passedTime;
		explosion.position = (*it).position;
		explosion.position.y += 25;
		explosion.radius = 50;
		explosion.textureIndex = (*it).textureIndex;
		explosion.alpha = 1-(it->passedTime/4);
		if (explosion.alpha < 0.2) explosion.alpha = 0.2;
		explosion.distanceToCamera = getDistanceToCamera(it->position);

		projectileSprites.push_back(explosion);
		it++;
	}

	for (list<Particle>::iterator it = particles.begin(); it != particles.end();)
	{
		SpriteVertex particle;
		particle.lifetime = it->duration;
		particle.position = it->position;
		particle.radius = 2;
		particle.textureIndex = it->spriteIndex;
		particle.alpha = 1-it->passedTime;
		particle.distanceToCamera = getDistanceToCamera(it->position);
	
		projectileSprites.push_back(particle);
		it++;
	}

	std::sort(projectileSprites.begin(),projectileSprites.end(),sortSprites);

	g_SpriteRenderer->renderSprites(pd3dImmediateContext, projectileSprites, g_camera);

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    V(g_hud.OnRender( fElapsedTime ));
    V(g_sampleUI.OnRender( fElapsedTime ));
    RenderText();
    DXUT_EndPerfEvent();

    static DWORD dwTimefirst = GetTickCount();
    if ( GetTickCount() - dwTimefirst > 5000 )
    {    
        OutputDebugString( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
        OutputDebugString( L"\n" );
        dwTimefirst = GetTickCount();
    }
}
