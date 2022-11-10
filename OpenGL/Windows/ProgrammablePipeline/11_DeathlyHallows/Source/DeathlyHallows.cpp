
#include<windows.h>
#include<GL/glew.h>
#include<gl/GL.h>
#include<stdio.h>
#include"vmath.h"
#include<math.h>

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

//macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define PI 3.14169
#define NUM_POINTS 1000
using namespace vmath;

//global variables
HDC ghdc = NULL;
HWND ghwnd = NULL;
HGLRC ghrc = NULL;
bool gbActiveWindow = false;
FILE *gpFile = NULL;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0
};

GLuint vao_Circle, vao_Line,vao_Traiangle;
GLuint vbo_Position_Circle, vbo_Position_Line, vbo_Position_Triangle;
GLuint mvpUniform;
mat4 perspectiveProjectionMatrix;
GLfloat CircleRotate = 0.0f;
bool triangleComplete = false;
bool circleComplete = false;
bool lineComplete = false;

float xTranslateLine = 0.0f, yTranslateLine = 1.0f;
float xTranslateTriangle = 2.0f, yTranslateTriangle = 0.0f;
float xTranslateCircle = -2.0f, yTranslateCircle = -1.0f;



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

	//Vertex Shader
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//Write Vertex Shader Object
	GLchar *vertexShaderSourceCode =
		"#version 420 core" \
		"\n" \
		"in vec4 vPosition;" \
		"uniform mat4 u_mvp_matrix;"
		"void main(void)" \
		"{" \
		"gl_Position=u_mvp_matrix*vPosition;" \
		"}";

	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
	glCompileShader(gVertexShaderObject);
	//Error Check
	GLint iShaderComileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar* szLogInfo = NULL;

	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderComileStatus);
	if (iShaderComileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szLogInfo = (GLchar *)malloc(iInfoLogLength);
			if (szLogInfo != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szLogInfo);
				fprintf(gpFile, szLogInfo);
				free(szLogInfo);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}


	//Fragment Shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//Write Vertex Shader Object
	GLchar *fragmentShaderSourceCode =
		"#version 420 core" \
		"\n" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor=vec4(1.0,1.0,1.0,1.0);" \
		"}";

	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
	glCompileShader(gFragmentShaderObject);
	//Error Check
	iShaderComileStatus = 0;
	iInfoLogLength = 0;
	szLogInfo = NULL;

	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderComileStatus);
	if (iShaderComileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szLogInfo = (GLchar *)malloc(iInfoLogLength);
			if (szLogInfo != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szLogInfo);
				fprintf(gpFile, szLogInfo);
				free(szLogInfo);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}

	//Create Program
	gShaderProgramObject = glCreateProgram();
	glAttachShader(gShaderProgramObject, gVertexShaderObject);
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");

	//glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_COLOR, "vColor");

	glLinkProgram(gShaderProgramObject);
	GLint iShaderLinkStatus = 0;
	iInfoLogLength = 0;
	szLogInfo = NULL;

	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderLinkStatus);
	if (iShaderLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szLogInfo = (GLchar *)malloc(iInfoLogLength);
			if (szLogInfo != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szLogInfo);
				fprintf(gpFile, szLogInfo);
				free(szLogInfo);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}

	mvpUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");

	GLfloat circleVertices[NUM_POINTS *3];
	int cnt=0;
	for (int i = 0; i < NUM_POINTS; i++)
	{
		double angle = (2 * PI * i) / NUM_POINTS;
		circleVertices[cnt++] = (GLfloat)(0.62f*cos(angle));
		circleVertices[cnt++]=(GLfloat)(0.62f*sin(angle));
		circleVertices[cnt++]=0.0f;
	}

	GLfloat lineVertices[] = {
		0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
	};

	GLfloat triangleVertices[] = {
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
	};

	//Traingle
	glGenVertexArrays(1, &vao_Traiangle);
	glBindVertexArray(vao_Traiangle);
	//Position
	glGenBuffers(1, &vbo_Position_Triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Position_Triangle);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(triangleVertices),
		triangleVertices,
		GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glGenVertexArrays(1, &vao_Circle);
	glBindVertexArray(vao_Circle);
	//Position
	glGenBuffers(1, &vbo_Position_Circle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Position_Circle);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(circleVertices),
		circleVertices,
		GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);


	glGenVertexArrays(1, &vao_Line);
	glBindVertexArray(vao_Line);
	//Position
	glGenBuffers(1, &vbo_Position_Line);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Position_Line);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(lineVertices),
		lineVertices,
		GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	perspectiveProjectionMatrix = mat4::identity();
	resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;
}


void unInitialize(void)
{

	if (vbo_Position_Triangle)
	{
		glDeleteBuffers(1, &vbo_Position_Triangle);
		vbo_Position_Triangle = 0;
	}
	
	if (vao_Traiangle)
	{
		glDeleteBuffers(1, &vao_Traiangle);
		vao_Traiangle = 0;
	}

	if (vbo_Position_Line)
	{
		glDeleteBuffers(1, &vbo_Position_Line);
		vbo_Position_Line = 0;
	}

	if (vao_Line)
	{
		glDeleteBuffers(1, &vao_Line);
		vao_Line = 0;
	}

	if (vbo_Position_Circle)
	{
		glDeleteBuffers(1, &vbo_Position_Circle);
		vbo_Position_Circle = 0;
	}

	if (vao_Circle)
	{
		glDeleteBuffers(1, &vao_Circle);
		vao_Circle = 0;
	}
	GLsizei shaderCount;
	GLsizei shaderNumber;
	if (gShaderProgramObject)
	{
		glUseProgram(gShaderProgramObject);

		glGetProgramiv(gShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);
		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint)* shaderCount);
		if (pShaders)
		{
			glGetAttachedShaders(gShaderProgramObject, shaderCount, &shaderCount, pShaders);
			for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
			{
				glDetachShader(gShaderProgramObject, pShaders[shaderNumber]);
				glDeleteShader(pShaders[shaderNumber]);
				pShaders[shaderNumber] = 0;
			}
			free(pShaders);
		}
		glDeleteProgram(gShaderProgramObject);
		gShaderProgramObject = 0;
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
	glUseProgram(gShaderProgramObject);

	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 translationMatrix;
	
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	
	
	if (triangleComplete == false)
	{
		translationMatrix = translate(xTranslateTriangle, yTranslateTriangle, 0.0f);
		modelViewMatrix = modelViewMatrix*translationMatrix;
	}
	translationMatrix = translate(0.0f, 0.0f, -3.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;

	glLineWidth(15.0f);

	modelViewProjectionMatrix = perspectiveProjectionMatrix*modelViewMatrix;

	glUniformMatrix4fv(mvpUniform,
		1,
		GL_FALSE,
		modelViewProjectionMatrix);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBindVertexArray(vao_Traiangle);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	

	if (triangleComplete)
	{
		modelViewMatrix = mat4::identity();
		modelViewProjectionMatrix = mat4::identity();
		translationMatrix = mat4::identity();

		if (circleComplete == false)
		{
			translationMatrix = translate(xTranslateCircle, xTranslateCircle, 0.0f);
			modelViewMatrix = modelViewMatrix*translationMatrix;
		}
		translationMatrix = translate(0.0f, 0.31f - 0.68f, -3.0f);
		modelViewMatrix = modelViewMatrix*translationMatrix;
		modelViewMatrix = modelViewMatrix*rotate(CircleRotate, 0.0f, 1.0f, 0.0f);
		modelViewProjectionMatrix = perspectiveProjectionMatrix*modelViewMatrix;

		glLineWidth(9.0f);	
		glUniformMatrix4fv(mvpUniform,
			1,
			GL_FALSE,
			modelViewProjectionMatrix);
		glBindVertexArray(vao_Circle);
		glDrawArrays(GL_LINE_LOOP, 0, 1000);
		glBindVertexArray(0);
	}

	if (circleComplete)
	{
		modelViewMatrix = mat4::identity();
		modelViewProjectionMatrix = mat4::identity();
		translationMatrix = mat4::identity();

		glLineWidth(10.0f);
		if (lineComplete == false)
		{

			translationMatrix = translate(xTranslateLine, yTranslateLine, 0.0f);
			modelViewMatrix = modelViewMatrix*translationMatrix;

		}

		translationMatrix = translate(0.0f, 0.0f, -3.0f);
		modelViewMatrix = modelViewMatrix*translationMatrix;

		modelViewProjectionMatrix = perspectiveProjectionMatrix*modelViewMatrix;

		glUniformMatrix4fv(mvpUniform,
			1,
			GL_FALSE,
			modelViewProjectionMatrix);
		glBindVertexArray(vao_Line);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);
	}
	
	glUseProgram(0);
	SwapBuffers(ghdc);
}

void update(void)
{
	//triangle
	if (xTranslateTriangle > 0.0f)
	{
		xTranslateTriangle -= 0.001f;
		yTranslateTriangle = 0.0;
	}
	else
	{
		triangleComplete = true;
	}
	//Circle
	if (triangleComplete == true && circleComplete == false)
	{
		if (xTranslateCircle < 0.0f)
		{
			xTranslateCircle += 0.0009f;
			yTranslateCircle += 0.0009f;
		}
		else
		{
			circleComplete = true;
			CircleRotate = 0.0f;
		}
		if (CircleRotate > 360.0f)
		{
			CircleRotate = 0.0f;
		}
		else
		{
			CircleRotate = CircleRotate + 0.1f;
		}
	}
	//Circle
	if (triangleComplete == true)
	{
		if (CircleRotate > 360.0f)
		{
			CircleRotate = 0.0f;
		}
		else
		{
			CircleRotate = CircleRotate + 0.1f;
		}
	}

	//line
	if (circleComplete == true && lineComplete == false)
	{
		if (yTranslateLine > 0.0f)
		{
			xTranslateLine = 0.0f;
			yTranslateLine -= 0.0009f;
		}
		else
		{
			lineComplete = true;
		}
	}
}


