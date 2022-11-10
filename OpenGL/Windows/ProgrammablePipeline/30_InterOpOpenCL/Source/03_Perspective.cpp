#include<windows.h>
#include<C:\glew-2.1.0\include\GL\glew.h>
#include<gl/GL.h>

#include<stdio.h>
#include"vmath.h"

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

//cuda includes
#include <CL/opencl.h>

#pragma comment(lib,"C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v10.1\\lib\\x64\\cudaRT.lib")
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
GLuint vao; //vertex array object
GLuint vbo; //vertex buffer object
GLuint vbo_gpu; //vertex buffer object

GLuint mvpUniform;
GLuint colorClearColorUniform;
mat4 perspectiveProjectionMatrix;

const int gMesh_Width = 2048;
const int gMesh_Height = 2048;

#define MyArraySize gMesh_Width*gMesh_Height*4

float pos[gMesh_Width][gMesh_Height][4];
struct clGraphicsResource *graphicsResource=NULL;

// global OpenCL variables
cl_int ret_ocl;
cl_platform_id oclPlatformID;
cl_device_id oclComputeDeviceID; // compute device id
cl_context oclContext; // compute context
cl_command_queue oclCommandQueue; // compute command queue
cl_program oclProgram; // compute program
cl_kernel oclKernel; // compute kernel

char *oclSourceCode=NULL;
size_t sizeKernelCodeLength;

float animationTime=0;
bool bOnGPU = false;

float colorSineWave[4] = { 1.0f,0.0f,0.0f,1.0f};
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
		case 'G':
		case 'g':
			bOnGPU = true;
			break;
		case 'C':
		case 'c':
			bOnGPU = false;
			break;
		case 'Y':
		case 'y':
			colorSineWave[0] = 1.0f;
			colorSineWave[1] = 1.0f;
			colorSineWave[2] = 0.0f;
			colorSineWave[3] = 1.0f;
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
	BOOL LoadTexture(GLuint *texture, TCHAR ImageResourceID[]);
	void resize(int, int);
	void unInitialize(void);
	//variable Declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;
	GLenum result;
	memset((void*)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));

	
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

	////////////////////////////////////
	//OpenCL
	cl_platform_id platformID;
	cl_device_id *devIDs;
	cl_uint devCount;

	ret_ocl=clGetPlatformIDs(1,&platformID,NULL);
    if(ret_ocl != CL_SUCCESS)
    {
        printf("OpenCL Error - clGetDeviceIDs() Failed : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
	
	ret_ocl=clGetDeviceIDs(platformID,CL_DEVICE_TYPE_GPU,0,NULL,&devCount);
    if(ret_ocl != CL_SUCCESS)
    {
        printf("OpenCL Error - clGetDeviceIDs() Failed : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
	else
	{
		devIDs=(cl_device_id*)malloc(sizeof(cl_device_id)*devCount);
		if(devIDs!=NULL)
		{
			ret_ocl=clGetDeviceIDs(platformID,CL_DEVICE_TYPE_GPU,devCount,devIDs,NULL);
			
			if(ret_ocl != CL_SUCCESS)
			{
				printf("OpenCL Error - clGetDeviceIDs() Failed : %d. Exitting Now ...\n",ret_ocl);
				cleanup();
				exit(EXIT_FAILURE);
			}
			oclComputeDeviceID=devIDs[0];
			free(devIDs);
		}
	}
	
	cl_context_properties context_properties[]={
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR , (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM,(cl_context_properties)platformID,
		0
	};
	
	///
	oclContext=clCreateContext(context_properties,1,&oclComputeDeviceID,NULL,NULL,&ret_ocl);
    if(ret_ocl!=CL_SUCCESS)
    {
        printf("OpenCL Error - clCreateContext() Failed : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
	
	 // create command queue
    oclCommandQueue=clCreateCommandQueueWithProperties(oclContext,oclComputeDeviceID,0,&ret_ocl);
    if(ret_ocl!=CL_SUCCESS)
    {
        printf("OpenCL Error - clCreateCommandQueue() Failed : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
	
	//
	// create OpenCL program from .cl
    oclSourceCode=loadOclProgramSource("SineWave.cl","",&sizeKernelCodeLength);
    
	cl_int status=0;
    oclProgram = clCreateProgramWithSource(oclContext, 1, (const char **)&oclSourceCode, &sizeKernelCodeLength, &ret_ocl);
    if(ret_ocl!=CL_SUCCESS)
    {
        printf("OpenCL Error - clCreateProgramWithSource() Failed : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(0);
    }
	
	
	// build OpenCL program
    ret_ocl=clBuildProgram(oclProgram,0,NULL,"-cl-fast-relaxed-math",NULL,NULL);
    if(ret_ocl!=CL_SUCCESS)
    {
        printf("OpenCL Error - clBuildProgram() Failed : %d. Exitting Now ...\n",ret_ocl);
        
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(oclProgram,oclComputeDeviceID,CL_PROGRAM_BUILD_LOG,sizeof(buffer),buffer,&len);
        printf("OpenCL Program Build Log : %s\n",buffer);
        
        cleanup();
        exit(EXIT_FAILURE);
    }
    
	// create OpenCL kernel by passing kernel function name that we used in .cl file
    oclKernel=clCreateKernel(oclProgram,"vecAdd",&ret_ocl);
    if(ret_ocl!=CL_SUCCESS)
    {
        printf("OpenCL Error - clCreateKernel() Failed : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
	
	//VertexShader Object
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//Write Shader source code
	GLchar *vertexShaderSourceCode =
		"#version 420 core" \
		"\n" \
		"in vec4 vPosition;" \
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position=u_mvp_matrix*vPosition;" \
		"}";

	//Specify the above source code to the shader object
	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	//compile the vertex shader source code
	glCompileShader(gVertexShaderObject);

	//Error Checking
	GLint iShaderCompileStatus = 0;
	GLint iInfoLength = 0;
	GLchar *szInfoLog = NULL;

	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLength);
		if (iInfoLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLength);
			if (szInfoLog != NULL) {
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLength, &written, szInfoLog);
				fprintf(gpFile, szInfoLog);
				free(szInfoLog);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}


	//FragmentShader Object
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//Write Shader source code
	GLchar *fragmentShaderSourceCode =
		"#version 420 core" \
		"\n" \
		"out vec4 FragColor;" \
		"uniform vec3 u_colorClearColor;" \
		"void main(void)" \
		"{" \
		"FragColor=vec4(1.0,1.0,0.0,1.0);" \
		"}";

	//Specify the above source code to the shader object
	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	//compile the vertex shader source code
	glCompileShader(gFragmentShaderObject);

	//Error Checking
	iShaderCompileStatus = 0;
	iInfoLength = 0;
	szInfoLog = NULL;

	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLength);
		if (iInfoLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLength);
			if (szInfoLog != NULL) {
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLength, &written, szInfoLog);
				fprintf(gpFile, szInfoLog);
				free(szInfoLog);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}

	//Create a shader program Object
	gShaderProgramObject = glCreateProgram();

	//Attach Vertex Shader to Program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	//Attach Fragment Shader to Program
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	//Prelinking binding Vertex Attributes
	glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");
	
	//Link the program 
	glLinkProgram(gShaderProgramObject);

	//error checking
	GLint iProgramLinkStatus = 0;
	iInfoLength = 0;
	szInfoLog = NULL;

	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);
	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLength);
		if (iInfoLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLength);
			if (szInfoLog != NULL) {
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLength, &written, szInfoLog);
				fprintf(gpFile, szInfoLog);
				free(szInfoLog);
				unInitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}

	//post linking
	mvpUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
	colorClearColorUniform= glGetUniformLocation(gShaderProgramObject, "u_colorClearColor");
	//cleaning pos [gMesh_Width][gMesh_Height][4];
	for (int i = 0; i <gMesh_Width; i++)
	{
		for(int j=0;j<gMesh_Height;j++)
		{ 
			for (int k = 0; k <4 ; k++)
			{
				pos[i][j][k] = 0.0f;
			}
		}
	}


	//create vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
		MyArraySize * sizeof(float),
		NULL,
		GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glGenBuffers(1, &vbo_gpu);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_gpu);
	glBufferData(GL_ARRAY_BUFFER,
		MyArraySize * sizeof(float),
		NULL,
		GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//register Our VBO with CUDA graphic Resource
	graphicsResource=clCreateFromGLBuffer(oclContext//Cl context
	, CL_MEM_WRITE_ONLY,vbo_gpu,&ret_ocl );	//Write only
	 if(ret_ocl!=CL_SUCCESS)
    {
        printf("OpenCL Error - clCreateBuffer() Failed For 2nd Input Array : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
	
	glBindVertexArray(0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	perspectiveProjectionMatrix = mat4::identity();
	glPointSize(3.0);
	resize(WIN_WIDTH, WIN_HEIGHT);
	return 0;
}

void unInitialize(void)
{
	if(graphicsResource)
	{
	clReleaseMemObject(graphicsResource);
	graphicsResource=NULL;
	}
	if(oclSourceCode)
    {
        free((void *)oclSourceCode);
        oclSourceCode=NULL;
    }
    
    if(oclKernel)
    {
        clReleaseKernel(oclKernel);
        oclKernel=NULL;
    }
    
    if(oclProgram)
    {
        clReleaseProgram(oclProgram);
        oclProgram=NULL;
    }
    
    if(oclCommandQueue)
    {
        clReleaseCommandQueue(oclCommandQueue);
        oclCommandQueue=NULL;
    }
    
    if(oclContext)
    {
        clReleaseContext(oclContext);
        oclContext=NULL;
    }
	if (vbo)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}

	if (vbo_gpu)
	{
		glDeleteBuffers(1, &vbo_gpu);
		vbo_gpu = 0;
	}
	if (vao)
	{
		glDeleteBuffers(1, &vao);
		vbo = 0;
	}

	GLsizei shaderCount;
	GLsizei shaderNumber;
	if (gShaderProgramObject)
	{
		glUseProgram(gShaderProgramObject);

		glGetProgramiv(gShaderProgramObject,GL_ATTACHED_SHADERS,&shaderCount);
		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint)* shaderCount);
		if (pShaders)
		{
			glGetAttachedShaders(gShaderProgramObject,shaderCount,&shaderCount,pShaders);
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
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	perspectiveProjectionMatrix = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
}

void LaunchCPUKernel(unsigned int meshWidth,unsigned int meshHeight, float time)
{
	for (unsigned int i = 0; i < meshWidth; i++)
	{
		for (unsigned int j = 0; j < meshHeight; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				float u = i / (float)meshWidth;
				float v = j / (float)meshHeight;
				u = u*2.0f -1.0f;
				v = v*2.0f -1.0f;

				float frequency = 4.0f;
				float w =(float) sin(frequency*u + animationTime)*(float)cos(frequency*v + animationTime)*0.5f;
				if (k == 0)
				{
					pos[i][j][k] = u;
				}
				if (k == 1)
				{
					pos[i][j][k] = w;
				}
				if (k == 2)
				{
					pos[i][j][k] = v;
				}
				if (k == 3)
				{
					pos[i][j][k] = 1.0;
				}
			}
		}
	}
}

void display(void)
{

	void LaunchCudaKernal(float4 *pos, unsigned int meshWidth, unsigned int meshHeight, float time);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gShaderProgramObject);
	
	//Declaration of matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 translationMatrix;
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix= mat4::identity();
	//Do necessary transformation
	//translationMatrix = translate(0.0f, 0.0f, -3.0f);
	//modelViewMatrix = modelViewMatrix*translationMatrix;
	//Do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix*modelViewMatrix;

	//send necessary matrices to shaders
	glUniformMatrix4fv(mvpUniform,
		1, GL_FALSE,
		modelViewProjectionMatrix);

	glUniform4f(colorClearColorUniform,
		(GLfloat)colorSineWave[0],
		(GLfloat)colorSineWave[1],
		(GLfloat)colorSineWave[2],
		(GLfloat)colorSineWave[3]);
	//Bind with vao
	glBindVertexArray(vao);

	if (bOnGPU == true)
	{
		ret_ocl=clSetKernelArg(oclKernel,0,sizeof(cl_mem),(void *)&graphicsResource); // 'deviceInput1' maps to 'in1' param of kernel function in .cl file
    if(ret_ocl != CL_SUCCESS)
    {
        printf("OpenCL Error - clSetKernelArg() Failed For 1st Argument : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
    
    // set 0 based 1st argument i.e. deviceInput2
    ret_ocl=clSetKernelArg(oclKernel,1,sizeof(cl_mem),(void *)&meshWidth); // 'deviceInput2' maps to 'in2' param of kernel function in .cl file
    if(ret_ocl != CL_SUCCESS)
    {
        printf("OpenCL Error - clSetKernelArg() Failed For 2nd Argument : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
    
    // set 0 based 2nd argument i.e. deviceOutput
    ret_ocl=clSetKernelArg(oclKernel,2,sizeof(cl_mem),(void *)&meshHeight); // 'deviceOutput' maps to 'out' param of kernel function in .cl file
    if(ret_ocl != CL_SUCCESS)
    {
        printf("OpenCL Error - clSetKernelArg() Failed For 3rd Argument : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
    
    // set 0 based 3rd argument i.e. len
    ret_ocl=clSetKernelArg(oclKernel,3,sizeof(cl_int),(void *)&animationTime); // 'iNumberOfArrayElements' maps to 'len' param of kernel function in .cl file
    if(ret_ocl != CL_SUCCESS)
    {
        printf("OpenCL Error - clSetKernelArg() Failed For 4th Argument : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
	
	//////////////
	ret_ocl=clEnqueueAcquireGLObjects(oclCommandQueue,1,&graphicsResource,0,NULL,NULL);
	if(ret_ocl != CL_SUCCESS)
    {
        printf("OpenCL Error - clEnqueueAcquireGLObjects() Failed For 1st Input Device Buffer : %d. Exitting Now ...\n",ret_ocl);
        cleanup();
        exit(EXIT_FAILURE);
    }
	
	//Run Kernel
	size_t globalWorkSize[2];
	globalWorkSize[0]=meshWidth;
	globalWorkSize[1]=meshHeight;

	ret_ocl=clEnqueueNDRangeKernel(oclCommandQueue,oclKernel,2,NULL,globalWorkSize,0,0,NULL,NULL);
	
	clEnqueueReleaseGLObject(oclCommandQueue,oclKernel,1,NULL,globalWorkSize,0,0,NULL,NULL);
	
	clFinish(oclCommandQueue);
	}
	else
	{
		LaunchCPUKernel(gMesh_Width, gMesh_Height, animationTime);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER,
			MyArraySize*sizeof(float),
			pos,
			GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	

	if (bOnGPU)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo_gpu);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	//Bind with textures

	//Draw necessary scene
	glDrawArrays(GL_POINTS, 0, gMesh_Width*gMesh_Height*4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Unbind vao
	glBindVertexArray(0);
	glUseProgram(0);
	SwapBuffers(ghdc);
	animationTime = animationTime + 1.0f;
}

void update(void)
{
}

char* loadOclProgramSource(const char *filename, const char *preamble, size_t *sizeFinalLength)
{
    // locals
    FILE *pFile=NULL;
    size_t sizeSourceLength;
    
    pFile=fopen(filename,"rb"); // binary read
    if(pFile==NULL)
        return(NULL);
    
    size_t sizePreambleLength=(size_t)strlen(preamble);
    
    // get the length of the source code
    fseek(pFile,0,SEEK_END);
    sizeSourceLength=ftell(pFile);
    fseek(pFile,0,SEEK_SET); // reset to beginning
    
    // allocate a buffer for the source code string and read it in
    char *sourceString=(char *)malloc(sizeSourceLength+sizePreambleLength+1);
    memcpy(sourceString, preamble, sizePreambleLength);
    if(fread((sourceString)+sizePreambleLength,sizeSourceLength,1,pFile)!=1)
    {
        fclose(pFile);
        free(sourceString);
        return(0);
    }
    
    // close the file and return the total length of the combined (preamble + source) string
    fclose(pFile);
    if(sizeFinalLength != 0)
    {
        *sizeFinalLength = sizeSourceLength + sizePreambleLength;
    }
    sourceString[sizeSourceLength + sizePreambleLength]='\0';
    
    return(sourceString);
}


//cl.exe /I
