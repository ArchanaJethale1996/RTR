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

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//lights variabless
bool bLight = false;
GLfloat lightAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat lightDiffused[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 0.0f,3.0f,3.0f,1.0f };

GLfloat light_Model_Ambient[] = { 0.2f,0.2f,0.2f,1.0f };
GLfloat light_Model_Local_Viewer[] = { 0.0f };
GLUquadric *quadric[24];

GLfloat angleOfXRotation = 0.0f;
GLfloat angleOfYRotation = 0.0f;
GLfloat angleOfZRotation = 0.0f;
GLint KeyPressed = 0;

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
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffused);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,light_Model_Ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER,light_Model_Local_Viewer);
	glEnable(GL_LIGHT0);
	for (int i= 0; i < 24; i++)
	{
		quadric[i] = gluNewQuadric();
	}
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

	for (int i = 0; i < 24; i++)
	{
		if (quadric[i])
		{
			gluDeleteQuadric(quadric[i]);
			quadric[i] = NULL;
		}
	}
	
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (width <= height)
	{
		glOrtho(0.0f,
			15.5f,
			0,
			(15.5f*((GLfloat)height / (GLfloat)width)),
			-10.0f,
			10.0f);
	}
	else
	{
		glOrtho(0,
			(15.5f*((GLfloat)width / (GLfloat)height)),
			0.0f,
			15.5f,
			-10.0f,
			10.0f);
	}

}

void display(void)
{
	void draw24Sphere(void);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	if (KeyPressed==1)
	{
		glRotatef(angleOfXRotation, 1.0f, 0.0f, 0.0f);

	}

	if (KeyPressed == 2)
	{
		glRotatef(angleOfYRotation, 0.0f, 1.0f, 0.0f);
		
	}

	if (KeyPressed == 3)
	{
		glRotatef(angleOfZRotation, 0.0f, 0.0f, 1.0f);
		
	}
	glLightfv(GL_LIGHT0,GL_POSITION,lightPosition);
	draw24Sphere();
	SwapBuffers(ghdc);
}

void update(void)
{
	if (KeyPressed == 1)
	{
		angleOfXRotation += 1.0f;
		if (angleOfXRotation >= 360.0f)
		{
			angleOfXRotation = 0.0f;
		}
	}
	if (KeyPressed == 2)
	{
		angleOfYRotation += 0.1f;
		if (angleOfYRotation >= 360.0f)
		{
			angleOfYRotation = 0.0f;
		}
	}
	if (KeyPressed == 3)
	{
		angleOfZRotation += 0.1f;
		if (angleOfZRotation >= 360.0f)
		{
			angleOfZRotation = 0.0f;
		}
	}
}
void draw24Sphere()
{
	GLfloat materialAmbient[4] ;
	GLfloat materialDiffused[4] ;
	GLfloat materialSpecular[4] ;
	GLfloat materialShininess[1] ;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//1st sphere 1st col emrald
	materialAmbient[0] = 0.0215f;
	materialAmbient[1] = 0.1745f;
	materialAmbient[2] = 0.0215f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.07568f;
	materialDiffused[1] = 0.61424f;
	materialDiffused[2] = 0.07568f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.633f;
	materialSpecular[1] = 0.727811f;
	materialSpecular[2] = 0.633f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.6f*128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(3.5f, 14.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//2nd sphere on  1st col,jade

	materialAmbient[0] = 0.135f;
	materialAmbient[1] = 0.2225f;
	materialAmbient[2] = 0.1575f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.54f;
	materialDiffused[1] = 0.89f;
	materialDiffused[2] = 0.63f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.316228f;
	materialSpecular[1] = 0.316228f;
	materialSpecular[2] = 0.316228f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.1f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(3.5f, 11.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//3rd sphere on  1st col,obsidian

	materialAmbient[0] = 0.05375f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.06625f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.18275f;
	materialDiffused[1] = 0.17f;
	materialDiffused[2] = 0.22525f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.332741f;
	materialSpecular[1] = 0.328634f;
	materialSpecular[2] = 0.346435f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.3f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(3.5f, 9.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//4th sphere on  1st col,pearl

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

	materialShininess[0] = 0.88f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(3.5f, 6.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//5th sphere on  1st col,ruby

	materialAmbient[0] = 0.1745;
	materialAmbient[1] = 0.01175f;
	materialAmbient[2] = 0.01175f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.61424f;
	materialDiffused[1] = 0.04136f;
	materialDiffused[2] = 0.04136f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.727811f;
	materialSpecular[1] = 0.626959f;
	materialSpecular[2] = 0.626959f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.6f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(3.5f, 4.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//6th sphere on  1st col,turquoise

	materialAmbient[0] = 0.1;
	materialAmbient[1] = 0.18725f;
	materialAmbient[2] = 0.1745f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.396f;
	materialDiffused[1] = 0.074151f;
	materialDiffused[2] = 0.69102f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.297254f;
	materialSpecular[1] = 0.30829f;
	materialSpecular[2] = 0.306678f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.1f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(3.5f, 1.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);



	//2nd Col

	//1st sphere 2nd col brass
	materialAmbient[0] = 0.329412f;
	materialAmbient[1] = 0.223529f;
	materialAmbient[2] = 0.027451;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.780392f;
	materialDiffused[1] = 0.568627f;
	materialDiffused[2] = 0.113725f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.992157f;
	materialSpecular[1] = 0.941176f;
	materialSpecular[2] = 0.807843f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.21794872f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(6.0f, 14.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//2nd sphere on  2nd col,bronze

	materialAmbient[0] = 0.2125f;
	materialAmbient[1] = 0.1275f;
	materialAmbient[2] = 0.054f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.714f;
	materialDiffused[1] = 0.4284f;
	materialDiffused[2] = 0.18144f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.393548f;
	materialSpecular[1] = 0.271906f;
	materialSpecular[2] = 0.166721f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.2f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(6.0f, 11.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//3rd sphere on  2nd col,chrome

	materialAmbient[0] = 0.25f;
	materialAmbient[1] = 0.25f;
	materialAmbient[2] = 0.25f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.4f;
	materialDiffused[1] = 0.4f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.774597f;
	materialSpecular[1] = 0.774597f;
	materialSpecular[2] = 0.774597f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.6f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(6.0f, 9.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//4th sphere on  2nd col,copper

	materialAmbient[0] = 0.19125;
	materialAmbient[1] = 0.0735f;
	materialAmbient[2] = 0.0225f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.7038f;
	materialDiffused[1] = 0.27048f;
	materialDiffused[2] = 0.0828f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.256777f;
	materialSpecular[1] = 0.137622f;
	materialSpecular[2] = 0.086014f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.1f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(6.0f, 6.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//5th sphere on  2nd col,gold

	materialAmbient[0] = 0.2472f;
	materialAmbient[1] = 0.1995f;
	materialAmbient[2] = 0.0745f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.75164f;
	materialDiffused[1] = 0.60648f;
	materialDiffused[2] = 0.22648f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.628281f;
	materialSpecular[1] =0.555802f;
	materialSpecular[2] = 0.366065f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.4f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(6.0f, 4.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//6th sphere on  2nd col,silver

	materialAmbient[0] = 0.19225f;
	materialAmbient[1] = 0.19225f;
	materialAmbient[2] = 0.19225f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.5074f;
	materialDiffused[1] = 0.5074f;
	materialDiffused[2] = 0.5074f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.508273f;
	materialSpecular[1] = 0.508273f;
	materialSpecular[2] = 0.508273f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.4f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(6.0f, 1.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//3rd col
	//1st sphere 3rd col black
	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.0f;
	materialDiffused[1] = 0.0f;
	materialDiffused[2] = 0.0f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.5f;
	materialSpecular[1] = 0.5;
	materialSpecular[2] = 0.5f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.25f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(8.5f, 14.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.1f;
	materialAmbient[2] = 0.06f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.0f;
	materialDiffused[1] = 0.50980392f;
	materialDiffused[2] = 0.5098392f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.50196078f;
	materialSpecular[1] = 0.50196078f;
	materialSpecular[2] = 0.50196078f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.25f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(8.5f, 11.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//3rd sphere on  3rd col,green

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.1f;
	materialDiffused[1] = 0.35f;
	materialDiffused[2] = 0.1f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.45f;
	materialSpecular[1] = 0.55f;
	materialSpecular[2] = 0.45f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.25f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(8.5f, 9.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//4th sphere on  3rd col,red

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.0f;
	materialDiffused[2] = 0.0f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.7f;
	materialSpecular[1] = 0.6f;
	materialSpecular[2] = 0.6f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.25f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(8.5f, 6.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//5th sphere on  3rd col,white

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.55f;
	materialDiffused[1] = 0.55f;
	materialDiffused[2] = 0.55f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.70f;
	materialSpecular[1] = 0.70f;
	materialSpecular[2] = 0.70f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.25f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(8.5f, 4.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.0f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.60f;
	materialSpecular[1] = 0.60f;
	materialSpecular[2] = 0.50f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.25f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(8.5f, 1.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//3rd col
	//1st sphere 3rd col black
	materialAmbient[0] = 0.02f;
	materialAmbient[1] = 0.02f;
	materialAmbient[2] = 0.02f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.01f;
	materialDiffused[1] = 0.01f;
	materialDiffused[2] = 0.01f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.4f;
	materialSpecular[1] = 0.4f;
	materialSpecular[2] = 0.4f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.78125f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(11.0f, 14.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.05f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.4f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.5f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.04f;
	materialSpecular[1] = 0.7f;
	materialSpecular[2] = 0.7f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.078125f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(11.0f, 11.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//3rd sphere on  3rd col,green

	materialAmbient[0] = 0.0f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.4f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.04f;
	materialSpecular[1] = 0.7f;
	materialSpecular[2] = 0.04f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.078125f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(11.0f, 9.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//4th sphere on  3rd col,red

	materialAmbient[0] = 0.05f;
	materialAmbient[1] = 0.0f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.4f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.7f;
	materialSpecular[1] = 0.04f;
	materialSpecular[2] = 0.04f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.078125f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(11.0f, 6.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


	//5th sphere on  3rd col,white

	materialAmbient[0] = 0.05f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.05f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.5f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.70f;
	materialSpecular[1] = 0.70f;
	materialSpecular[2] = 0.70f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.078125f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(11.0f, 4.0f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);

	//6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.05f;
	materialAmbient[1] = 0.05f;
	materialAmbient[2] = 0.0f;
	materialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);

	materialDiffused[0] = 0.5f;
	materialDiffused[1] = 0.5f;
	materialDiffused[2] = 0.4f;
	materialDiffused[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffused);

	materialSpecular[0] = 0.70f;
	materialSpecular[1] = 0.70f;
	materialSpecular[2] = 0.04f;
	materialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	materialShininess[0] = 0.078125f * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(11.0f, 1.5f, 0.0f);
	//glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	gluSphere(quadric[0], 1.0f, 30, 30);


}