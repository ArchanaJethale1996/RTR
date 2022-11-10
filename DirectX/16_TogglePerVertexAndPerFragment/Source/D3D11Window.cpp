#include<windows.h>
#include<stdio.h>

#include<d3d11.h>
#include<d3dcompiler.h>

#pragma warning(disable:4838)
#include"XNAMath_204\xnamath.h"

#include"Sphere.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3dcompiler.lib")
#pragma comment(lib,"Sphere.lib")
//macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//global variables

FILE *gpFile = NULL;
char gszLogFileName[] = "Log.txt";

HWND ghwnd = NULL;

bool gbActiveWindow = false;
bool gbEscapedKeyIsPressed = false;
bool gbFullscreen = false;
int gLighting = 0;

float gClearColor[4];

IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device*gpD3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader_PerFragment = NULL;
ID3D11PixelShader *gpID3D11PixelShader_PerFragment = NULL;

ID3D11VertexShader *gpID3D11VertexShader_PerVertex = NULL;
ID3D11PixelShader *gpID3D11PixelShader_PerVertex = NULL;


ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Position_Sphere = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Normal_Sphere = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Index_Sphere = NULL;

ID3D11InputLayout *gpID3D11InputLayout_PerFragment = NULL;
ID3D11InputLayout *gpID3D11InputLayout_PerVertex = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

ID3D11RasterizerState *gpID3D11RaterizerState = NULL;
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;
//animation 
float gAngleSphere = 360.0f;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumElements, gNumVertices;

struct CBUFFER
{
	XMMATRIX WorldMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;
	XMVECTOR Ld;
	XMVECTOR La;
	XMVECTOR Ls;
	XMVECTOR Kd;
	XMVECTOR Ka;
	XMVECTOR Ks;
	XMVECTOR LightPosition;
	float MaterialShininess;
	unsigned int LightEnabled;
};


XMMATRIX gPerspectiveProjectionMatrix;
bool bVertex=true;
//WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmddLine, int iCmdShow)
{
	//functions Declaration
	HRESULT initialize(void);
	void uninitialize(void);
	void display(void);
	void update(void);

	//variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyApp");
	bool bDone = false;

	//code
	if (fopen_s(&gpFile, "log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("File cannot be created"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf_s(gpFile, "File Created and opened Successfully\n");
		fclose(gpFile);
	}

	//initialization of WNDCLASSSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


	//register the above class
	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("My Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghwnd = hwnd;
	HRESULT hr;
	hr = initialize();
	if (FAILED(hr))
	{
		fopen_s(&gpFile, "log.txt", "a+");
		fprintf_s(gpFile, "initialize() Failed...Exitting now...\n");
		fclose(gpFile);
		DestroyWindow(hwnd);
	}
	else
	{
		fopen_s(&gpFile, "log.txt", "a+");
		fprintf_s(gpFile, "initialize() Successfull...\n");
		fclose(gpFile);
	}
	ShowWindow(hwnd, iCmdShow);

	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == true)
			{
				//call update here
				update();
			}
			//call display here
			display();
		}
	}
	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void ToggleFullScreen(void);
	HRESULT resize(int, int);
	void display(void);
	void unInitialize(void);
	HRESULT hr;
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;
	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
			if (FAILED(hr))
			{
				fopen_s(&gpFile, "log.txt", "a+");
				fprintf_s(gpFile, "resize() Failed...Exitting now...\n");
				fclose(gpFile);
				DestroyWindow(hwnd);
			}
			else
			{
				fopen_s(&gpFile, "log.txt", "a+");
				fprintf_s(gpFile, "resize() Successfull...\n");
				fclose(gpFile);
			}
		}
		break;
	case WM_ERASEBKGND:
		return(0);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		case 0x46:
			ToggleFullScreen();
			break;
		case 'L':
		case 'l':
			if (gLighting == 0)
				gLighting = 1;
			else
				gLighting = 0;
			break;
		case 'V':
		case 'v':
			bVertex = !bVertex;
			if (bVertex == false)
			{
				gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader_PerFragment, 0, 0);
				gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader_PerFragment, 0, 0);
				gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout_PerFragment);
			}
			else
			{

				gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader_PerVertex, 0, 0);
				gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader_PerVertex, 0, 0);
				gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout_PerVertex);
			}
			break;
		default:
			break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		unInitialize();
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

bool bFullScreen = false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev{ sizeof(WINDOWPLACEMENT) };
void ToggleFullScreen(void)
{
	MONITORINFO mi;
	if (bFullScreen == false)
	{
		dwStyle = GetWindowLong(ghwnd,
			GWL_STYLE);
		if (dwStyle&WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev)
				&&
				GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd,
					GWL_STYLE,
					dwStyle&~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		bFullScreen = true;

	}
	else
	{
		SetWindowLong(ghwnd,
			GWL_STYLE,
			dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd,
			&wpPrev);
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
		ShowCursor(TRUE);
		bFullScreen = false;
	}
}

HRESULT initialize(void)
{

	HRESULT resize(int, int);
	void unInitialize(void);

	//variable Declarations
	HRESULT hr;

	D3D_DRIVER_TYPE d3dDriverType;
	D3D_DRIVER_TYPE d3dDriverTypes[] = { D3D_DRIVER_TYPE_HARDWARE,D3D_DRIVER_TYPE_WARP,D3D_DRIVER_TYPE_REFERENCE };

	D3D_FEATURE_LEVEL d3dFeatureLevel_aquired = D3D_FEATURE_LEVEL_10_0;
	D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;

	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevel = 1;

	numDriverTypes = (sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]));

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void *)&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = ghwnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = d3dDriverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, d3dDriverType, NULL, createDeviceFlags, &d3dFeatureLevel_required, numFeatureLevel, D3D11_SDK_VERSION, &dxgiSwapChainDesc, &gpIDXGISwapChain, &gpD3D11Device, &d3dFeatureLevel_aquired, &gpID3D11DeviceContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() Failed...Exitting now...\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() succeeded.\n");
		fprintf_s(gpFile, "The choosen Driver Is of = ");
		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(gpFile, "Hardware Type.\n ");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			fprintf_s(gpFile, "Warp Type.\n ");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			fprintf_s(gpFile, "Reference Type.\n ");
		}
		else
		{
			fprintf_s(gpFile, "Unknown Type.\n ");
		}

		fprintf_s(gpFile, "The Supported Highest Feature Type is ");
		if (d3dFeatureLevel_aquired == D3D_FEATURE_LEVEL_11_0)
		{
			fprintf_s(gpFile, "11.0\n ");
		}
		else if (d3dFeatureLevel_aquired == D3D_FEATURE_LEVEL_10_0)
		{
			fprintf_s(gpFile, "10.0\n ");
		}
		else if (d3dFeatureLevel_aquired == D3D_FEATURE_LEVEL_10_1)
		{
			fprintf_s(gpFile, "10.1\n ");
		}
		else
		{
			fprintf_s(gpFile, "Unknown\n ");
		}
		fclose(gpFile);
	}
	////////////////////////////////////////
	//Vertex Shader

	//initialize shader,input layouts,constant buffers,etc
	const char *vertexShaderSourceCodePerVertex =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 worldMatrix;" \
		"float4x4 viewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 ld;" \
		"float4 la;" \
		"float4 ls;" \
		"float4 kd;" \
		"float4 ka;" \
		"float4 ks;" \
		"float4 lightPosition;" \
		"float materialShininess;" \
		"uint lightEnabled;" \
		"}" \
		"struct vertex_output{" \
		"float4 position:SV_POSITION;" \
		"float4 phongLight:COLOR;" \
		"};" \
		"vertex_output main(float4 pos : POSITION,float4 normal : NORMAL)" \
		"{" \
		"vertex_output output;" \
		"if(lightEnabled==1)" \
		"{" \
		"		float4 eyeCoordinates=mul(worldMatrix,pos);" \
		"		eyeCoordinates=mul(viewMatrix,eyeCoordinates);" \
		"		float3 tNorm=normalize((mul((float3x3)mul(viewMatrix,worldMatrix),normal)));" \
		"		float3 lightDirection=normalize((float3)(lightPosition-eyeCoordinates));" \
		"		float tndotld=max(dot(lightDirection,tNorm),0.0);" \
		"		float3 ReflectionVector=reflect(-lightDirection,tNorm);" \
		"		float3 viewerVector=normalize((float3)(-eyeCoordinates.xyz));" \
		"		float3 ambient=la*ka;" \
		"		float3 diffused=ld*kd*tndotld;" \
		"		float3 specular=ls*ks*pow(max(dot(ReflectionVector,viewerVector),0.0),materialShininess);" \
		"       output.phongLight = float4(ambient+diffused+specular,1.0);" \
		"}" \
		"else" \
		"{" \
		"       output.phongLight =float4(1.0,1.0,1.0,1.0);" \
		"}" \
		"output.position=mul(worldMatrix,pos);" \
		"output.position=mul(viewMatrix,output.position);" \
		"output.position=mul(projectionMatrix,output.position);" \
		"return(output);" \
		"}";

	ID3DBlob *pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCodePerVertex,
		lstrlenA(vertexShaderSourceCodePerVertex) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderCode,
		&pID3DBlob_Error);
	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For VertexShader :%s\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeded For VertexShader.\n");
		fclose(gpFile);
	}

	hr = gpD3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11VertexShader_PerVertex);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateVertexShader() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateVertexShader() Succeded.\n");
		fclose(gpFile);
	}

	//Pixel Shaders
	const char* pixelShaderSourceCodePerVertex =
		"float4 main(float4 position:SV_POSITION,float4 phongLight:COLOR):SV_TARGET" \
		"{" \
		"return(phongLight);" \
		"}";

	ID3DBlob *pID3DBlob_PixelShaderCode = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(pixelShaderSourceCodePerVertex,
		lstrlenA(pixelShaderSourceCodePerVertex) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode,
		&pID3DBlob_Error);
	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Pixel Shader :%s\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeded For VertexShader.\n");
		fclose(gpFile);
	}

	hr = gpD3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode->GetBufferPointer(),
		pID3DBlob_PixelShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11PixelShader_PerVertex);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreatePixelShader() Failed For VertexShader.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreatePixelShader() Succeded For VertexShader.\n");
		fclose(gpFile);
	}


	/////////////////////////////////////////
	//Fragment SHader

	//initialize shader,input layouts,constant buffers,etc
	const char *vertexShaderSourceCode_PerFragment =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 worldMatrix;" \
		"float4x4 viewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 ld;" \
		"float4 la;" \
		"float4 ls;" \
		"float4 kd;" \
		"float4 ka;" \
		"float4 ks;" \
		"float4 lightPosition;" \
		"float materialShininess;" \
		"uint lightEnabled;" \
		"}" \
		"struct vertex_output{" \
		"float4 position:SV_POSITION;" \
		"float3 tNormVertexShader:NORMAL0;" \
		"float4 eye_coordinatesVertexShader:NORMAL1;" \
		"};" \
		"vertex_output main(float4 pos : POSITION,float4 normal : NORMAL)" \
		"{" \
		"vertex_output output;" \
		"if(lightEnabled==1)" \
		"{" \
		"		output.eye_coordinatesVertexShader=mul(viewMatrix,(mul(worldMatrix,pos)));" \
		"		output.tNormVertexShader=mul((float3x3)mul(viewMatrix,worldMatrix),normal);" \
		"}" \
		"output.position=mul(worldMatrix,pos);" \
		"output.position=mul(viewMatrix,output.position);" \
		"output.position=mul(projectionMatrix,output.position);" \
		"return(output);" \
		"}";


	ID3DBlob *pID3DBlob_VertexShaderCode_PerFragment = NULL;
	ID3DBlob *pID3DBlob_Error_PerFragment = NULL;

	hr = D3DCompile(vertexShaderSourceCode_PerFragment,
		lstrlenA(vertexShaderSourceCode_PerFragment) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderCode_PerFragment,
		&pID3DBlob_Error_PerFragment);
	if (FAILED(hr))
	{
		if (pID3DBlob_Error_PerFragment != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For VertexShader :%s\n", (char*)pID3DBlob_Error_PerFragment->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error_PerFragment->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeded For VertexShader.\n");
		fclose(gpFile);
	}

	hr = gpD3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode_PerFragment->GetBufferPointer(),
		pID3DBlob_VertexShaderCode_PerFragment->GetBufferSize(),
		NULL,
		&gpID3D11VertexShader_PerFragment);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateVertexShader() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateVertexShader() Succeded.\n");
		fclose(gpFile);
	}


	//Pixel Shaders
	const char* pixelShaderSourceCode_PerFragment =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 worldMatrix;" \
		"float4x4 viewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 ld;" \
		"float4 la;" \
		"float4 ls;" \
		"float4 kd;" \
		"float4 ka;" \
		"float4 ks;" \
		"float4 lightPosition;" \
		"float materialShininess;" \
		"uint lightEnabled;" \
		"}" \
		"struct vertex_output{" \
		"float4 position:SV_POSITION;" \
		"float3 tNormVertexShader:NORMAL0;" \
		"float4 eye_coordinatesVertexShader:NORMAL1;" \
		"};" \
		"float4 main(float4 position:SV_POSITION,vertex_output input):SV_TARGET" \
		"{" \
		"float4 FragColor;" \
		"if(lightEnabled==1)" \
		"{" \
		"float3 tNorm=normalize(input.tNormVertexShader);" \
		"float3 lightDirection=normalize((float3)(lightPosition-input.eye_coordinatesVertexShader));" \
		"float tndotld=max(dot(lightDirection,tNorm),0.0);" \
		"float3 ReflectionVector=reflect(-lightDirection,tNorm);" \
		"float3 viewerVector=normalize((float3)(-input.eye_coordinatesVertexShader.xyz));" \
		"float3 ambient=la*ka;" \
		"float3 diffused=ld*kd*tndotld;" \
		"float3 specular=ls*ks*pow(max(dot(ReflectionVector,viewerVector),0.0),materialShininess);" \
		"float3 phong_ads_light = ambient+diffused+specular;" \
		"FragColor=float4(phong_ads_light,1.0);" \
		"}" \
		"else" \
		"{" \
		"FragColor=float4(1.0,1.0,1.0,1.0);"
		"}" \

		"return(FragColor);" \
		"}";

	ID3DBlob *pID3DBlob_PixelShaderCode_PerFragment = NULL;
	pID3DBlob_Error_PerFragment = NULL;

	hr = D3DCompile(pixelShaderSourceCode_PerFragment,
		lstrlenA(pixelShaderSourceCode_PerFragment) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode_PerFragment,
		&pID3DBlob_Error_PerFragment);
	if (FAILED(hr))
	{
		if (pID3DBlob_Error_PerFragment != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed For Pixel Shader :%s\n", (char*)pID3DBlob_Error_PerFragment->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error_PerFragment->Release();
			pID3DBlob_Error_PerFragment = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeded For VertexShader.\n");
		fclose(gpFile);
	}

	hr = gpD3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode_PerFragment->GetBufferPointer(),
		pID3DBlob_PixelShaderCode_PerFragment->GetBufferSize(),
		NULL,
		&gpID3D11PixelShader_PerFragment);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreatePixelShader() Failed For VertexShader.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreatePixelShader() Succeded For VertexShader.\n");
		fclose(gpFile);
	}

	///////////////////////////////////////////////////////
	//Sphere
	//////////////////////////////////////////////////////
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();


	D3D11_BUFFER_DESC bufferDesc_position_Sphere;
	ZeroMemory(&bufferDesc_position_Sphere, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_position_Sphere.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_position_Sphere.ByteWidth = sizeof(float)*ARRAYSIZE(sphere_vertices);
	bufferDesc_position_Sphere.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc_position_Sphere.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpD3D11Device->CreateBuffer(&bufferDesc_position_Sphere, NULL, &gpID3D11Buffer_VertexBuffer_Position_Sphere);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateBuffer() Failed For VertexShader.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateBuffer() succeded For VertexShader.\n");
		fclose(gpFile);
	}

	//copy vertices into above buffer
	D3D11_MAPPED_SUBRESOURCE  mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Position_Sphere, NULL, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_vertices, sizeof(sphere_vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Position_Sphere, 0);


	D3D11_BUFFER_DESC bufferDesc_normal_Sphere;
	ZeroMemory(&bufferDesc_normal_Sphere, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_normal_Sphere.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_normal_Sphere.ByteWidth = sizeof(float)*ARRAYSIZE(sphere_normals);
	bufferDesc_normal_Sphere.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc_normal_Sphere.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpD3D11Device->CreateBuffer(&bufferDesc_normal_Sphere, NULL, &gpID3D11Buffer_VertexBuffer_Normal_Sphere);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateBuffer() Failed For VertexShader.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateBuffer() succeded For VertexShader.\n");
		fclose(gpFile);
	}

	//copy vertices into above buffer
	//D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Normal_Sphere, NULL, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_normals, sizeof(sphere_normals));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Normal_Sphere, 0);

	//Index
	D3D11_BUFFER_DESC bufferDesc_element_Sphere;
	ZeroMemory(&bufferDesc_element_Sphere, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_element_Sphere.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_element_Sphere.ByteWidth = gNumElements * sizeof(short);
	bufferDesc_element_Sphere.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc_element_Sphere.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpD3D11Device->CreateBuffer(&bufferDesc_element_Sphere, NULL, &gpID3D11Buffer_VertexBuffer_Index_Sphere);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateBuffer() Failed For VertexShader.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateBuffer() succeded For VertexShader.\n");
		fclose(gpFile);
	}

	//copy vertices into above buffer
	//D3D11_MAPPED_SUBRESOURCE  mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Index_Sphere, NULL, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_elements, gNumElements * sizeof(short));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Index_Sphere, 0);

	//////////////////////////////////////////////////////////
	//Per Fragment
	//create and set input Layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;


	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].SemanticName = "NORMAL";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

	hr = gpD3D11Device->CreateInputLayout(inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		&gpID3D11InputLayout_PerFragment);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateInputLayout() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateInputLayout() succeded.\n");
		fclose(gpFile);
	}

	///////////////////////////////
	//Per Vertex
	//create and set input Layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc1[2];
	inputElementDesc1[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc1[0].InputSlot = 0;
	inputElementDesc1[0].SemanticName = "POSITION";
	inputElementDesc1[0].SemanticIndex = 0;
	inputElementDesc1[0].AlignedByteOffset = 0;
	inputElementDesc1[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc1[0].InstanceDataStepRate = 0;


	inputElementDesc1[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc1[1].InputSlot = 1;
	inputElementDesc1[1].SemanticName = "NORMAL";
	inputElementDesc1[1].SemanticIndex = 0;
	inputElementDesc1[1].AlignedByteOffset = 0;
	inputElementDesc1[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc1[1].InstanceDataStepRate = 0;

	hr = gpD3D11Device->CreateInputLayout(inputElementDesc1,
		_ARRAYSIZE(inputElementDesc1),
		pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		&gpID3D11InputLayout_PerVertex);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateInputLayout() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateInputLayout() succeded.\n");
		fclose(gpFile);
	}

	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_PixelShaderCode->Release();
	pID3DBlob_VertexShaderCode = NULL;
	pID3DBlob_PixelShaderCode = NULL;

	//define and set the constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = gpD3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, NULL, &gpID3D11Buffer_ConstantBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateBuffer() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateBuffer() Succeded.\n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);
	gpID3D11DeviceContext->PSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory((void*)&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;

	hr = gpD3D11Device->CreateRasterizerState(&rasterizerDesc, &gpID3D11RaterizerState);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateRasterizerState() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " gpD3D11Device->CreateRasterizerState() Succeded.\n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->RSSetState(gpID3D11RaterizerState);

	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.0f;
	gClearColor[2] = 0.0f;
	gClearColor[3] = 1.0f;

	gPerspectiveProjectionMatrix = XMMatrixIdentity();

	hr = resize(WIN_WIDTH, WIN_HEIGHT);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() Succeded.\n");
		fclose(gpFile);
	}
	return (S_OK);
}

HRESULT resize(int width, int height)
{
	HRESULT hr = S_OK;
	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}
	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

	gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer;

	gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pID3D11Texture2D_BackBuffer);
	hr = gpD3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &gpID3D11RenderTargetView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateRenderTargetView() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateRenderTargetView() Succeded.\n");
		fclose(gpFile);
	}
	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;


	//Depth Stencil buffer
	D3D11_TEXTURE2D_DESC textureDesc = {};
	ZeroMemory((void *)&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D *pID3D11Texture2D_DepthBuffer = NULL;
	hr = gpD3D11Device->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2D_DepthBuffer);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateTexture2D() Failed.\n");
		fclose(gpFile);
		return(hr);
	}

	//create Depth Stencil Buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	ZeroMemory((void *)&depthStencilViewDesc,
		sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	hr = gpD3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer,
		&depthStencilViewDesc,
		&gpID3D11DepthStencilView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateDepthStencilView() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateDepthStencilView() Succeded.\n");
		fclose(gpFile);
	}
	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);

	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Height = (float)height;
	d3dViewPort.Width = (float)width;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	
	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader_PerVertex, 0, 0);
	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader_PerVertex, 0, 0);
	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout_PerVertex);

	//set Perspective matrix
	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	return(hr);
}


void display(void)
{
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);



	//select which vertex buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;


	////////////////////////////////////////////////////////////////////////
	//Sphere
	//select which vertex buffer to display
	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer_Position_Sphere, &stride, &offset);

	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_VertexBuffer_Normal_Sphere, &stride, &offset);

	gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_VertexBuffer_Index_Sphere, DXGI_FORMAT_R16_UINT, 0);

	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//translation is concerned with world matrix transformation
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX translateMatrix = XMMatrixTranslation(0.0f, 0.0f, 3.0f);
	XMMATRIX r1 = XMMatrixRotationX(gAngleSphere);
	XMMATRIX r2 = XMMatrixRotationY(gAngleSphere);
	XMMATRIX r3 = XMMatrixRotationZ(gAngleSphere);
	XMMATRIX rotationMatrix = r1*r2*r3;
	worldMatrix = rotationMatrix*translateMatrix;


	//load the data into the constant buffer
	CBUFFER constantBuffer;
	if (gLighting == 1)
	{
		constantBuffer.WorldMatrix = worldMatrix;
		constantBuffer.ViewMatrix = viewMatrix;
		constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;
		constantBuffer.Ld = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.La = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		constantBuffer.Ls = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.Kd = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.Ka = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		constantBuffer.Ks = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.LightPosition = XMVectorSet(100.0f, 100.0f, 100.0f, 1.0f);
		constantBuffer.MaterialShininess = 256.0f;
		constantBuffer.LightEnabled = 1;
	}
	else
	{
		constantBuffer.WorldMatrix = worldMatrix;
		constantBuffer.ViewMatrix = viewMatrix;
		constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;
		constantBuffer.Ld = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.La = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		constantBuffer.Ls = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.Kd = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.Ka = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		constantBuffer.Ks = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.LightPosition = XMVectorSet(100.0f, 100.0f, 300.0f, 1.0f);
		constantBuffer.MaterialShininess = 256.0f;
		constantBuffer.LightEnabled = 0;
	}
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);
	gpIDXGISwapChain->Present(0, 0);
}


void update(void)
{

	gAngleSphere = gAngleSphere - 0.001f;
	if (gAngleSphere <= 0.0f)
	{
		gAngleSphere = 360.0f;
	}
}


void unInitialize(void)
{
	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
	}
	if (gpID3D11InputLayout_PerFragment)
	{
		gpID3D11InputLayout_PerFragment->Release();
		gpID3D11InputLayout_PerFragment = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Index_Sphere)
	{
		gpID3D11Buffer_VertexBuffer_Index_Sphere->Release();
		gpID3D11Buffer_VertexBuffer_Index_Sphere = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Position_Sphere)
	{
		gpID3D11Buffer_VertexBuffer_Position_Sphere->Release();
		gpID3D11Buffer_VertexBuffer_Position_Sphere = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Normal_Sphere)
	{
		gpID3D11Buffer_VertexBuffer_Normal_Sphere->Release();
		gpID3D11Buffer_VertexBuffer_Normal_Sphere = NULL;
	}
	if (gpID3D11RaterizerState)
	{
		gpID3D11RaterizerState->Release();
		gpID3D11RaterizerState = NULL;
	}


	if (gpID3D11PixelShader_PerFragment)
	{
		gpID3D11PixelShader_PerFragment->Release();
		gpID3D11PixelShader_PerFragment = NULL;
	}
	if (gpID3D11VertexShader_PerFragment)
	{
		gpID3D11VertexShader_PerFragment->Release();
		gpID3D11VertexShader_PerFragment = NULL;
	}
	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}
	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
	}
	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext->Release();
		gpID3D11DeviceContext = NULL;
	}
	if (gpD3D11Device)
	{
		gpD3D11Device->Release();
		gpD3D11Device = NULL;
	}

	if (gpFile)
	{
		fopen_s(&gpFile, "log.txt", "a+");
		fprintf(gpFile, "File closed successfully");
		fclose(gpFile);
		gpFile = NULL;
	}

}