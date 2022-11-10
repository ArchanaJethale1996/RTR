#include<windows.h>
#include<GL/glew.h>
#include<gl/GL.h>
#include<stdio.h>
#include"vmath.h"
#include"Sphere.h"
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"Sphere.lib")

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

void draw24Sphere();
GLuint gVertexShaderObjectPerVertex;
GLuint gFragmentShaderObjectPerVertex;
GLuint gShaderProgramObjectPerVertex;

GLuint gVertexShaderObjectPerFragment;
GLuint gFragmentShaderObjectPerFragment;
GLuint gShaderProgramObjectPerFragment;
bool gAnimate = false;
int gLighting = 0;
bool bPerVertex = true;
//vertices for sphere
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumElements, gNumVertices;

GLfloat lightAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat lightDiffused[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 0.0f,3.0f,3.0f,1.0f };

GLfloat light_Model_Ambient[] = { 0.2f,0.2f,0.2f,1.0f };
GLfloat light_Model_Local_Viewer[] = { 0.0f };
GLUquadric *quadric[24];

GLfloat angleOfXRotation = 0.0f;
GLfloat angleOfYRotation = 0.0f;
GLfloat angleOfZRotation = 0.0f;
GLint KeyPressed = 0;

float rotateSphere = 0.0f;
enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0
};

GLuint vao_Sphere;
GLuint vbo_Position_Sphere;
GLuint vbo_Normal_Sphere;
GLuint vbo_Element_Sphere;
GLuint mUniformPerVertex, vUniformPerVertex, pUniformPerVertex, KDUniformPerVertex, KAUniformPerVertex, KSUniformPerVertex, MaterialShininessUniformPerVertex, LKeyIsPressedUniformPerVertex;
GLuint LDUniformZeroPerVertex, LAUniformZeroPerVertex, LSUniformZeroPerVertex, LightPositionUniformZeroPerVertex;

GLuint mUniformPerFragment, vUniformPerFragment, pUniformPerFragment, KDUniformPerFragment, KAUniformPerFragment, KSUniformPerFragment, MaterialShininessUniformPerFragment, LKeyIsPressedUniformPerFragment;
GLuint LDUniformZeroPerFragment, LAUniformZeroPerFragment, LSUniformZeroPerFragment, LightPositionUniformZeroPerFragment;

mat4 perspectiveProjectionMatrix;

GLuint samplerUniform;
int gWidth, gHeight;
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
		case 'v':
		case 'V':
			bPerVertex = !bPerVertex;
			break;
		case 'L':
		case 'l':
			if (gLighting == 0)
				gLighting = 1;
			else
				gLighting = 0;
			break;
		case 'X':
		case 'x':
			KeyPressed = 1;
			lightPosition[0] = 0.0f;
			lightPosition[1] = 0.0f;
			lightPosition[2] = 100.0f;
			angleOfXRotation = 0.0f;
			break;
		case 'Y':
		case 'y':
			KeyPressed = 2;
			lightPosition[1] = 0.0f;
			lightPosition[2] = 0.0f;
			lightPosition[0] = 100.0f;
			angleOfYRotation = 0.0f;
			break;
		case 'Z':
		case 'z':
			KeyPressed = 3;
			lightPosition[1] = 100.0f;
			lightPosition[0] = 0.0f;
			lightPosition[2] = 0.0f;
			angleOfZRotation = 0.0f;
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
	///////////////////////////////Per Vertex
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
		"uniform vec3 u_LdZero;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_LsZero;" \
		"uniform vec3 u_Ks;" \
		"uniform vec3 u_LaZero;" \
		"uniform vec3 u_Ka;" \
		"uniform vec4 u_Light_PositionZero;" \
		"uniform float u_MaterialShininess;" \
		"uniform int u_LKeyIsPressed;" \
		"out vec3 phong_ads_light;" \
		"void main(void)" \
		"{" \
		"if(u_LKeyIsPressed==1)" \
		"{" \
		"vec4 eye_coordinates=u_v_matrix*u_m_matrix*vPosition;" \

		"vec3 tNorm=normalize(mat3(u_v_matrix*u_m_matrix)*vNormal);" \
		"vec3 lightDirectionZero=normalize(vec3(u_Light_PositionZero-eye_coordinates));" \
		"float tndotldZero=max(dot(lightDirectionZero,tNorm),0.0);" \
		"vec3 ReflectionVectorZero=reflect(-lightDirectionZero,tNorm);" \
		"vec3 viewerVector=normalize(vec3(-eye_coordinates.xyz));" \
		"vec3 ambientZero=u_LaZero*u_Ka;" \
		"vec3 diffusedZero=u_LdZero*u_Kd*tndotldZero;" \
		"vec3 specularZero=u_LsZero*u_Ks*pow(max(dot(ReflectionVectorZero,viewerVector),0.0),u_MaterialShininess);" \
		"phong_ads_light = ambientZero+diffusedZero+specularZero;" \
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
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor=vec4(phong_ads_light,1.0);" \
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
	LDUniformZeroPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LdZero");
	LAUniformZeroPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LaZero");
	LSUniformZeroPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LsZero");
	LightPositionUniformZeroPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Light_PositionZero");

	KDUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Kd");
	KAUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ka");
	KSUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ks");
	MaterialShininessUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_MaterialShininess");
	LKeyIsPressedUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LKeyIsPressed");


	//////////////////////////////////////////////////Per Fragment
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
		"uniform vec3 u_LdZero;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_LsZero;" \
		"uniform vec3 u_Ks;" \
		"uniform vec3 u_LaZero;" \
		"uniform vec3 u_Ka;" \
		"uniform vec4 u_Light_PositionZero;" \
		"uniform float u_MaterialShininess;" \
		"uniform int u_LKeyIsPressed;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"if(u_LKeyIsPressed==1)" \
		"{" \
		"vec3 tNorm=normalize(tNormVertexShader);"
		"vec3 lightDirectionZero=normalize(vec3(u_Light_PositionZero-eye_coordinatesVertexShader));" \
		"float tndotldZero=max(dot(lightDirectionZero,tNorm),0.0);" \
		"vec3 ReflectionVectorZero=reflect(-lightDirectionZero,tNorm);" \
		"vec3 viewerVector=normalize(vec3(-eye_coordinatesVertexShader.xyz));" \
		"vec3 ambientZero=u_LaZero*u_Ka;" \
		"vec3 diffusedZero=u_LdZero*u_Kd*tndotldZero;" \
		"vec3 specularZero=u_LsZero*u_Ks*pow(max(dot(ReflectionVectorZero,viewerVector),0.0),u_MaterialShininess);" \
		"vec3 phong_ads_light = ambientZero+diffusedZero+specularZero;" \
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
    LDUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ld");
    KDUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Kd");
    LAUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_La");
    KAUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ka");
    LSUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ls");
    KSUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ks");
    MaterialShininessUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_MaterialShininess");
    LightPositionUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Light_Position");
    LKeyIsPressedUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LKeyIsPressed");
    
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	//Rectangle vao
	glGenVertexArrays(1, &vao_Sphere);
	glBindVertexArray(vao_Sphere);
	//Position
	glGenBuffers(1, &vbo_Position_Sphere);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Position_Sphere);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Normals
	glGenBuffers(1, &vbo_Normal_Sphere);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Normal_Sphere);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sphere_normals),
		sphere_normals,
		GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Elements
	glGenBuffers(1, &vbo_Element_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);
	perspectiveProjectionMatrix = mat4::identity();
	resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;
}

void unInitialize(void)
{

	if (vbo_Position_Sphere)
	{
		glDeleteBuffers(1, &vbo_Position_Sphere);
		vbo_Position_Sphere = 0;
	}

	if (vbo_Normal_Sphere)
	{
		glDeleteBuffers(1, &vbo_Normal_Sphere);
		vbo_Normal_Sphere = 0;
	}

	if (vbo_Element_Sphere)
	{
		glDeleteBuffers(1, &vbo_Element_Sphere);
		vbo_Element_Sphere = 0;
	}

	if (vao_Sphere)
	{
		glDeleteBuffers(1, &vao_Sphere);
		vao_Sphere = 0;
	}

	GLsizei shaderCount;
	GLsizei shaderNumber;
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
	gWidth = width;
	gHeight = height;
	perspectiveProjectionMatrix = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}


void draw24SpherePerFragment()
{
	GLfloat materialAmbient[4];
	GLfloat materialDiffused[4];
	GLfloat materialSpecular[4];
	GLfloat materialShininess;

	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 translationMatrix;
	mat4 scaleMatrix;
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix= mat4::identity();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//1st sphere 1st col emrald
	materialAmbient[0] = 0.0215f;
	materialAmbient[1] = 0.1745f;
	materialAmbient[2] = 0.0215f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.07568f;
	materialDiffused[1] = 0.61424f;
	materialDiffused[2] = 0.07568f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.633f;
	materialSpecular[1] = 0.727811f;
	materialSpecular[2] = 0.633f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.6f * 128;
	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 0, (gHeight / 4) * 3,gWidth/6,gHeight/6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);
	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);




	////2nd sphere on  1st col,jade

	materialAmbient[0] = 0.135f;
	materialAmbient[1] = 0.2225f;
	materialAmbient[2] = 0.1575f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.54f;
	materialDiffused[1] = 0.89f;
	materialDiffused[2] = 0.63f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.316228f;
	materialSpecular[1] = 0.316228f;
	materialSpecular[2] = 0.316228f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.1f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 1, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd sphere on  1st col,obsidian
	materialAmbient[0] = 0.05375f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.06625f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.18275f;
	materialDiffused[1] = 0.17f;
	materialDiffused[2] = 0.22525f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.332741f;
	materialSpecular[1] = 0.328634f;
	materialSpecular[2] = 0.346435f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.3f * 128;
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth/ 6) * 2, (gHeight / 4) * 3,  gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////4th sphere on  1st col,pearl

	materialAmbient[0] = 0.25;
	materialAmbient[1] = 0.20725f;
	materialAmbient[2] = 0.20725f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 1.0f;
	materialDiffused[1] = 0.829f;
	materialDiffused[2] = 0.829f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.296648f;
	materialSpecular[1] = 0.296648f;
	materialSpecular[2] = 0.296648f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess = 0.88f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 3, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix; 
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////5th sphere on  1st col,ruby

	materialAmbient[0] = 0.1745f;
	materialAmbient[1] = 0.01175f;
	materialAmbient[2] = 0.01175f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.61424f;
	materialDiffused[1] = 0.04136f;
	materialDiffused[2] = 0.04136f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.727811f;
	materialSpecular[1] = 0.626959f;
	materialSpecular[2] = 0.626959f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.6f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 4, (gHeight / 4) * 3,gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix; 
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	////6th sphere on  1st col,turquoise

	materialAmbient[0] = 0.1f;
	materialAmbient[1] = 0.18725f;
	materialAmbient[2] = 0.1745f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.396f;
	materialDiffused[1] = 0.074151f;
	materialDiffused[2] = 0.69102f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.297254f;
	materialSpecular[1] = 0.30829f;
	materialSpecular[2] = 0.306678f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.1f * 128;
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 5, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);



	////2nd Col

	//1st sphere 2nd col brass
	materialAmbient[0] = 0.329412f;
	materialAmbient[1] = 0.223529f;
	materialAmbient[2] = 0.027451f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.780392f;
	materialDiffused[1] = 0.568627f;
	materialDiffused[2] = 0.113725f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.992157f;
	materialSpecular[1] = 0.941176f;
	materialSpecular[2] = 0.807843f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.21794872f * 128;
	
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 0, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////2nd sphere on  2nd col,bronze

	materialAmbient[0] = 0.2125f;
	materialAmbient[1] = 0.1275f;
	materialAmbient[2] = 0.054f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.714f;
	materialDiffused[1] = 0.4284f;
	materialDiffused[2] = 0.18144f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.393548f;
	materialSpecular[1] = 0.271906f;
	materialSpecular[2] = 0.166721f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.2f * 128;
	
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 1, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd sphere on  2nd col,chrome

	materialAmbient[0] = 0.25f;
	materialAmbient[1] = 0.25f;
	materialAmbient[2] = 0.25f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.4f;
	materialDiffused[1] = 0.4f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.774597f;
	materialSpecular[1] = 0.774597f;
	materialSpecular[2] = 0.774597f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.6f * 128;
	

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 2, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////4th sphere on  2nd col,copper

	materialAmbient[0] = 0.19125f;
	materialAmbient[1] = 0.0735f;
	materialAmbient[2] = 0.0225f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.7038f;
	materialDiffused[1] = 0.27048f;
	materialDiffused[2] = 0.0828f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.256777f;
	materialSpecular[1] = 0.137622f;
	materialSpecular[2] = 0.086014f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.1f * 128;
	

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 3, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////5th sphere on  2nd col,gold

	materialAmbient[0] = 0.2472f;
	materialAmbient[1] = 0.1995f;
	materialAmbient[2] = 0.0745f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.75164f;
	materialDiffused[1] = 0.60648f;
	materialDiffused[2] = 0.22648f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.628281f;
	materialSpecular[1] = 0.555802f;
	materialSpecular[2] = 0.366065f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.4f * 128;


	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 4, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////6th sphere on  2nd col,silver

	materialAmbient[0] = 0.19225f;
	materialAmbient[1] = 0.19225f;
	materialAmbient[2] = 0.19225f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.5074f;
	materialDiffused[1] = 0.5074f;
	materialDiffused[2] = 0.5074f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.508273f;
	materialSpecular[1] = 0.508273f;
	materialSpecular[2] = 0.508273f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.4f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 5, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd col
	//1st sphere 3rd col black
	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.0f;
	materialDiffused[1] = 0.0f;
	materialDiffused[2] = 0.0f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.5f;
	materialSpecular[1] = 0.5;
	materialSpecular[2] = 0.5f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.25f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 0, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	////2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.1f;
	materialAmbient[2] = 0.06f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.0f;
	materialDiffused[1] = 0.50980392f;
	materialDiffused[2] = 0.5098392f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.50196078f;
	materialSpecular[1] = 0.50196078f;
	materialSpecular[2] = 0.50196078f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.25f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 1, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd sphere on  3rd col,green

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.1f;
	materialDiffused[1] = 0.35f;
	materialDiffused[2] = 0.1f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.45f;
	materialSpecular[1] = 0.55f;
	materialSpecular[2] = 0.45f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.25f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 2, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	//
	////4th sphere on  3rd col,red
	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.0f;
	materialDiffused[2] = 0.0f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.7f;
	materialSpecular[1] = 0.6f;
	materialSpecular[2] = 0.6f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.25f * 128;

	glMatrixMode(GL_MODELVIEW);

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 3, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	////5th sphere on  3rd col,white

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.55f;
	materialDiffused[1] = 0.55f;
	materialDiffused[2] = 0.55f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.70f;
	materialSpecular[1] = 0.70f;
	materialSpecular[2] = 0.70f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.25f * 128;
	

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 4, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.0f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.60f;
	materialSpecular[1] = 0.60f;
	materialSpecular[2] = 0.50f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.25f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 5, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	////4th col
	////1st sphere 3rd col black
	materialAmbient[0] = 0.02f;
	materialAmbient[1] = 0.02f;
	materialAmbient[2] = 0.02f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.01f;
	materialDiffused[1] = 0.01f;
	materialDiffused[2] = 0.01f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.4f;
	materialSpecular[1] = 0.4f;
	materialSpecular[2] = 0.4f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.78125f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 0, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.05f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.4f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.5f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.04f;
	materialSpecular[1] = 0.7f;
	materialSpecular[2] = 0.7f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.078125f * 128;
	
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 1, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd sphere on  3rd col,green
	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.4f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.04f;
	materialSpecular[1] = 0.7f;
	materialSpecular[2] = 0.04f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.078125f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 2, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	//////4th sphere on  3rd col,red

	materialAmbient[0] = 0.05f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.4f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.7f;
	materialSpecular[1] = 0.04f;
	materialSpecular[2] = 0.04f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.078125f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 3, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	//////5th sphere on  3rd col,white

	materialAmbient[0] = 0.05f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.05f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.5f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.70f;
	materialSpecular[1] = 0.70f;
	materialSpecular[2] = 0.70f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.078125f * 128;
	
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);


	glViewport((gWidth / 6) * 4, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.05f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	
	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;
	
	materialSpecular[0] = 0.70f;
	materialSpecular[1] = 0.70f;
	materialSpecular[2] = 0.04f;
	materialSpecular[3] = 1.0f;
	
	materialShininess = 0.078125f * 128;
	
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

	glViewport((gWidth / 6) * 5, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

}


void draw24SpherePerVertex()
{
	GLfloat materialAmbient[4];
	GLfloat materialDiffused[4];
	GLfloat materialSpecular[4];
	GLfloat materialShininess;

	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 translationMatrix;
	mat4 scaleMatrix;
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//1st sphere 1st col emrald
	materialAmbient[0] = 0.0215f;
	materialAmbient[1] = 0.1745f;
	materialAmbient[2] = 0.0215f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.07568f;
	materialDiffused[1] = 0.61424f;
	materialDiffused[2] = 0.07568f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.633f;
	materialSpecular[1] = 0.727811f;
	materialSpecular[2] = 0.633f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.6f * 128;
	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 0, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);
	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);




	////2nd sphere on  1st col,jade

	materialAmbient[0] = 0.135f;
	materialAmbient[1] = 0.2225f;
	materialAmbient[2] = 0.1575f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.54f;
	materialDiffused[1] = 0.89f;
	materialDiffused[2] = 0.63f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.316228f;
	materialSpecular[1] = 0.316228f;
	materialSpecular[2] = 0.316228f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.1f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 1, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd sphere on  1st col,obsidian
	materialAmbient[0] = 0.05375f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.06625f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.18275f;
	materialDiffused[1] = 0.17f;
	materialDiffused[2] = 0.22525f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.332741f;
	materialSpecular[1] = 0.328634f;
	materialSpecular[2] = 0.346435f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.3f * 128;
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 2, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////4th sphere on  1st col,pearl

	materialAmbient[0] = 0.25;
	materialAmbient[1] = 0.20725f;
	materialAmbient[2] = 0.20725f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 1.0f;
	materialDiffused[1] = 0.829f;
	materialDiffused[2] = 0.829f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.296648f;
	materialSpecular[1] = 0.296648f;
	materialSpecular[2] = 0.296648f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess = 0.88f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 3, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////5th sphere on  1st col,ruby

	materialAmbient[0] = 0.1745f;
	materialAmbient[1] = 0.01175f;
	materialAmbient[2] = 0.01175f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.61424f;
	materialDiffused[1] = 0.04136f;
	materialDiffused[2] = 0.04136f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.727811f;
	materialSpecular[1] = 0.626959f;
	materialSpecular[2] = 0.626959f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.6f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 4, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	////6th sphere on  1st col,turquoise

	materialAmbient[0] = 0.1f;
	materialAmbient[1] = 0.18725f;
	materialAmbient[2] = 0.1745f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.396f;
	materialDiffused[1] = 0.074151f;
	materialDiffused[2] = 0.69102f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.297254f;
	materialSpecular[1] = 0.30829f;
	materialSpecular[2] = 0.306678f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.1f * 128;
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 5, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);



	////2nd Col

	//1st sphere 2nd col brass
	materialAmbient[0] = 0.329412f;
	materialAmbient[1] = 0.223529f;
	materialAmbient[2] = 0.027451f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.780392f;
	materialDiffused[1] = 0.568627f;
	materialDiffused[2] = 0.113725f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.992157f;
	materialSpecular[1] = 0.941176f;
	materialSpecular[2] = 0.807843f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.21794872f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 0, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////2nd sphere on  2nd col,bronze

	materialAmbient[0] = 0.2125f;
	materialAmbient[1] = 0.1275f;
	materialAmbient[2] = 0.054f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.714f;
	materialDiffused[1] = 0.4284f;
	materialDiffused[2] = 0.18144f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.393548f;
	materialSpecular[1] = 0.271906f;
	materialSpecular[2] = 0.166721f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.2f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 1, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd sphere on  2nd col,chrome

	materialAmbient[0] = 0.25f;
	materialAmbient[1] = 0.25f;
	materialAmbient[2] = 0.25f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.4f;
	materialDiffused[1] = 0.4f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.774597f;
	materialSpecular[1] = 0.774597f;
	materialSpecular[2] = 0.774597f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.6f * 128;


	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 2, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////4th sphere on  2nd col,copper

	materialAmbient[0] = 0.19125f;
	materialAmbient[1] = 0.0735f;
	materialAmbient[2] = 0.0225f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.7038f;
	materialDiffused[1] = 0.27048f;
	materialDiffused[2] = 0.0828f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.256777f;
	materialSpecular[1] = 0.137622f;
	materialSpecular[2] = 0.086014f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.1f * 128;


	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 3, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////5th sphere on  2nd col,gold

	materialAmbient[0] = 0.2472f;
	materialAmbient[1] = 0.1995f;
	materialAmbient[2] = 0.0745f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.75164f;
	materialDiffused[1] = 0.60648f;
	materialDiffused[2] = 0.22648f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.628281f;
	materialSpecular[1] = 0.555802f;
	materialSpecular[2] = 0.366065f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.4f * 128;


	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 4, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////6th sphere on  2nd col,silver

	materialAmbient[0] = 0.19225f;
	materialAmbient[1] = 0.19225f;
	materialAmbient[2] = 0.19225f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.5074f;
	materialDiffused[1] = 0.5074f;
	materialDiffused[2] = 0.5074f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.508273f;
	materialSpecular[1] = 0.508273f;
	materialSpecular[2] = 0.508273f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.4f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 5, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd col
	//1st sphere 3rd col black
	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.0f;
	materialDiffused[1] = 0.0f;
	materialDiffused[2] = 0.0f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.5f;
	materialSpecular[1] = 0.5;
	materialSpecular[2] = 0.5f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.25f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 0, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	////2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.1f;
	materialAmbient[2] = 0.06f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.0f;
	materialDiffused[1] = 0.50980392f;
	materialDiffused[2] = 0.5098392f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.50196078f;
	materialSpecular[1] = 0.50196078f;
	materialSpecular[2] = 0.50196078f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.25f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 1, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd sphere on  3rd col,green

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.1f;
	materialDiffused[1] = 0.35f;
	materialDiffused[2] = 0.1f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.45f;
	materialSpecular[1] = 0.55f;
	materialSpecular[2] = 0.45f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.25f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 2, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	//
	////4th sphere on  3rd col,red
	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.0f;
	materialDiffused[2] = 0.0f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.7f;
	materialSpecular[1] = 0.6f;
	materialSpecular[2] = 0.6f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.25f * 128;

	glMatrixMode(GL_MODELVIEW);

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 3, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	////5th sphere on  3rd col,white

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.55f;
	materialDiffused[1] = 0.55f;
	materialDiffused[2] = 0.55f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.70f;
	materialSpecular[1] = 0.70f;
	materialSpecular[2] = 0.70f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.25f * 128;


	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 4, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.0f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.60f;
	materialSpecular[1] = 0.60f;
	materialSpecular[2] = 0.50f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.25f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 5, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;
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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	////4th col
	////1st sphere 3rd col black
	materialAmbient[0] = 0.02f;
	materialAmbient[1] = 0.02f;
	materialAmbient[2] = 0.02f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.01f;
	materialDiffused[1] = 0.01f;
	materialDiffused[2] = 0.01f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.4f;
	materialSpecular[1] = 0.4f;
	materialSpecular[2] = 0.4f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.78125f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 0, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.05f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.4f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.5f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.04f;
	materialSpecular[1] = 0.7f;
	materialSpecular[2] = 0.7f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.078125f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 1, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////3rd sphere on  3rd col,green
	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.4f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.04f;
	materialSpecular[1] = 0.7f;
	materialSpecular[2] = 0.04f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.078125f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 2, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	//////4th sphere on  3rd col,red

	materialAmbient[0] = 0.05f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.4f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.7f;
	materialSpecular[1] = 0.04f;
	materialSpecular[2] = 0.04f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.078125f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 3, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	//////5th sphere on  3rd col,white

	materialAmbient[0] = 0.05f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.05f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.5f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.70f;
	materialSpecular[1] = 0.70f;
	materialSpecular[2] = 0.70f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.078125f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);


	glViewport((gWidth / 6) * 4, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);


	////6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.05f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;

	materialSpecular[0] = 0.70f;
	materialSpecular[1] = 0.70f;
	materialSpecular[2] = 0.04f;
	materialSpecular[3] = 1.0f;

	materialShininess = 0.078125f * 128;

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
	glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
	glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
	glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

	glViewport((gWidth / 6) * 5, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
	translationMatrix = translate(0.0f, 0.0f, -1.0f);

	modelMatrix = modelMatrix*translationMatrix;
	scaleMatrix = scale(0.7f, 0.7f, 0.7f);
	modelMatrix = modelMatrix*scaleMatrix;

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

	glBindVertexArray(vao_Sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element_Sphere);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

}

void update(void)
{
	rotateSphere = rotateSphere + 0.01f;
	if (rotateSphere >= 360.0f)
	{
		rotateSphere = 0.0f;
	}
}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (bPerVertex)
	{
		glUseProgram(gShaderProgramObjectPerVertex);
		if (gLighting == 1)
		{
			glUniform1i(LKeyIsPressedUniformPerVertex,
				gLighting);

			if (KeyPressed == 1)
			{
				lightPosition[0] = 0.0f;
				lightPosition[1] = sinf(rotateSphere) * 100.0f - 3.0f;
				lightPosition[2] = cosf(rotateSphere) * 100.0f - 3.0f;
			}

			if (KeyPressed == 2)
			{
				lightPosition[0] = sinf(rotateSphere) * 100.0f - 3.0f;
				lightPosition[1] = 0.0f;
				lightPosition[2] = cosf(rotateSphere) * 100.0f - 3.0f;
			}

			if (KeyPressed == 3)
			{
				lightPosition[0] = sinf(rotateSphere) * 100.0f - 3.0f;
				lightPosition[1] = cosf(rotateSphere) * 100.0f - 3.0f;
				lightPosition[2] = 0.0f;
			}
			glUniform4fv(LightPositionUniformZeroPerVertex, 1, (GLfloat*)lightPosition);
			glUniform3fv(LAUniformZeroPerVertex, 1, (GLfloat*)lightAmbient);
			glUniform3fv(LDUniformZeroPerVertex, 1, (GLfloat*)lightDiffused);
			glUniform3fv(LSUniformZeroPerVertex, 1, (GLfloat*)lightSpecular);

			draw24SpherePerVertex();
		}
		else
		{

			mat4 modelMatrix;
			mat4 viewMatrix;
			mat4 translationMatrix;

			modelMatrix = mat4::identity();
			viewMatrix = mat4::identity();
			translationMatrix = mat4::identity();

			translationMatrix = translate(0.0f, 0.0f, -3.0f);
			modelMatrix = modelMatrix*translationMatrix;

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

			glUniform1i(LKeyIsPressedUniformPerVertex, gLighting);
			draw24SpherePerVertex();
		}
		glUseProgram(0);
	}
	else
	{
		glUseProgram(gShaderProgramObjectPerFragment);
		if (gLighting == 1)
		{
			glUniform1i(LKeyIsPressedUniformPerFragment,
				gLighting);

			if (KeyPressed == 1)
			{
				lightPosition[0] = 0.0f;
				lightPosition[1] = sinf(rotateSphere) * 100.0f - 3.0f;
				lightPosition[2] = cosf(rotateSphere) * 100.0f - 3.0f;
			}

			if (KeyPressed == 2)
			{
				lightPosition[0] = sinf(rotateSphere) * 100.0f - 3.0f;
				lightPosition[1] = 0.0f;
				lightPosition[2] = cosf(rotateSphere) * 100.0f - 3.0f;
			}

			if (KeyPressed == 3)
			{
				lightPosition[0] = sinf(rotateSphere) * 100.0f - 3.0f;
				lightPosition[1] = cosf(rotateSphere) * 100.0f - 3.0f;
				lightPosition[2] = 0.0f;
			}
			glUniform4fv(LightPositionUniformZeroPerFragment, 1, (GLfloat*)lightPosition);
			glUniform3fv(LAUniformZeroPerFragment, 1, (GLfloat*)lightAmbient);
			glUniform3fv(LDUniformZeroPerFragment, 1, (GLfloat*)lightDiffused);
			glUniform3fv(LSUniformZeroPerFragment, 1, (GLfloat*)lightSpecular);

			draw24SpherePerFragment();
		}
		else
		{

			mat4 modelMatrix;
			mat4 viewMatrix;
			mat4 translationMatrix;

			modelMatrix = mat4::identity();
			viewMatrix = mat4::identity();
			translationMatrix = mat4::identity();

			translationMatrix = translate(0.0f, 0.0f, -3.0f);
			modelMatrix = modelMatrix*translationMatrix;

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

			glUniform1i(LKeyIsPressedUniformPerFragment, gLighting);
			draw24SpherePerFragment();
		}
		glUseProgram(0);
	}
	SwapBuffers(ghdc);
}
