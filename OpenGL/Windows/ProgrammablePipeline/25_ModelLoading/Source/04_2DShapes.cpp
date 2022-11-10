#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h> 

#include <gl\glew.h>
#include <gl\gl.h> 

#include "vmath.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")

#define WIN_WIDTH 	800 
#define WIN_HEIGHT 	600 

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

FILE *gpFile = NULL;

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullScreen = false;

enum
{
	AMC_ATTRIBUTE_VERTEX = 0,
};

GLuint gVertexShaderObject, gFragmentShaderObject, gProgramObject;
GLuint gVao, gVbo, gElementBuffer;

GLuint gMVPUniform;

vmath::mat4 gPerspectiveProjectionMatrix;

struct vec_int
{
	int *p;
	int size;
};

struct vec_float
{
	float *pf;
	int size;
};

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];

FILE *gp_mesh_file;
struct vec_float *gp_vertex, *gp_texture, *gp_normal;
struct vec_float *gp_vertex_sorted, *gp_texture_sorted, *gp_normal_sorted;
struct vec_int *gp_vertex_indices, *gp_texture_indices, *gp_normal_indices;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	void initialize(void);
	void uninitialize(void);
	void display(void);

	TCHAR szClassName[] = TEXT("Yogeshwar Shukla : Programmable Pipeline Perspective Triangle");
	HWND hwnd = NULL;
	WNDCLASSEX wndex;
	MSG msg;
	bool bDone = false;

	ZeroMemory(&wndex, sizeof(WNDCLASSEX));
	ZeroMemory(&msg, sizeof(MSG));

	if (fopen_s(&gpFile, "PP_OBJ_PARSER.TXT", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log File Cannot Be Created"), TEXT("fopen_s"),
			MB_ICONERROR | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file has been successfully created\n");
	}

	wndex.cbSize = sizeof(WNDCLASSEX);
	wndex.cbClsExtra = 0;
	wndex.cbWndExtra = 0;
	wndex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndex.hInstance = hInstance;
	wndex.lpszClassName = szClassName;
	wndex.lpfnWndProc = WndProc;
	wndex.lpszMenuName = NULL;
	wndex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	RegisterClassEx(&wndex);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szClassName, szClassName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		100, 100, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);

	ghwnd = hwnd;

	ShowWindow(hwnd, nShowCmd);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	initialize();

	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			display();

			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyIsPressed == true)
					bDone = true;
			}
		}
	}

	uninitialize();

	return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	void resize(int, int);
	void ToggleFullScreen(void);
	void uninitialize();

	switch (uMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;

	case WM_ERASEBKGND:
		return (0);

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			gbEscapeKeyIsPressed = true;
			break;

		case 0x46:
			if (gbFullScreen == false)
			{
				ToggleFullScreen();
				gbFullScreen = true;
			}
			else
			{
				ToggleFullScreen();
				gbFullScreen = false;
			}
			break;

		default:
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		break;

	case WM_CLOSE:
		uninitialize();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
	MONITORINFO mi;

	if (gbFullScreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle && WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) &&
				GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}

		ShowCursor(FALSE);
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}

void initialize(void)
{
	void uninitialize(void);
	void resize(int, int);
	void load_mesh(void);

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == false)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (wglMakeCurrent(ghdc, ghrc) == false)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	// Common compilation and linking variables 
	GLint iShaderCompilationStatus = -1;
	GLint iProgramLinkStatus = -1;
	GLint iInfoLogLength = 0;
	GLint iWritten = 0;
	GLchar *szInfoLog = NULL;

	/* Vertex Shader Block */
	static const GLchar *szvecVertexShaderSource1[] =
	{
		"#version 420 core                          \n"
		"                                           \n"
		"in vec4 vPosition;                         \n"
		"uniform mat4 u_mvp_matrix;                 \n"
		"void main(void)                            \n"
		"{                                          \n"
		"   gl_Position = u_mvp_matrix * vPosition; \n"
		"}                                          \n"
	};

	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(gVertexShaderObject, 1, szvecVertexShaderSource1, NULL);
	glCompileShader(gVertexShaderObject);

	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompilationStatus);
	if (iShaderCompilationStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (!szInfoLog)
			{
				fprintf(gpFile, "malloc:Out of memory\n");
				uninitialize();
				exit(EXIT_FAILURE);
			}
			memset(szInfoLog, 0, iInfoLogLength);
			glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &iWritten, szInfoLog);
			fprintf(gpFile, "Vertex Shader : %s", szInfoLog);
			free(szInfoLog);
			uninitialize();
			exit(EXIT_FAILURE);
		}
	}

	/* Fragment Shader Block */
	static const GLchar *szvecFragmentShaderSource1[] =
	{
		"#version 420 core                          \n"
		"                                           \n"
		"out vec4 FragColor;                        \n"
		"void main(void)                            \n"
		"{                                          \n"
		"   FragColor = vec4(1.0, 1.0, 1.0, 1.0f);  \n"
		"}                                          \n"
	};

	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(gFragmentShaderObject, 1, szvecFragmentShaderSource1, NULL);
	glCompileShader(gFragmentShaderObject);

	iShaderCompilationStatus = 0;

	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompilationStatus);
	if (iShaderCompilationStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (!szInfoLog)
			{
				fprintf(gpFile, "malloc:out of virtual memory\n");
				uninitialize();
				exit(EXIT_FAILURE);
			}

			memset(szInfoLog, 0, iInfoLogLength);
			glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &iWritten, szInfoLog);
			fprintf(gpFile, "Fragment Shader : %s", szInfoLog);
			free(szInfoLog);
			uninitialize();
			exit(EXIT_FAILURE);
		}
	}

	gProgramObject = glCreateProgram();
	glAttachShader(gProgramObject, gVertexShaderObject);
	glAttachShader(gProgramObject, gFragmentShaderObject);
	glBindAttribLocation(gProgramObject, AMC_ATTRIBUTE_VERTEX, "vPosition");
	glLinkProgram(gProgramObject);

	glGetProgramiv(gProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);
	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (!szInfoLog)
			{
				fprintf(gpFile, "malloc:out of virtual memory\n");
				uninitialize();
				exit(EXIT_FAILURE);
			}

			glGetProgramInfoLog(gProgramObject, iInfoLogLength, &iWritten, szInfoLog);
			fprintf(gpFile, "Program Info Log :%s", szInfoLog);
			uninitialize();
			exit(EXIT_FAILURE);
		}
	}

	gMVPUniform = glGetUniformLocation(gProgramObject, "u_mvp_matrix");

	load_mesh();

	glGenVertexArrays(1, &gVao);
	glBindVertexArray(gVao); /*BIND VAO*/

	glGenBuffers(1, &gVbo); /* BIND VBO */
	glBindBuffer(GL_ARRAY_BUFFER, gVbo);
	glBufferData(GL_ARRAY_BUFFER, (gp_vertex_sorted->size) * sizeof(GLfloat), gp_vertex_sorted->pf, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0); /*UNBIND VBO */

	glGenBuffers(1, &gElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, gp_vertex_indices->size * sizeof(int), gp_vertex_indices->p,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0); /*UNBIND VAO */

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	gPerspectiveProjectionMatrix = vmath::mat4::identity();

	resize(WIN_WIDTH, WIN_HEIGHT);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gProgramObject);

	vmath::mat4 modelViewMatrix = vmath::mat4::identity();
	vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();

	modelViewMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	glBindVertexArray(gVao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
	glDrawElements(GL_TRIANGLES, (gp_vertex_indices->size), GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void resize(int width, int height)
{
	//code
	if (width == 0)
		width = 1;
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	gPerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1, 100.0);
}

void uninitialize(void)
{
	int destroy_vec_int(struct vec_int *p_vec_int);
	int destroy_vec_float(struct vec_float *p_vec_float);

	if (gbFullScreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}

	if (gProgramObject && gVertexShaderObject)
	{
		glDetachShader(gProgramObject, gVertexShaderObject);
		glDeleteShader(gVertexShaderObject);
		gVertexShaderObject = 0;
	}

	if (gFragmentShaderObject)
	{
		glDetachShader(gProgramObject, gFragmentShaderObject);
		glDeleteShader(gFragmentShaderObject);
		gFragmentShaderObject = 0;
	}

	glDeleteProgram(gProgramObject);
	gProgramObject = 0;
	glUseProgram(0);

	glDeleteBuffers(1, &gVbo);
	gVbo = 0;

	glDeleteBuffers(1, &gElementBuffer);
	gElementBuffer = 0;

	glDeleteVertexArrays(1, &gVao);
	gVao = 0;

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(ghrc);
	ghrc = NULL;
	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;

	destroy_vec_float(gp_vertex);
	gp_vertex = NULL;

	destroy_vec_float(gp_texture);
	gp_texture = NULL;

	destroy_vec_float(gp_normal);
	gp_normal = NULL;

	destroy_vec_float(gp_vertex_sorted);
	gp_vertex_sorted = NULL;
	destroy_vec_float(gp_texture_sorted);
	gp_texture_sorted = NULL;
	destroy_vec_float(gp_normal_sorted);
	gp_normal_sorted = NULL;

	destroy_vec_int(gp_vertex_indices);
	gp_vertex_indices = NULL;
	destroy_vec_int(gp_texture_indices);
	gp_texture_indices = NULL;
	destroy_vec_int(gp_normal_indices);
	gp_normal_indices = NULL;

	if (gpFile)
	{
		fprintf(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

void load_mesh(void)
{
	struct vec_int *create_vec_int();
	struct vec_float *create_vec_float();
	int push_back_vec_int(struct vec_int *p_vec_int, int data);
	int push_back_vec_float(struct vec_float *p_vec_int, float data);
	void show_vec_float(struct vec_float *p_vec_float);
	void show_vec_int(struct vec_int *p_vec_int);
	int destroy_vec_float(struct vec_float *p_vec_float);


	char *space = " ", *slash = "/", *first_token = NULL, *token;
	char *f_entries[3] = { NULL, NULL, NULL };
	int nr_pos_cords = 0, nr_tex_cords = 0, nr_normal_cords = 0, nr_faces = 0;
	int i, vi;

	gp_mesh_file = fopen("MonkeyHead.OBJ", "r");
	if (gp_mesh_file == NULL)
	{
		fprintf(stderr, "error in opening file\n");
		exit(EXIT_FAILURE);
	}

	gp_vertex = create_vec_float();
	gp_texture = create_vec_float();
	gp_normal = create_vec_float();

	gp_vertex_indices = create_vec_int();
	gp_texture_indices = create_vec_int();
	gp_normal_indices = create_vec_int();

	while (fgets(buffer, BUFFER_SIZE, gp_mesh_file) != NULL)
	{
		first_token = strtok(buffer, space);

		if (strcmp(first_token, "v") == 0)
		{
			nr_pos_cords++;
			while ((token = strtok(NULL, space)) != NULL)
				push_back_vec_float(gp_vertex, atof(token));

		}
		else if (strcmp(first_token, "vt") == 0)
		{
			nr_tex_cords++;
			while ((token = strtok(NULL, space)) != NULL)
				push_back_vec_float(gp_texture, atof(token));
		}
		else if (strcmp(first_token, "vn") == 0)
		{
			nr_normal_cords++;
			while ((token = strtok(NULL, space)) != NULL)
				push_back_vec_float(gp_normal, atof(token));
		}
		else if (strcmp(first_token, "f") == 0)
		{
			nr_faces++;
			for (i = 0; i < 3; i++)
				f_entries[i] = strtok(NULL, space);

			for (i = 0; i < 3; i++)
			{
				token = strtok(f_entries[i], slash);
				push_back_vec_int(gp_vertex_indices, atoi(token) - 1);
				token = strtok(NULL, slash);
				push_back_vec_int(gp_texture_indices, atoi(token) - 1);
				token = strtok(NULL, slash);
				push_back_vec_int(gp_normal_indices, atoi(token) - 1);
			}
		}
	}

	gp_vertex_sorted = create_vec_float();
	for (int i = 0; i < gp_vertex_indices->size; i++)
		push_back_vec_float(gp_vertex_sorted, gp_vertex->pf[i]);

	gp_texture_sorted = create_vec_float();
	for (int i = 0; i < gp_texture_indices->size; i++)
		push_back_vec_float(gp_texture_sorted, gp_texture->pf[i]);

	gp_normal_sorted = create_vec_float();
	for (int i = 0; i < gp_normal_indices->size; i++)
		push_back_vec_float(gp_normal_sorted, gp_normal->pf[i]);


	fclose(gp_mesh_file);
	gp_mesh_file = NULL;


	
}

struct vec_int *create_vec_int()
{
	struct vec_int *p = (struct vec_int*)malloc(sizeof(struct vec_int));
	memset(p, 0, sizeof(struct vec_int));
	return p;
}

struct vec_float *create_vec_float()
{
	struct vec_float *p = (struct vec_float*)malloc(sizeof(struct vec_float));
	memset(p, 0, sizeof(struct vec_float));
	return p;
}

int push_back_vec_int(struct vec_int *p_vec_int, int data)
{
	p_vec_int->p = (int*)realloc(p_vec_int->p, (p_vec_int->size + 1) * sizeof(int));
	p_vec_int->size = p_vec_int->size + 1;
	p_vec_int->p[p_vec_int->size - 1] = data;
	return (0);
}

int push_back_vec_float(struct vec_float *p_vec_float, float data)
{
	p_vec_float->pf = (float*)realloc(p_vec_float->pf, (p_vec_float->size + 1) * sizeof(float));
	p_vec_float->size = p_vec_float->size + 1;
	p_vec_float->pf[p_vec_float->size - 1] = data;
	return (0);
}

int destroy_vec_int(struct vec_int *p_vec_int)
{
	free(p_vec_int->p);
	free(p_vec_int);
	return (0);
}

int destroy_vec_float(struct vec_float *p_vec_float)
{
	free(p_vec_float->pf);
	free(p_vec_float);
	return (0);
}

void show_vec_float(struct vec_float *p_vec_float)
{
	int i;
	for (i = 0; i < p_vec_float->size; i++)
		fprintf(gpFile, "%f\n", p_vec_float->pf[i]);
}

void show_vec_int(struct vec_int *p_vec_int)
{
	int i;
	for (i = 0; i < p_vec_int->size; i++)
		fprintf(gpFile, "%d\n", p_vec_int->p[i]);
}

