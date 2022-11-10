#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>
#include<math.h>
#include"resource.h"

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"winmm.lib")

//macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//global variables
HDC ghdc = NULL;
HWND ghwnd = NULL;
HGLRC ghrc = NULL;
bool gbActiveWindow = false;
FILE *gpFile = NULL;
void drawIAlphabet();
void drawNAlphabet();
void drawDAlphabet();
void drawAAlphabet();
void DrawPlane();
void update();
void resetTransitions();
void DrawFlagPlaneLines();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool gI1Complete = false;
float xI1Translate = -4.0f;
float yI1Translate = 0.0f;

bool gNComplete = false;
float xNTranslate = -1.3f + 0.5f;
float yNTranslate = 1.5f;

bool gDComplete = false;
float xDBlend = 0.0f;

bool gI2Complete = false;
float xI2Translate = 0.0f;
float yI2Translate = 0.0f;

bool gAComplete = false;
float xATranslate = 2.0f;
float yATranslate = 0.0f;

bool gPlaneComplete = false;
float xPlaneTranslate = -4.0f;
float xPlaneBlend = 1.0f;

float anglem = 3.142f;
float zPlane = 0.1f;

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
	PlaySound(
		MAKEINTRESOURCE(IDR_WAVE1),
		GetModuleHandle(NULL),
		SND_RESOURCE | SND_ASYNC);

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
		case 0x52:
			resetTransitions();
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
	dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
	if (dwStyle&WS_OVERLAPPEDWINDOW)
	{
		mi = { sizeof(MONITORINFO) };
		if (GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ToggleFullScreen();
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
	resetTransitions();
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void resetTransitions()
{
	gI1Complete = false;
	gNComplete = false;
	gDComplete = false;
	gI2Complete = false;
	gAComplete = false;
	gPlaneComplete = false;

	xI1Translate = -4.0f;
	yI1Translate = 0.0f;

	xNTranslate = -1.35f + 0.5f;
	yNTranslate = 1.5f;

	xDBlend = 0.0f;

	xI2Translate = 0.0f;
	yI2Translate = -1.5f;

	xATranslate = 2.0f;
	yATranslate = 0.0f;

	xPlaneTranslate = -3.0f;
	xPlaneBlend = 1.0f;

	anglem = 3.00f;
	zPlane = 0.5f;

	PlaySound(
		MAKEINTRESOURCE(IDR_WAVE1),
		GetModuleHandle(NULL),
		SND_RESOURCE | SND_ASYNC);
}

void update()
{
	//I1 translate
	if (xI1Translate <= -1.4f)
	{
		xI1Translate += 0.001f;
		yI1Translate = 0.0f;
	}
	else
	{
		gI1Complete = true;
	}

	if (gI1Complete == true)
	{
		//A Translate
		if (xATranslate >= -1.26f + 4 * 0.5f)
		{
			xATranslate -= 0.001f;
			yATranslate = 0.0f;
		}
		else
		{
			gAComplete = true;
		}
	}

	if (gAComplete == true)
	{
		//N Translate
		if (yNTranslate >= 0.0f)
		{
			xNTranslate = -1.34f + 0.5f;
			yNTranslate -= 0.001f;
		}
		else
		{
			gNComplete = true;
		}
	}


	if (gNComplete == true)
	{
		//I2 Translate
		if (yI2Translate <= 0.0f)
		{
			xI2Translate = -1.35f + 3 * 0.5f;
			yI2Translate += 0.001f;
		}
		else
		{
			gI2Complete = true;
		}
	}

	if (gI2Complete == true)
	{
		//D fade Translate
		if (xDBlend <= 1.0f)
		{
			xDBlend += 0.001f;
		}
		else
		{
			gDComplete = true;
		}
	}

	if (gDComplete == true)
	{

		if (xPlaneTranslate <= 4.5f)
		{
			xPlaneTranslate += 0.001f;
			if (anglem > 3.142f / 2.0f)
			{
				anglem -= 0.00079f;
				if (zPlane < 1.0f)
					zPlane += 0.0001f;
				else
					zPlane = 1.0f;
			}
		}
		else
		{
			gPlaneComplete = true;
		}

		if (gPlaneComplete)
		{
			if (xPlaneBlend >= 0.0f)
			{
				xPlaneBlend -= 0.0005f;
			}
		}

	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	//Draw I
	glLoadIdentity();
	glTranslatef(xI1Translate, yI1Translate, -4.0f);
	drawIAlphabet();

	if (gAComplete)
	{
		//Draw N
		glLoadIdentity();
		glTranslatef(xNTranslate, yNTranslate, -4.0f);
		drawNAlphabet();
	}

	if (gI2Complete)
	{
		//Draw D
		glLoadIdentity();
		glTranslatef(-1.35f + 2 * 0.5f, 0.0f, -4.0f);
		glColor4f(0.0f, 1.0f, 0.0f, 0.001f);
		drawDAlphabet();
	}

	if (gNComplete)
	{
		//Draw I
		glLoadIdentity();
		glTranslatef(xI2Translate, yI2Translate, -4.0f);
		drawIAlphabet();
	}

	if (gI1Complete)
	{
		//Draw A
		glLoadIdentity();
		glTranslatef(xATranslate, yATranslate, -4.0f);
		drawAAlphabet();
	}

	if (gDComplete)
	{
		//Draw Plane
		glLoadIdentity();
		glTranslatef(xPlaneTranslate, 0.1f, -4.0f);
		DrawPlane();

		glLoadIdentity();
		glTranslatef(xPlaneTranslate, (GLfloat)cos(anglem) + 0.1f, -4.0f);
		//glScalef(zPlane, zPlane, 0.0f);
		DrawPlane();

		glLoadIdentity();
		glTranslatef(xPlaneTranslate, (GLfloat)-cos(anglem) + 0.1f, -4.0f);
		//glScalef(zPlane, zPlane, 0.0f);
		DrawPlane();

		//Draw Lines
		if (xPlaneTranslate >= -1.5f + 0.9f)
		{
			glLoadIdentity();
			glTranslatef(0.0f, 0.1f, -4.0f);
			DrawFlagPlaneLines();
		}

	}

	SwapBuffers(ghdc);
}

void DrawFontLines()
{
	glBegin(GL_QUADS);
		glColor3f(1.0f, 0.60f, 0.2f);
		glVertex2f(0.04f, 0.81f);

		glColor3f(1.0f, 0.60f, 0.2f);
		glVertex2f(-0.04f, 0.81f);

		glColor3f(0.0745f, 0.533f, 0.0313f);
		glVertex2f(-0.04f, -0.81f);

		glColor3f(0.0745f, 0.533f, 0.0313f);
		glVertex2f(0.04f, -0.81f);
	glEnd();
}

void DrawFontThickForDLines()
{
	glBegin(GL_QUADS);
		glColor4f(1.0f, 0.60f, 0.2f, xDBlend);
		glVertex2f(0.05f, 0.73f);

		glColor4f(1.0f, 0.60f, 0.2f, xDBlend);
		glVertex2f(-0.05f, 0.813f);

		glColor4f(0.0745f, 0.533f, 0.0313f, xDBlend);
		glVertex2f(-0.05f, -0.813f);

		glColor4f(0.0745f, 0.533f, 0.0313f, xDBlend);
		glVertex2f(0.05f, -0.73f);
	glEnd();
}

void DrawFontHorizontalLines()
{
	glBegin(GL_QUADS);
		glVertex2f(0.205f, 0.05f);
		glVertex2f(-0.205f, 0.05f);
		glVertex2f(-0.205f, -0.05f);
		glVertex2f(0.205f, -0.05f);
	glEnd();
}

void DrawFontHorizontalForDLines()
{
	glBegin(GL_QUADS);
		glVertex2f(0.12f, 0.05f);
		glVertex2f(-0.13f, 0.05f);
		glVertex2f(-0.13f, -0.05f);
		glVertex2f(0.12f, -0.05f);
	glEnd();
}

void drawIAlphabet()
{
	glTranslatef(0.25f, 0.0f, 0.0f);
	DrawFontLines();

	glColor3f(1.0f, 0.60f, 0.2f);
	glTranslatef(0.0f, 0.76f, 0.0f);
	DrawFontHorizontalLines();

	glColor3f(0.0745f, 0.533f, 0.0313f);
	glTranslatef(0.0f, -0.76f*2.0f, 0.0f);
	DrawFontHorizontalLines();
}
void drawNAlphabet()
{
	glTranslatef(0.05f, 0.0f, 0.0f);
	DrawFontLines();
	glTranslatef(0.26f, 0.0f, 0.0f);
	DrawFontLines();
	glTranslatef(-0.26f / 2.0f, 0.0f, 0.0f);
	glRotatef(9.0f, 0.0f, 0.0f, 1.0f);
	DrawFontLines();
}
void drawDAlphabet()
{
	glTranslatef(0.05f, 0.0f, 0.0f);
	DrawFontThickForDLines();
	glTranslatef(0.33f, 0.0f, 0.0f);
	DrawFontThickForDLines();

	glColor4f(1.0f, 0.60f, 0.2f, xDBlend);
	glTranslatef(-0.33f / 2.0f, 0.76f, 0.0f);
	DrawFontHorizontalForDLines();

	glColor4f(0.0745f, 0.533f, 0.0313f, xDBlend);
	glTranslatef(0.0f, -0.76f*2.0f, 0.0f);
	DrawFontHorizontalForDLines();
}
void drawAAlphabet()
{
	glTranslatef(0.15f, 0.0f, 0.0f);
	glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);
	DrawFontLines();

	glRotatef(10.0f, 0.0f, 0.0f, 1.0f);

	glTranslatef(0.3f, 0.0f, 0.0f);
	glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
	DrawFontLines();
	if (gPlaneComplete)
	{
		glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);
		glTranslatef(-0.4f, -0.2f, 0.0f);

		glColor3f(1.0f, 0.60f, 0.2f);
		glTranslatef((0.5f / 2.0f), 0.1f, 0.0f);
		DrawFontHorizontalLines();

		glColor3f(1.00f, 1.00f, 1.00f);
		glTranslatef(0.0f, -0.1f, 0.0f);
		DrawFontHorizontalLines();

		glColor3f(0.0745f, 0.533f, 0.0313f);
		glTranslatef(0.0f, -0.1f, 0.0f);
		DrawFontHorizontalLines();

		glTranslatef(-0.5f / 2.0f, 0.1f, 0.0f);

	}

}

void DrawPlane()
{
	glColor3f(0.729f, 0.866f, 0.933f);
	glBegin(GL_QUADS);
		glVertex3f(-0.5f, -0.3f, 0.0f);
		glVertex3f(0.0f, -0.3f, 0.0f);
		glVertex3f(0.2f, 0.0f, 0.0f);
		glVertex3f(-0.2f, 0.0f, 0.0f);
	glEnd();

	//Print I On Plane
	glColor3f(0.0f,0.0f,0.0f);
	glLineWidth(2);
	glBegin(GL_LINES);
		glVertex3f(-0.22f, -0.1f, 0.0f);
		glVertex3f(-0.22f, -0.2f, 0.0f);

		glVertex3f(-0.20f, -0.105f, 0.0f);
		glVertex3f(-0.24f, -0.105f, 0.0f);

		glVertex3f(-0.20f, -0.197f, 0.0f);
		glVertex3f(-0.24f, -0.197f, 0.0f);
	glEnd();

	//Print A n Plane
	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(2);
	glBegin(GL_LINES);
		glVertex3f(-0.18f, -0.2f, 0.0f);
		glVertex3f(-0.15f, -0.1f, 0.0f);

		glVertex3f(-0.15f, -0.1f, 0.0f);
		glVertex3f(-0.12f, -0.2f, 0.0f);

		glVertex3f(-0.165f, -0.18f, 0.0f);
		glVertex3f(-0.135f, -0.18f, 0.0f);
	glEnd();

	//Print F On Plane
	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(2);
	glBegin(GL_LINES);
		glVertex3f(-0.10f, -0.1f, 0.0f);
		glVertex3f(-0.10f, -0.2f, 0.0f);

		glVertex3f(-0.10f, -0.1f, 0.0f);
		glVertex3f(-0.05f, -0.1f, 0.0f);

		glVertex3f(-0.10f, -0.15f, 0.0f);
		glVertex3f(-0.05f, -0.15f, 0.0f);
	glEnd();

	glColor4f(0.729f, 0.866f, 0.933f, 0.95f);
	glBegin(GL_TRIANGLES);
		glVertex3f(-0.1f, -0.3f, 0.0f);
		glVertex3f(0.1f, -0.15f, 0.0f);
		glVertex3f(0.25f, -0.45f, 0.0f);
	glEnd();

	glColor4f(0.729f, 0.866f, 0.933f, 1.0f);
	glBegin(GL_TRIANGLES);
		glVertex3f(-0.2f, 0.0f, 0.0f);
		glVertex3f(-0.1f, 0.30f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();

	glColor4f(0.729f, 0.866f, 0.933f, 1.0f);
	glBegin(GL_TRIANGLES);
		glVertex3f(-0.5f, -0.3f, 0.0f);
		glVertex3f(-0.6f, -0.45f, 0.0f);
		glVertex3f(-0.6f, -0.15f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
		glColor4f(0.0745f, 0.533f, 0.0313f, (rand() % 10000) / 10000.0f);
		glVertex3f(-0.6f, -0.45f, 0.0f);

		glColor4f(0.0745f, 0.533f, 0.0313f, (rand() % 10000) / 10000.0f);
		glVertex3f(-0.6f, -0.35f, 0.0f);

		glColor4f(0.0745f, 0.533f, 0.0313f, (rand() % 10000) / 10000.0f);
		glVertex3f(-1.0f, -0.35f, 0.0f);

		glColor4f(0.0745f, 0.533f, 0.0313f, (rand() % 10000) / 10000.0f);
		glVertex3f(-1.0f, -0.45f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
		glColor4f(1.00f, 1.00f, 1.00f, (rand() % 10000) / 10000.0f);
		glVertex3f(-0.6f, -0.35f, 0.0f);

		glColor4f(1.00f, 1.00f, 1.00f, (rand() % 10000) / 10000.0f);
		glVertex3f(-0.6f, -0.25f, 0.0f);

		glColor4f(1.00f, 1.00f, 1.00f, (rand() % 10000) / 10000.0f);
		glVertex3f(-1.0f, -0.25f, 0.0f);

		glColor4f(1.00f, 1.00f, 1.00f, (rand() % 10000) / 10000.0f);
		glVertex3f(-1.0f, -0.35f, 0.0f);
	glEnd();


	glBegin(GL_QUADS);
		glColor4f(1.0f, 0.60f, 0.2f, (rand() % 10000) / 10000.0f);
		glVertex3f(-0.6f, -0.25f, 0.0f);

		glColor4f(1.0f, 0.60f, 0.2f, (rand() % 10000) / 10000.0f);
		glVertex3f(-0.6f, -0.15f, 0.0f);

		glColor4f(1.0f, 0.60f, 0.2f, (rand() % 10000) / 10000.0f);
		glVertex3f(-1.0f, -0.15f, 0.0f);

		glColor4f(1.0f, 0.60f, 0.2f, (rand() % 10000) / 10000.0f);
		glVertex3f(-1.0f, -0.25f, 0.0f);
	glEnd();

}
float flagIncreaseWidth = 0.0f;
void DrawFlagPlaneLines()
{

	flagIncreaseWidth = xPlaneTranslate - 0.9f;
	if (flagIncreaseWidth > 1.5f)
	{
		flagIncreaseWidth = 1.5f;
	}
	glColor4f(0.0745f, 0.533f, 0.0313f, xPlaneBlend);
	glBegin(GL_QUADS);
		glVertex3f(-1.5f, -0.45f, 0.0f);
		glVertex3f(-1.5f, -0.35f, 0.0f);
		glVertex3f(flagIncreaseWidth, -0.35f, 0.0f);
		glVertex3f(flagIncreaseWidth, -0.45f, 0.0f);
	glEnd();

	glColor4f(1.00f, 1.00f, 1.00f, xPlaneBlend);
	glBegin(GL_QUADS);
		glVertex3f(-1.5f, -0.35f, 0.0f);
		glVertex3f(-1.5f, -0.25f, 0.0f);
		glVertex3f(flagIncreaseWidth, -0.25f, 0.0f);
		glVertex3f(flagIncreaseWidth, -0.35f, 0.0f);
	glEnd();

	glColor4f(1.0f, 0.60f, 0.2f, xPlaneBlend);
	glBegin(GL_QUADS);
		glVertex3f(-1.5f, -0.25f, 0.0f);
		glVertex3f(-1.5f, -0.15f, 0.0f);
		glVertex3f(flagIncreaseWidth, -0.15f, 0.0f);
		glVertex3f(flagIncreaseWidth, -0.25f, 0.0f);
	glEnd();
}
