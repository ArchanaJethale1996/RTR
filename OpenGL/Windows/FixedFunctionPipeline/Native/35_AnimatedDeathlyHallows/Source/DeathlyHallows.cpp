#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>
#include<math.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

#define PI 3.14169
#define NUM_POINTS 1000
//macros
#define WIN_WIDTH 600
#define WIN_HEIGHT 600

//global variables
HDC ghdc = NULL;
HWND ghwnd = NULL;
HGLRC ghrc = NULL;
bool gbActiveWindow = false;
FILE *gpFile = NULL;
GLfloat CircleRotate=0.0f;
bool triangleComplete = false;
bool circleComplete=false;
bool lineComplete=false;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawTriangle();
void DrawCircle(GLfloat);
void DrawLine(GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2);

float xTranslateLine = 0.0f, yTranslateLine = 1.0f;
float xTranslateTriangle = 2.0f, yTranslateTriangle = 0.0f;
float xTranslateCircle = -2.0f, yTranslateCircle = -1.0f;

//WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmddLine, int iCmdShow)
{
	//functions Declaration
	int initialize(void);
	void display(void);

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
				if (triangleComplete==true && circleComplete==false)
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
				if (circleComplete==true && lineComplete==false)
				{
					if (yTranslateLine > 0.0f)
					{
						xTranslateLine = 0.0f;
						yTranslateLine -= 0.0009;
					}
					else
					{
						lineComplete = true;
					}
				}
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
	//variable Declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;
}

void unInitialize(void)
{
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
	xTranslateLine = 0.0f; yTranslateLine = 1.0f;
	xTranslateTriangle = 2.0f; yTranslateTriangle = 2.0f;
	xTranslateCircle = -2.0f; yTranslateCircle = -1.0f;
	triangleComplete = false;
	circleComplete = false;
	lineComplete = false;
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);	

	glLoadIdentity();
	if (triangleComplete == false)
	{
		glTranslatef(xTranslateTriangle, yTranslateTriangle, 0.0f);
	}
	glTranslatef(0.0f, 0.0f, -3.0f);
	glLineWidth(15.0f);
	DrawTriangle();	

	if (triangleComplete)
	{
		glLoadIdentity();
		if (circleComplete == false)
		{
			glTranslatef(xTranslateCircle, xTranslateCircle, 0.0f);
		}
		glTranslatef(0.0f, 0.31f - 0.5f, -3.0f);
		glRotatef(CircleRotate, 0.0f, 1.0f, 0.0f);
		glLineWidth(12.0f);
		DrawCircle(0.31f);
	}

	if (circleComplete)
	{
		glLoadIdentity();
		glLineWidth(10.0f);
		if (lineComplete == false)
		{
			glTranslatef(xTranslateLine, yTranslateLine, 0.0f);
		}
		glTranslatef(0.0f, 0.0f, -3.0f);
		DrawLine(0.0f, 0.5f, 0.0f, -0.5f);
	}

	SwapBuffers(ghdc); 
}

void DrawTriangle()
{
	glBegin(GL_LINE_LOOP);
		glVertex3f(0.0f, 0.5f, 0.0f);
		glVertex3f(-0.5f, -0.5f, 0.0f);
		glVertex3f(0.5f, -0.5f, 0.0f);
	glEnd();
}

void DrawCircle(GLfloat radius)
{
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i <= NUM_POINTS; i++)
	{
		double angle = (2 * PI * i) / NUM_POINTS;
		glVertex3f((GLfloat)radius*cos(angle), (GLfloat)radius*sin(angle), 0.0f);
	}
	glEnd();
}

void DrawLine(GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2)
{
	if (lineComplete)
	{
		glBegin(GL_LINES);
		glVertex3f(x1, x2, 0.0f);
		glVertex3f(y1, y2, 0.0f);
		glEnd();
	}
	else {
		glBegin(GL_LINES);
		glVertex3f(x1, x2, 0.0f);
		glVertex3f(y1, y2, 0.0f);
		glEnd();
	}
}