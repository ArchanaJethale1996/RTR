#include<windows.h>
#include<GL/glew.h>
#include<gl/GL.h>
#include<stdio.h>
#include"vmath.h"
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

//macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
using namespace vmath;

//global variables
HDC ghdc = NULL;
HWND ghwnd = NULL;
HGLRC ghrc = NULL;
bool gbActiveWindow = false;
FILE *gpFile = NULL;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

GLuint gVertexShaderObjectPerVertex;
GLuint gFragmentShaderObjectPerVertex;
GLuint gShaderProgramObjectPerVertex;

GLuint gVertexShaderObjectPerFragment;
GLuint gFragmentShaderObjectPerFragment;
GLuint gShaderProgramObjectPerFragment;

bool gAnimate = false;
int gLighting = 0;
bool bPerVertex;

struct Light
{
	GLfloat lightAmbient[3];
	GLfloat lightDiffused[3];
	GLfloat lightSpecular[3];
	GLfloat lightPosition[4];
};


struct Light lights[2];

float rotatePyramid = 0.0f;

GLfloat materialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat materialDiffused[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialShininess =  128.0f ;

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0
};

GLuint vao_Pyramid;
GLuint vbo_Position_Pyramid;
GLuint vbo_Normal_Pyramid;
GLuint mUniformPerVertex, vUniformPerVertex, pUniformPerVertex,KDUniformPerVertex,  KAUniformPerVertex, KSUniformPerVertex, MaterialShininessUniformPerVertex, LKeyIsPressedUniformPerVertex;
GLuint 	LDUniformOnePerVertex, LAUniformOnePerVertex, LSUniformOnePerVertex, LightPositionUniformOnePerVertex, LDUniformTwoPerVertex, LAUniformTwoPerVertex, LSUniformTwoPerVertex, LightPositionUniformTwoPerVertex;

GLuint mUniformPerFragment, vUniformPerFragment, pUniformPerFragment, KDUniformPerFragment, KAUniformPerFragment, KSUniformPerFragment, MaterialShininessUniformPerFragment, LKeyIsPressedUniformPerFragment;
GLuint 	LDUniformOnePerFragment, LAUniformOnePerFragment, LSUniformOnePerFragment, LightPositionUniformOnePerFragment, LDUniformTwoPerFragment, LAUniformTwoPerFragment, LSUniformTwoPerFragment, LightPositionUniformTwoPerFragment;

mat4 perspectiveProjectionMatrix;

GLuint samplerUniform;
//WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmddLine, int iCmdShow)
{
	//functions Declaration
	int initialize(void);
	void display(void);
	void update(void);

	//variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyApp");
	bool bDone = false;
	int iRet;

	//code
	if (fopen_s(&gpFile, "log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("File cannot be created"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "File Created and opened Successfully\n");
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

	iRet = initialize();
	if (iRet == -1)
	{
		fprintf(gpFile, "ChoosePixelFormat failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2)
	{
		fprintf(gpFile, "SetPixelFormat failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3)
	{
		fprintf(gpFile, "wglCreateContext failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4)
	{
		fprintf(gpFile, "wglMakeCurrent failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -5)
	{
		fprintf(gpFile, "glewInit failed\n");
		DestroyWindow(hwnd);
	}
	else
	{
		fprintf(gpFile, "Initialization succcceded\n");
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
	void resize(int, int);
	void display(void);
	void unInitialize(void);

	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
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
		case 'V':
		case 'v':
			bPerVertex = !bPerVertex;
			break;
		case 'L':
		case 'l':
			if (gLighting == 0)
				gLighting = 1;
			else
				gLighting = 0;
			break;
		case 'A':
		case 'a':
			gAnimate = !gAnimate;
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

int initialize(void)
{
	void resize(int, int);
	void unInitialize(void);
	//variable Declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;
	GLenum result;

	memset((void*)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));

	//code
	//Initialize pfd
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cBlueBits = 8;
	pfd.cGreenBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;
	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		return(-1);
	}
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return(-2);
	}
	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		return(-3);
	}
	if (!wglMakeCurrent(ghdc, ghrc))
	{
		return(-4);
	}

	result = glewInit();
	if (result != GLEW_OK)
	{
		return(-5);
	}
	//////////////////////////////////////////////////////////////
	//Per Vertex
	//////////////////////////////////////////////////////////////

	//Vertex Shader
	gVertexShaderObjectPerVertex = glCreateShader(GL_VERTEX_SHADER);

	//Write Vertex Shader Object
	GLchar *vertexShaderSourceCodePerVertex =
		"#version 420 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_m_matrix;" \
		"uniform mat4 u_v_matrix;" \
		"uniform mat4 u_p_matrix;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_LdOne;" \
		"uniform vec3 u_LsOne;" \
		"uniform vec3 u_LaOne;" \
		"uniform vec4 u_Light_PositionOne;" \
		"uniform vec3 u_LdTwo;" \
		"uniform vec3 u_LsTwo;" \
		"uniform vec3 u_LaTwo;" \
		"uniform vec4 u_Light_PositionTwo;" \
		"uniform float u_MaterialShininess;" \
		"uniform int u_LKeyIsPressed;" \
		"out vec3 phong_ads_light;" \
		"void main(void)" \
		"{" \
		"if(u_LKeyIsPressed==1)" \
		"{" \
		"vec4 eye_coordinates=u_v_matrix*u_m_matrix*vPosition;" \

		"vec3 tNorm=normalize(mat3(u_v_matrix*u_m_matrix)*vNormal);" \
		"vec3 lightDirectionOne=normalize(vec3(u_Light_PositionOne-eye_coordinates));" \
		"vec3 lightDirectionTwo=normalize(vec3(u_Light_PositionTwo-eye_coordinates));" \
		"float tndotldOne=max(dot(lightDirectionOne,tNorm),0.0);" \
		"float tndotldTwo=max(dot(lightDirectionTwo,tNorm),0.0);" \
		"vec3 ReflectionVectorOne=reflect(-lightDirectionOne,tNorm);" \
		"vec3 ReflectionVectorTwo=reflect(-lightDirectionTwo,tNorm);" \
		"vec3 viewerVector=normalize(vec3(-eye_coordinates.xyz));" \
		"vec3 ambientOne=u_LaOne*u_Ka;" \
		"vec3 diffusedOne=u_LdOne*u_Kd*tndotldOne;" \
		"vec3 specularOne=u_LsOne*u_Ks*pow(max(dot(ReflectionVectorOne,viewerVector),0.0),u_MaterialShininess);" \
		"vec3 ambientTwo=u_LaTwo*u_Ka;" \
		"vec3 diffusedTwo=u_LdTwo*u_Kd*tndotldTwo;" \
		"vec3 specularTwo=u_LsTwo*u_Ks*pow(max(dot(ReflectionVectorTwo,viewerVector),0.0),u_MaterialShininess);" \
		"phong_ads_light = ambientOne+diffusedOne+specularOne+ambientTwo+diffusedTwo+specularTwo;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_light=vec3(1.0,1.0,1.0);" \
		"}" \
		"gl_Position=u_p_matrix*u_v_matrix*u_m_matrix*vPosition;" \
		"}";
	glShaderSource(gVertexShaderObjectPerVertex, 1, (const GLchar **)&vertexShaderSourceCodePerVertex, NULL);
	glCompileShader(gVertexShaderObjectPerVertex);
	//Error Check
	GLint iShaderComileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar* szLogInfo = NULL;

	glGetShaderiv(gVertexShaderObjectPerVertex, GL_COMPILE_STATUS, &iShaderComileStatus);
	if (iShaderComileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObjectPerVertex, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szLogInfo = (GLchar *)malloc(iInfoLogLength);
			if (szLogInfo != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObjectPerVertex, iInfoLogLength, &written, szLogInfo);
				fprintf(gpFile, szLogInfo);
				free(szLogInfo);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}


	//Fragment Shader
	gFragmentShaderObjectPerVertex = glCreateShader(GL_FRAGMENT_SHADER);

	//Write Vertex Shader Object
	GLchar *fragmentShaderSourceCodePerVertex =
		"#version 420 core" \
		"\n" \
		"in vec3 phong_ads_light;" \
		"uniform int u_LKeyIsPressed;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"if(u_LKeyIsPressed==1)" \
		"{" \
		"FragColor=vec4(phong_ads_light,1.0);" \
		"}" \
		"else" \
		"{" \
		"FragColor=vec4(1.0,1.0,1.0,1.0);"
		"}" \
		"}";


	glShaderSource(gFragmentShaderObjectPerVertex, 1, (const GLchar **)&fragmentShaderSourceCodePerVertex, NULL);
	glCompileShader(gFragmentShaderObjectPerVertex);
	//Error Check
	iShaderComileStatus = 0;
	iInfoLogLength = 0;
	szLogInfo = NULL;

	glGetShaderiv(gFragmentShaderObjectPerVertex, GL_COMPILE_STATUS, &iShaderComileStatus);
	if (iShaderComileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObjectPerVertex, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szLogInfo = (GLchar *)malloc(iInfoLogLength);
			if (szLogInfo != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObjectPerVertex, iInfoLogLength, &written, szLogInfo);
				fprintf(gpFile, szLogInfo);
				free(szLogInfo);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}

	//Create Program
	gShaderProgramObjectPerVertex = glCreateProgram();
	glAttachShader(gShaderProgramObjectPerVertex, gVertexShaderObjectPerVertex);
	glAttachShader(gShaderProgramObjectPerVertex, gFragmentShaderObjectPerVertex);

	glBindAttribLocation(gShaderProgramObjectPerVertex, AMC_ATTRIBUTE_POSITION, "vPosition");

	glBindAttribLocation(gShaderProgramObjectPerVertex, AMC_ATTRIBUTE_NORMAL, "vNormal");

	glLinkProgram(gShaderProgramObjectPerVertex);
	GLint iShaderLinkStatus = 0;
	iInfoLogLength = 0;
	szLogInfo = NULL;

	glGetProgramiv(gShaderProgramObjectPerVertex, GL_LINK_STATUS, &iShaderLinkStatus);

	if (iShaderLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObjectPerVertex, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szLogInfo = (GLchar *)malloc(iInfoLogLength);
			if (szLogInfo != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObjectPerVertex, iInfoLogLength, &written, szLogInfo);
				fprintf(gpFile, szLogInfo);
				free(szLogInfo);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}

	mUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_m_matrix");
	vUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_v_matrix");
	pUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_p_matrix");
	LDUniformOnePerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LdOne");
	LAUniformOnePerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LaOne");
	LSUniformOnePerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LsOne");
	LightPositionUniformOnePerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Light_PositionOne");

	LDUniformTwoPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LdTwo");
	LAUniformTwoPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LaTwo");
	LSUniformTwoPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LsTwo");
	LightPositionUniformTwoPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Light_PositionTwo");


	KDUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Kd");
	KAUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ka");
	KSUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ks");
	MaterialShininessUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_MaterialShininess");

	LKeyIsPressedUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LKeyIsPressed");

	//////////////////////////////////////////////////////////////
	//Per Fragment 
	//////////////////////////////////////////////////////
	//Vertex Shader
	gVertexShaderObjectPerFragment = glCreateShader(GL_VERTEX_SHADER);

	//Write Vertex Shader Object
	GLchar *vertexShaderSourceCodePerFragment =
		"#version 420 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_m_matrix;" \
		"uniform mat4 u_v_matrix;" \
		"uniform mat4 u_p_matrix;" \
		"uniform int u_LKeyIsPressed;" \
		"out vec3 tNormVertexShader;" \
		"out vec4 eye_coordinatesVertexShader;" \
		"void main(void)" \
		"{" \
		"if(u_LKeyIsPressed==1)" \
		"{" \
		"eye_coordinatesVertexShader=u_v_matrix*u_m_matrix*vPosition;" \
		"tNormVertexShader=mat3(u_v_matrix*u_m_matrix)*vNormal;" \
		"}" \
		"gl_Position=u_p_matrix*u_v_matrix*u_m_matrix*vPosition;" \
		"}";

	glShaderSource(gVertexShaderObjectPerFragment, 1, (const GLchar **)&vertexShaderSourceCodePerFragment, NULL);
	glCompileShader(gVertexShaderObjectPerFragment);
	//Error Check
	iShaderComileStatus = 0;
	iInfoLogLength = 0;
	szLogInfo = NULL;

	glGetShaderiv(gVertexShaderObjectPerFragment, GL_COMPILE_STATUS, &iShaderComileStatus);
	if (iShaderComileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObjectPerFragment, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szLogInfo = (GLchar *)malloc(iInfoLogLength);
			if (szLogInfo != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObjectPerFragment, iInfoLogLength, &written, szLogInfo);
				fprintf(gpFile, szLogInfo);
				free(szLogInfo);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}


	//Fragment Shader
	gFragmentShaderObjectPerFragment = glCreateShader(GL_FRAGMENT_SHADER);

	//Write Vertex Shader Object
	GLchar *fragmentShaderSourceCodePerFragment =
		"#version 420 core" \
		"\n" \
		"in vec3 tNormVertexShader;" \
		"in vec4 eye_coordinatesVertexShader;" \
		"uniform vec3 u_LdOne;" \
		"uniform vec3 u_LdTwo;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_LsOne;" \
		"uniform vec3 u_LsTwo;" \
		"uniform vec3 u_Ks;" \
		"uniform vec3 u_LaOne;" \
		"uniform vec3 u_LaTwo;" \
		"uniform vec3 u_Ka;" \
		"uniform vec4 u_Light_PositionOne;" \
		"uniform vec4 u_Light_PositionTwo;" \
		"uniform float u_MaterialShininess;" \
		"uniform int u_LKeyIsPressed;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"if(u_LKeyIsPressed==1)" \
		"{" \
		"vec3 tNorm=normalize(tNormVertexShader);"
		"vec3 lightDirectionOne=normalize(vec3(u_Light_PositionOne-eye_coordinatesVertexShader));" \
		"vec3 lightDirectionTwo=normalize(vec3(u_Light_PositionTwo-eye_coordinatesVertexShader));" \
		"float tndotldOne=max(dot(lightDirectionOne,tNorm),0.0);" \
		"float tndotldTwo=max(dot(lightDirectionTwo,tNorm),0.0);" \
		"vec3 ReflectionVectorOne=reflect(-lightDirectionOne,tNorm);" \
		"vec3 ReflectionVectorTwo=reflect(-lightDirectionTwo,tNorm);" \
		"vec3 viewerVector=normalize(vec3(-eye_coordinatesVertexShader.xyz));" \
		"vec3 ambientOne=u_LaOne*u_Ka;" \
		"vec3 ambientTwo=u_LaTwo*u_Ka;" \
		"vec3 diffusedOne=u_LdOne*u_Kd*tndotldOne;" \
		"vec3 diffusedTwo=u_LdTwo*u_Kd*tndotldTwo;" \
		"vec3 specularOne=u_LsOne*u_Ks*pow(max(dot(ReflectionVectorOne,viewerVector),0.0),u_MaterialShininess);" \
		"vec3 specularTwo=u_LsTwo*u_Ks*pow(max(dot(ReflectionVectorTwo,viewerVector),0.0),u_MaterialShininess);" \
		"vec3 phong_ads_light = ambientOne+diffusedOne+specularOne+ambientTwo+diffusedTwo+specularTwo;" \
		"FragColor=vec4(phong_ads_light,1.0);" \
		"}" \
		"else" \
		"{" \
		"FragColor=vec4(1.0,1.0,1.0,1.0);"
		"}" \
		
		"}";


	glShaderSource(gFragmentShaderObjectPerFragment, 1, (const GLchar **)&fragmentShaderSourceCodePerFragment, NULL);
	glCompileShader(gFragmentShaderObjectPerFragment);
	//Error Check
	iShaderComileStatus = 0;
	iInfoLogLength = 0;
	szLogInfo = NULL;

	glGetShaderiv(gFragmentShaderObjectPerFragment, GL_COMPILE_STATUS, &iShaderComileStatus);
	if (iShaderComileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObjectPerFragment, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szLogInfo = (GLchar *)malloc(iInfoLogLength);
			if (szLogInfo != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObjectPerFragment, iInfoLogLength, &written, szLogInfo);
				fprintf(gpFile, szLogInfo);
				free(szLogInfo);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}

	//Create Program
	gShaderProgramObjectPerFragment = glCreateProgram();
	glAttachShader(gShaderProgramObjectPerFragment, gVertexShaderObjectPerFragment);
	glAttachShader(gShaderProgramObjectPerFragment, gFragmentShaderObjectPerFragment);

	glBindAttribLocation(gShaderProgramObjectPerFragment, AMC_ATTRIBUTE_POSITION, "vPosition");

	glBindAttribLocation(gShaderProgramObjectPerFragment, AMC_ATTRIBUTE_NORMAL, "vNormal");

	glLinkProgram(gShaderProgramObjectPerFragment);
	iShaderLinkStatus = 0;
	iInfoLogLength = 0;
	szLogInfo = NULL;

	glGetProgramiv(gShaderProgramObjectPerFragment, GL_LINK_STATUS, &iShaderLinkStatus);
	if (iShaderLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObjectPerFragment, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szLogInfo = (GLchar *)malloc(iInfoLogLength);
			if (szLogInfo != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObjectPerFragment, iInfoLogLength, &written, szLogInfo);
				fprintf(gpFile, szLogInfo);
				free(szLogInfo);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}

	mUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_m_matrix");
	vUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_v_matrix");
	pUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_p_matrix");
	LDUniformOnePerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LdOne");
	LAUniformOnePerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LaOne");
	LSUniformOnePerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LsOne");
	LightPositionUniformOnePerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Light_PositionOne");

	LDUniformTwoPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LdTwo");
	LAUniformTwoPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LaTwo");
	LSUniformTwoPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LsTwo");
	LightPositionUniformTwoPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Light_PositionTwo");


	KDUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Kd");
	KAUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ka");
	KSUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ks");
	MaterialShininessUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_MaterialShininess");
	
	LKeyIsPressedUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LKeyIsPressed");

	const GLfloat Pyramid_vertices[] = {
		// Front face
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		// Right face
		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,

		// Back face
		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		// Left face
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f
	};

	const GLfloat Pyramid_normals[] = {
		// Front face
		0.0f, 0.447214f, 0.894427f,
		0.0f, 0.447214f, 0.894427f,
		0.0f, 0.447214f, 0.894427f,

		// Right face
		0.894427f, 0.447214f, 0.0f,
		0.894427f, 0.447214f, 0.0f,
		0.894427f, 0.447214f, 0.0f,

		// Back face
		0.0f, 0.447214f, -0.894427f,
		0.0f, 0.447214f, -0.894427f,
		0.0f, 0.447214f, -0.894427f,

		// Left face
		-0.894427f, 0.447214f, 0.0f,
		-0.894427f, 0.447214f, 0.0f,
		-0.894427f, 0.447214f, 0.0f
	};

	//Rectangle vao
	glGenVertexArrays(1, &vao_Pyramid);
	glBindVertexArray(vao_Pyramid);
	//Position
	glGenBuffers(1, &vbo_Position_Pyramid);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Position_Pyramid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Pyramid_vertices), Pyramid_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Normals
	glGenBuffers(1, &vbo_Normal_Pyramid);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Normal_Pyramid);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Pyramid_normals),
		Pyramid_normals,
		GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);
	perspectiveProjectionMatrix = mat4::identity();


	lights[0].lightAmbient[0] = 1.0f;
	lights[0].lightAmbient[1] = 0.0f;
	lights[0].lightAmbient[2] = 0.0f;
	
	lights[0].lightDiffused[0] = 1.0f;
	lights[0].lightDiffused[1] = 0.0f;
	lights[0].lightDiffused[2] = 0.0f;
	
	lights[0].lightSpecular[0] = 1.0f;
	lights[0].lightSpecular[1] = 0.0f;
	lights[0].lightSpecular[2] = 0.0f;
	
	lights[0].lightPosition[0] = -2.0f;
	lights[0].lightPosition[1] = 0.0f;
	lights[0].lightPosition[2] = 0.0f;
	lights[0].lightPosition[3] = 1.0f;

	//light1
	lights[1].lightAmbient[0] = 0.0f;
	lights[1].lightAmbient[1] = 0.0f;
	lights[1].lightAmbient[2] = 1.0f;
	lights[1].lightAmbient[3] = 1.0f;

	lights[1].lightDiffused[0] = 0.0f;
	lights[1].lightDiffused[1] = 0.0f;
	lights[1].lightDiffused[2] = 1.0f;
	
	lights[1].lightSpecular[0] = 0.0f;
	lights[1].lightSpecular[1] = 0.0f;
	lights[1].lightSpecular[2] = 1.0f;


	lights[1].lightPosition[0] = 2.0f;
	lights[1].lightPosition[1] = 0.0f;
	lights[1].lightPosition[2] = 0.0f;
	lights[1].lightPosition[3] = 1.0f;

	resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;
}

void unInitialize(void)
{

	if (vbo_Position_Pyramid)
	{
		glDeleteBuffers(1, &vbo_Position_Pyramid);
		vbo_Position_Pyramid = 0;
	}

	if (vbo_Normal_Pyramid)
	{
		glDeleteBuffers(1, &vbo_Normal_Pyramid);
		vbo_Normal_Pyramid = 0;
	}

	if (vao_Pyramid)
	{
		glDeleteBuffers(1, &vao_Pyramid);
		vao_Pyramid = 0;
	}

	GLsizei shaderCount;
	GLsizei shaderNumber;
	if (gShaderProgramObjectPerVertex)
	{
		glUseProgram(gShaderProgramObjectPerVertex);

		glGetProgramiv(gShaderProgramObjectPerVertex, GL_ATTACHED_SHADERS, &shaderCount);
		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint)* shaderCount);
		if (pShaders)
		{
			glGetAttachedShaders(gShaderProgramObjectPerVertex, shaderCount, &shaderCount, pShaders);
			for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
			{
				glDetachShader(gShaderProgramObjectPerVertex, pShaders[shaderNumber]);
				glDeleteShader(pShaders[shaderNumber]);
				pShaders[shaderNumber] = 0;
			}
			free(pShaders);
		}
		glDeleteProgram(gShaderProgramObjectPerVertex);
		gShaderProgramObjectPerVertex = 0;
		glUseProgram(0);
	}
	if (gShaderProgramObjectPerFragment)
	{
		glUseProgram(gShaderProgramObjectPerFragment);

		glGetProgramiv(gShaderProgramObjectPerFragment, GL_ATTACHED_SHADERS, &shaderCount);
		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint)* shaderCount);
		if (pShaders)
		{
			glGetAttachedShaders(gShaderProgramObjectPerFragment, shaderCount, &shaderCount, pShaders);
			for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
			{
				glDetachShader(gShaderProgramObjectPerFragment, pShaders[shaderNumber]);
				glDeleteShader(pShaders[shaderNumber]);
				pShaders[shaderNumber] = 0;
			}
			free(pShaders);
		}
		glDeleteProgram(gShaderProgramObjectPerFragment);
		gShaderProgramObjectPerFragment = 0;
		glUseProgram(0);
	}
	if (bFullScreen == true)
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
	}
	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "File closed successfully");
		fclose(gpFile);
		gpFile = NULL;
	}
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	perspectiveProjectionMatrix = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (bPerVertex)
	{
		glUseProgram(gShaderProgramObjectPerVertex);

		mat4 modelMatrix;
		mat4 viewMatrix;
		mat4 translationMatrix;
		mat4 rotationMatrix;

		modelMatrix = mat4::identity();
		viewMatrix = mat4::identity();
		translationMatrix = mat4::identity();
		rotationMatrix = mat4::identity();

		translationMatrix = translate(0.0f, 0.0f, -5.0f);
		modelMatrix = modelMatrix*translationMatrix;
		rotationMatrix = rotate(rotatePyramid, 0.0f, 1.0f, 0.0f);

		modelMatrix = modelMatrix*rotationMatrix;

		glUniformMatrix4fv(mUniformPerVertex,
			1,
			GL_FALSE,
			modelMatrix);
		glUniformMatrix4fv(vUniformPerVertex,
			1,
			GL_FALSE,
			viewMatrix);
		glUniformMatrix4fv(pUniformPerVertex,
			1,
			GL_FALSE,
			perspectiveProjectionMatrix);
		if (gLighting == 1)
		{
			glUniform1i(LKeyIsPressedUniformPerVertex,
				gLighting);

			glUniform4fv(LightPositionUniformOnePerVertex, 1, (GLfloat*)lights[0].lightPosition);
			glUniform3fv(LAUniformOnePerVertex, 1, (GLfloat*)lights[0].lightAmbient);
			glUniform3fv(LDUniformOnePerVertex, 1, (GLfloat*)lights[0].lightDiffused);
			glUniform3fv(LSUniformOnePerVertex, 1, (GLfloat*)lights[0].lightSpecular);

			glUniform4fv(LightPositionUniformTwoPerVertex, 1, (GLfloat*)lights[1].lightPosition);
			glUniform3fv(LAUniformTwoPerVertex, 1, (GLfloat*)lights[1].lightAmbient);
			glUniform3fv(LDUniformTwoPerVertex, 1, (GLfloat*)lights[1].lightDiffused);
			glUniform3fv(LSUniformTwoPerVertex, 1, (GLfloat*)lights[1].lightSpecular);

			glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
			glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
			glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
			glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
		}
		else
		{
			glUniform1i(LKeyIsPressedUniformPerVertex, gLighting);
		}
		glBindVertexArray(vao_Pyramid);
		glDrawArrays(GL_TRIANGLES, 0, 12);
		glBindVertexArray(0);
		glUseProgram(0);
	}
	else
	{

		glUseProgram(gShaderProgramObjectPerFragment);

		mat4 modelMatrix;
		mat4 viewMatrix;
		mat4 translationMatrix;
		mat4 rotationMatrix;

		modelMatrix = mat4::identity();
		viewMatrix = mat4::identity();
		translationMatrix = mat4::identity();
		rotationMatrix = mat4::identity();

		translationMatrix = translate(0.0f, 0.0f, -5.0f);
		modelMatrix = modelMatrix*translationMatrix;
		rotationMatrix = rotate(rotatePyramid, 0.0f, 1.0f, 0.0f);

		modelMatrix = modelMatrix*rotationMatrix;

		glUniformMatrix4fv(mUniformPerFragment,
			1,
			GL_FALSE,
			modelMatrix);
		glUniformMatrix4fv(vUniformPerFragment,
			1,
			GL_FALSE,
			viewMatrix);
		glUniformMatrix4fv(pUniformPerFragment,
			1,
			GL_FALSE,
			perspectiveProjectionMatrix);
		if (gLighting == 1)
		{
			glUniform1i(LKeyIsPressedUniformPerFragment,
				gLighting);

			glUniform4fv(LightPositionUniformOnePerFragment, 1, (GLfloat*)lights[0].lightPosition);
			glUniform3fv(LAUniformOnePerFragment, 1, (GLfloat*)lights[0].lightAmbient);
			glUniform3fv(LDUniformOnePerFragment, 1, (GLfloat*)lights[0].lightDiffused);
			glUniform3fv(LSUniformOnePerFragment, 1, (GLfloat*)lights[0].lightSpecular);

			glUniform4fv(LightPositionUniformTwoPerFragment, 1, (GLfloat*)lights[1].lightPosition);
			glUniform3fv(LAUniformTwoPerFragment, 1, (GLfloat*)lights[1].lightAmbient);
			glUniform3fv(LDUniformTwoPerFragment, 1, (GLfloat*)lights[1].lightDiffused);
			glUniform3fv(LSUniformTwoPerFragment, 1, (GLfloat*)lights[1].lightSpecular);

			glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
			glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
			glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
			glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
		}
		else
		{
			glUniform1i(LKeyIsPressedUniformPerFragment, gLighting);
		}
		glBindVertexArray(vao_Pyramid);
		glDrawArrays(GL_TRIANGLES, 0, 12);
		glBindVertexArray(0);
		glUseProgram(0);
	}
	SwapBuffers(ghdc);
}

void update(void)
{
	if (gAnimate)
	{
		rotatePyramid -= 0.1f;
		if (rotatePyramid < 0)
		{
			rotatePyramid = 360.0f;
		}
	}
}
