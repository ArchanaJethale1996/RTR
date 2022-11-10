#include<windows.h>
#include<stdio.h>

#include<d3d11.h>
#include<d3dcompiler.h>
#include"WICTextureLoader.h"

#pragma warning(disable:4838)
#include"XNAMath_204\xnamath.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3dcompiler.lib")
#pragma comment(lib,"DirectXTK.lib")

using namespace DirectX;
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

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;

ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Position_Cube = NULL;

ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

ID3D11RasterizerState *gpID3D11RaterizerState = NULL;
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

ID3D11ShaderResourceView *gpID3D11ShaderResourceView_Texture_Marble = NULL;
ID3D11SamplerState *gpID3D11SamplerState_Texture_Marble = NULL;

//animation 
float gAngleCube = 360.0f;

struct CBUFFER
{
	XMMATRIX WorldViewMatrix;
	XMMATRIX ProjectionMatrix;
	XMVECTOR Ld;
	XMVECTOR Kd;
	XMVECTOR LightPosition;
	unsigned int LightEnabled;
};

XMMATRIX gPerspectiveProjectionMatrix;

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
	HRESULT LoadD3DTexture(const wchar_t*, ID3D11ShaderResourceView**);
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

	//initialize shader,input layouts,constant buffers,etc
	const char *vertexShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 worldViewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 ld;" \
		"float4 kd;" \
		"float4 lightPosition;" \
		"uint lightEnabled;" \
		"}" \
		"struct vertex_output{" \
		"float4 position:SV_POSITION;" \
		"float4 color:COLOR;" \
		"float4 diffuseLight:NORMAL0;" \
		"float2 texcoord:TEXCOORD;" \
		"};" \
		"vertex_output main(float4 pos : POSITION,float4 colorVert : COLOR,float4 normal : NORMAL0,float2 texcoordVert : TEXCOORD)" \
		"{" \
		"vertex_output output;" \
		"if(lightEnabled==1)" \
		"{" \
		"		float4 eyeCoordinates=mul(worldViewMatrix,pos);" \
		"		float3 tNorm=normalize(mul((float3x3)worldViewMatrix,(float3)normal));" \
		"       float3 s = (float3)normalize(lightPosition - eyeCoordinates);" \
		"       output.diffuseLight = ld * kd * max(dot(s, tNorm),0.0);" \
		"}" \
		"else" \
		"{" \
		"       output.diffuseLight =float4(1.0,1.0,1.0,1.0);" \
		"}" \
		"output.color=colorVert;" \
		"output.texcoord=texcoordVert;" \
		"output.position=mul(worldViewMatrix,pos);" \
		"output.position=mul(projectionMatrix,output.position);" \
		"return(output);" \
		"}";

	ID3DBlob *pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCode,
		lstrlenA(vertexShaderSourceCode) + 1,
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
		&gpID3D11VertexShader);
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

	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, 0, 0);

	//Pixel Shaders
	const char* pixelShaderSourceCode =
		"Texture2D myTexture2D;" \
		"SamplerState mySamplerState;" \
		"float4 main(float4 position:SV_POSITION,float4 color:COLOR,float4 diffuseLight:NORMAL0,float2 texcoord:TEXCOORD):SV_TARGET" \
		"{" \
		"float4 texColor = myTexture2D.Sample(mySamplerState,texcoord);" \
		"return(diffuseLight*color*texColor);" \
		"}";

	

	ID3DBlob *pID3DBlob_PixelShaderCode = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(pixelShaderSourceCode,
		lstrlenA(pixelShaderSourceCode) + 1,
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
		&gpID3D11PixelShader);
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
	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);
	//pID3DBlob_PixelShaderCode->Release();


	///////////////////////////////////////////////////////
	//Cube
	//////////////////////////////////////////////////////

	float verticesCube[] = {
		-1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 0.0f,		 0.0f, 1.0f, 0.0f,		 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f,
		1.0f, 1.0f, -1.0f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,		 0.0f, 1.0f, 0.0f,		0.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		1.0f, -1.0f, 1.0f,		0.0f, 1.0f, 0.0f,		 0.0f, -1.0f, 0.0f,		0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,	 0.0f, 1.0f, 0.0f,		0.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,	 0.0f, 1.0f, 0.0f,		0.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,		 0.0f, 1.0f, 0.0f,		0.0f, -1.0f, 0.0f,		0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,		0.0f, 1.0f, 0.0f,		 0.0f, -1.0f, 0.0f,		 1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,		 0.0f, 0.0f, 1.0f,		0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
		1.0f, 1.0f, -1.0f,		 0.0f, 0.0f, 1.0f,		0.0f, 0.0f, -1.0f,		0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,	 0.0f, 0.0f, 1.0f,		0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,	0.0f, 0.0f, 1.0f,		0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
		1.0f, 1.0f, -1.0f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f, -1.0f,		 0.0f, 1.0f,
		1.0f, -1.0f, -1.0f,		 0.0f, 0.0f, 1.0f,		0.0f, 0.0f, -1.0f,		1.0f, 1.0f,

		1.0f, -1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,		 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,		 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		 0.0f, 0.0f, 1.0f,		0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		 1.0f, 1.0f,

		-1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,	 -1.0f, 0.0f, 0.0f,			0.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,		1.0f, 0.0f, 1.0f,	 -1.0f, 0.0f, 0.0f,			 0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,		 1.0f, 0.0f, 1.0f,	 -1.0f, 0.0f, 0.0f,			1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,		 1.0f, 0.0f, 1.0f,	 -1.0f, 0.0f, 0.0f,			1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,		1.0f, 0.0f, 1.0f,	 -1.0f, 0.0f, 0.0f,			0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 1.0f,	 -1.0f, 0.0f, 0.0f,			1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,		1.0f, 1.0f, 0.0f,		1.0f, 0.0f, 0.0f,		 0.0f, 0.0f,
		1.0f, 1.0f, -1.0f,		1.0f, 1.0f, 0.0f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f,
		1.0f, -1.0f, 1.0f,		1.0f, 1.0f, 0.0f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,		1.0f, 1.0f, 0.0f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		1.0f, 1.0f, -1.0f,		1.0f, 1.0f, 0.0f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,		1.0f, 1.0f, 0.0f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f
	};

	D3D11_BUFFER_DESC bufferDesc_Cube;
	ZeroMemory(&bufferDesc_Cube, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_Cube.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_Cube.ByteWidth = sizeof(float)*ARRAYSIZE(verticesCube);
	bufferDesc_Cube.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc_Cube.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpD3D11Device->CreateBuffer(&bufferDesc_Cube, NULL, &gpID3D11Buffer_VertexBuffer_Position_Cube);
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
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Position_Cube, NULL, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, verticesCube, sizeof(verticesCube));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Position_Cube, 0);



	//create and set input Layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[4];
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;


	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].SemanticName = "COLOR";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

	inputElementDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[2].InputSlot = 2;
	inputElementDesc[2].SemanticName = "NORMAL";
	inputElementDesc[2].SemanticIndex = 0;
	inputElementDesc[2].AlignedByteOffset = 0;
	inputElementDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[2].InstanceDataStepRate = 0;

	inputElementDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[3].InputSlot = 3;
	inputElementDesc[3].SemanticName = "TEXCOORD";
	inputElementDesc[3].SemanticIndex = 0;
	inputElementDesc[3].AlignedByteOffset = 0;
	inputElementDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[3].InstanceDataStepRate = 0;

	hr = gpD3D11Device->CreateInputLayout(inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		&gpID3D11InputLayout);
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

	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);
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


	//Marble
	hr = LoadD3DTexture(L"Marble.bmp", &gpID3D11ShaderResourceView_Texture_Marble);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " LoadD3DTexture() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, " LoadD3DTexture() Succeded.\n");
		fclose(gpFile);
	}

	//sample state
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = gpD3D11Device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerState_Texture_Marble);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateSamplerState() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpD3D11Device->CreateSamplerState() Succeded.\n");
		fclose(gpFile);
	}


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


HRESULT LoadD3DTexture(const wchar_t *textureFileName, ID3D11ShaderResourceView **ppID3D11ShaderResourceView)
{
	HRESULT hr;

	hr = DirectX::CreateWICTextureFromFile(gpD3D11Device, gpID3D11DeviceContext, textureFileName, NULL, ppID3D11ShaderResourceView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "CreateWICTextureFromFile() Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "CreateWICTextureFromFile() Succeded.\n");
		fclose(gpFile);
	}
	return(hr);
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

	//set Perspective matrix

	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	return(hr);
}


void display(void)
{
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//select which vertex buffer to display
	UINT stride = sizeof(float) * 11;
	UINT offsetPos = sizeof(float)*0;
	UINT offsetColor = sizeof(float) * 3;
	UINT offsetNormal = sizeof(float) * 6;
	UINT offsetTexCoord = sizeof(float) * 9;

	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer_Position_Cube, &stride, &offsetPos);
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_VertexBuffer_Position_Cube, &stride, &offsetColor);
	gpID3D11DeviceContext->IASetVertexBuffers(2, 1, &gpID3D11Buffer_VertexBuffer_Position_Cube, &stride, &offsetNormal);
	gpID3D11DeviceContext->IASetVertexBuffers(3, 1, &gpID3D11Buffer_VertexBuffer_Position_Cube, &stride, &offsetTexCoord);

	gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpID3D11ShaderResourceView_Texture_Marble);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerState_Texture_Marble);

	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//translation is concerned with world matrix transformation
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX translateMatrix = XMMatrixTranslation(0.0f, 0.0f, 6.0f);
	XMMATRIX r1 = XMMatrixRotationX(gAngleCube);
	XMMATRIX r2 = XMMatrixRotationY(gAngleCube);
	XMMATRIX r3 = XMMatrixRotationZ(gAngleCube);
	XMMATRIX rotationMatrix = r1*r2*r3;
	worldMatrix = rotationMatrix*translateMatrix;
	XMMATRIX wvMatrix = worldMatrix*viewMatrix;

	//load the data into the constant buffer
	CBUFFER constantBuffer;
	if (gLighting == 1)
	{
		constantBuffer.WorldViewMatrix = wvMatrix;
		constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;
		constantBuffer.Ld = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.Kd = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
		constantBuffer.LightPosition = XMVectorSet(0.0f, 0.0f, 2.0f, 1.0f);
		constantBuffer.LightEnabled = 1;
	}
	else
	{
		constantBuffer.WorldViewMatrix = wvMatrix;
		constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;
		constantBuffer.Ld = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.Kd = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
		constantBuffer.LightPosition = XMVectorSet(0.0f, 0.0f, 2.0f, 1.0f);
		constantBuffer.LightEnabled = 0;
	}
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	gpID3D11DeviceContext->Draw(6, 0);
	gpID3D11DeviceContext->Draw(6, 6);
	gpID3D11DeviceContext->Draw(6, 12);
	gpID3D11DeviceContext->Draw(6, 18);
	gpID3D11DeviceContext->Draw(6, 24);
	gpID3D11DeviceContext->Draw(6, 30);
	gpIDXGISwapChain->Present(0, 0);
}


void update(void)
{
	
	gAngleCube = gAngleCube - 0.001f;
	if (gAngleCube <= 0.0f)
	{
		gAngleCube = 360.0f;
	}
}


void unInitialize(void)
{
	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
	}
	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Position_Cube)
	{
		gpID3D11Buffer_VertexBuffer_Position_Cube->Release();
		gpID3D11Buffer_VertexBuffer_Position_Cube = NULL;
	}

	if (gpID3D11RaterizerState)
	{
		gpID3D11RaterizerState->Release();
		gpID3D11RaterizerState = NULL;
	}


	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}
	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
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