#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

//macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//global variables
HDC ghdc = NULL;
HWND ghwnd = NULL;
HGLRC ghrc = NULL;
bool gbActiveWindow = false;
FILE *gpFile = NULL;
GLfloat rotatePyramid = 0.0f;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

struct Light
{
	GLfloat lightAmbient[4] ;
	GLfloat lightDiffused[4];
	GLfloat lightSpecular[4];
	GLfloat lightPosition[4];
};


struct Light lights[2];

//lights variabless
bool bLight = false;


GLfloat materialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat materialDiffused[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialShininess[] = { 128.0f };
GLUquadric *quadric = NULL;

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
		case 'L':
		case 'l':
			if (bLight == false)
			{
				bLight = true;
				glEnable(GL_LIGHTING);
			}
			else
			{
				bLight = false;
				glDisable(GL_LIGHTING);
			}
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
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	lights[0].lightAmbient[0] = 1.0f;
	lights[0].lightAmbient[1] = 0.0f;
	lights[0].lightAmbient[2] = 0.0f;
	lights[0].lightAmbient[3] = 1.0f;

	lights[0].lightDiffused[0] = 1.0f;
	lights[0].lightDiffused[1] = 0.0f;
	lights[0].lightDiffused[2] = 0.0f;
	lights[0].lightDiffused[3] = 1.0f;

	lights[0].lightSpecular[0] = 1.0f;
	lights[0].lightSpecular[1] = 0.0f;
	lights[0].lightSpecular[2] = 0.0f;
	lights[0].lightSpecular[3] = 1.0f;


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
	lights[1].lightDiffused[3] = 1.0f;

	lights[1].lightSpecular[0] = 0.0f;
	lights[1].lightSpecular[1] = 0.0f;
	lights[1].lightSpecular[2] = 1.0f;
	lights[1].lightSpecular[3] = 1.0f;


	lights[1].lightPosition[0] = 2.0f;
	lights[1].lightPosition[1] = 0.0f;
	lights[1].lightPosition[2] = 0.0f;
	lights[1].lightPosition[3] = 1.0f;

	glLightfv(GL_LIGHT0, GL_AMBIENT, lights[0].lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lights[0].lightDiffused);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lights[0].lightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, lights[0].lightPosition);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, lights[1].lightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lights[1].lightDiffused);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lights[1].lightSpecular);
	glLightfv(GL_LIGHT1, GL_POSITION, lights[1].lightPosition);
	glEnable(GL_LIGHT1);

	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialDiffused);
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);
	
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

	if (quadric)
	{
		gluDeleteQuadric(quadric);
		quadric = NULL;
	}
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -5.0f);
	glRotatef(rotatePyramid, 0.0f, 1.0f, 0.0f);
	glBegin(GL_TRIANGLES);
	//front face
	glNormal3f(0.0f, 0.447214f, 0.894427f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	//left face
	glNormal3f(-0.894427f, 0.447214f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	//back face
	glNormal3f(0.0f, 0.447214f, -0.894427f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	//right face
	glNormal3f(0.894427f, 0.447214f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	SwapBuffers(ghdc);
}

void update(void)
{
	rotatePyramid += 0.1f;
	if (rotatePyramid > 360.0f)
	{
		rotatePyramid = 0.0f;
	}
}