#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>			//smilar to windows.h
#include<X11/Xutil.h>			//XVisualInfo
#include<X11/XKBlib.h>		//keyboard utilization header
#include<X11/keysym.h>		//key symbol

//openGL
#include<GL/glew.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include"vmath.h"
using namespace std;
using namespace vmath;

bool bFullScreen=false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;

Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;
GLXContext gGLXContext;
typedef GLXContext (*GLXCreateContextAttribsARBProc)(Display *,GLXFBConfig,GLXContext,Bool,const int *);
GLXCreateContextAttribsARBProc gGLXCreateContextAttribsARB=NULL;
GLXFBConfig gGlXFBConfig;
GLXContext gGlXContext;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint vao;
GLuint vbo_Position;
GLuint vbo_Element;
GLuint mvUniform,pUniform, colorUniform;
mat4 perspectiveProjectionMatrix;

GLuint samplerUniform;

unsigned int pointElements[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

unsigned int pointElements1[] = { 0,3,4,7,8,11,0,12,1,4,8,2,12,3,13,7,14,11,13,1,14,2};

unsigned int pointElements2[] = { 0,3,4,7,8,11,12,15,0,12,1,13,2,14,3,15 };

unsigned int pointElements3[] = { 0,3,4,7,8,11,12,15,0,12,1,13,2,14,3,15,4,1,8,2,12,3,13,7,14,11};

unsigned int pointElements4[] = { 0,3,3,15,15,12,12,0,0,13,0,14,0,15,0,11,0,7 };
unsigned int pointElements5_1[] = { 0,1,4,4,5,1,  8,4,5,5,9,8,  12,8,9,12,9,13 };
unsigned int pointElements5_2[] = { 1,2,5,5,6,2,  9,5,6,6,10,9,  13,9,10,13,10,14 };
unsigned int pointElements5_3[] = { 2,3,6,6,7,3,10,6,7,7,11,10,14,10,11,11,15,14 };

enum
{
	AMC_ATTRIBUTE_POSITION=0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0
};


int main(void)
{
	//function prototyppe	
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void unInitialize();
	void Initialize(void);
	void resize(int,int);
	void display(void);	
	void update(void);
	//variable declaration
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;
	bool bDone=false;	

	//Code
	CreateWindow();
	Initialize();

	//Message Loop
	XEvent event;
	KeySym keysym;

	while(bDone==false)
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay,&event);
			switch(event.type)
			{
				case MapNotify:
					break;
				case KeyPress:
					keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
					switch(keysym)
					{
						case XK_Escape:
							bDone=true;
							unInitialize();
							exit(0);
						case XK_F:
						case XK_f:
							if(bFullScreen==false)
							{
								ToggleFullscreen();
								bFullScreen=true;
							}
							else
							{
								ToggleFullscreen();
								bFullScreen=false;
							}
							break;
						default:
							break;				
					}
					break;
				case ButtonPress:
					switch(event.xbutton.button)
					{
						case 1:
							break;
						case 2:
							break;
						case 3:
							break;
						default:
							break;
					}
					break;
				case MotionNotify:
					break;
				case ConfigureNotify:
					winWidth=event.xconfigure.width;
					winHeight=event.xconfigure.height;
					resize(winWidth,winHeight);
					break;
				case Expose:
					break;
				case 33:
					bDone=true;
					unInitialize();
					exit(0);
				default:
					break;
			}		
			
		}
		update();

		display();
	}
	unInitialize();
	return(0);
}
void CreateWindow(){

	//function prototype
	void unInitialize();

	int styleMask;
	XSetWindowAttributes winAttribs;


	GLXFBConfig *pGLXFBConfigs=NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo=NULL;
	int iNumberOfFBConfigs=0;
	
	static int frameBufferAttribs[]={GLX_X_RENDERABLE,True,
				GLX_DRAWABLE_TYPE,GLX_WINDOW_BIT,
				GLX_RENDER_TYPE,GLX_RGBA_BIT,
				GLX_X_VISUAL_TYPE,GLX_TRUE_COLOR,
				GLX_RED_SIZE,8,
				GLX_GREEN_SIZE,8,
				GLX_BLUE_SIZE,8,
				GLX_ALPHA_SIZE,8,
				GLX_DEPTH_SIZE,24,
				GLX_STENCIL_SIZE,8,
				GLX_DOUBLEBUFFER,True,				
				None};
	//code
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
			printf("ERROR: Uable to Open X Display");
			unInitialize();
			exit(1);
	}

	int defaultScreen=XDefaultScreen(gpDisplay);

	//Retrive All FBConfig that the driver has
	pGLXFBConfigs=glXChooseFBConfig(gpDisplay,defaultScreen,frameBufferAttribs,&iNumberOfFBConfigs);
	printf("There are %d FBConfigs",iNumberOfFBConfigs);

	int bestFrameBufferConfig=-1;
	int bestNumberOfSamples=-1;
	int worstFrameBufferConfig=-1;
	int worstNumberOfSampes=999;

	for(int i=0;i<iNumberOfFBConfigs;i++)
	{
		//for each obtained FBConfig getTempVisual
		pTempXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,pGLXFBConfigs[i]);			
		if(pTempXVisualInfo)
		{
			//Get Number of samples buffers from rspective fb COnfig
			int sampleBuffers,samples;
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLE_BUFFERS,&sampleBuffers);
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLES,&samples);

			//calculae its best
			if(bestFrameBufferConfig<0 || sampleBuffers&&sampleBuffers>bestNumberOfSamples)
			{
				bestFrameBufferConfig=i;
				bestNumberOfSamples=samples;			
			}

			//calculae its worst
			if(worstFrameBufferConfig<0 || sampleBuffers&&sampleBuffers<worstNumberOfSampes)
			{
				worstFrameBufferConfig=i;
				worstNumberOfSampes=samples;			
			}
						
		}
		XFree(pTempXVisualInfo);
	}
	//Now Assign Found Best One
	bestGLXFBConfig=pGLXFBConfigs[bestFrameBufferConfig];

	//Now Assign this to global one
	gGlXFBConfig=bestGLXFBConfig;
	
	XFree(pGLXFBConfigs);
	gpXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,bestGLXFBConfig);
	if(gpXVisualInfo==NULL)
	{
		printf("ERROR: glXGetVisualFromFBConfig failed");
		unInitialize();
		exit(1);
	}	
	
	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),gpXVisualInfo->visual,AllocNone);
	gColormap=winAttribs.colormap;
	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);

	winAttribs.event_mask=ExposureMask|VisibilityChangeMask|ButtonPressMask|KeyPressMask|PointerMotionMask|StructureNotifyMask;
	styleMask=CWBorderPixel|CWBackPixel|CWEventMask|CWColormap;

	gWindow=XCreateWindow(gpDisplay,
			RootWindow(gpDisplay,gpXVisualInfo->screen),
			0,
			0,
			giWindowWidth,
			giWindowHeight,
			0,
			gpXVisualInfo->depth,
			InputOutput,
			gpXVisualInfo->visual,
			styleMask,
			&winAttribs);
	if(!gWindow)
	{
		printf("ERROR: Create Window Failed");
		unInitialize();
		exit(1);
	}
	XStoreName(gpDisplay,gWindow,"First XWindow");
	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_ATOM",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
	
	XMapWindow(gpDisplay,gWindow);
}

void ToggleFullscreen(void)
{
	//variable declaration
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};

	//code
	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);
	memset(&xev,0,sizeof(xev));
	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;
	xev.xclient.data.l[0]=bFullScreen?0:1;

	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);
	xev.xclient.data.l[1]=fullscreen;
	XSendEvent(gpDisplay,
		RootWindow(gpDisplay,gpXVisualInfo->screen),False,StructureNotifyMask,&xev);
}


void resize(int width, int height)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	perspectiveProjectionMatrix = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(gShaderProgramObject);

	mat4 modelViewMatrix;
	mat4 translationMatrix;

	modelViewMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	
	translationMatrix = translate(-3.0f, 1.0f, -5.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;

	glUniformMatrix4fv(mvUniform,
		1,
		GL_FALSE,
		modelViewMatrix);
	glUniformMatrix4fv(pUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);

	GLfloat whiteColor[] = { 1.0f,1.0f, 1.0f,1.0f };

	glUniform4fv(colorUniform, 1, (GLfloat *)whiteColor);
	glPointSize(3.0);
	glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements), pointElements, GL_DYNAMIC_DRAW);
		glDrawElements(GL_POINTS, 16, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	//2

	modelViewMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	translationMatrix = translate(0.0f, 1.0f, -5.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;

	glUniformMatrix4fv(mvUniform,
		1,
		GL_FALSE,
		modelViewMatrix);
	glUniformMatrix4fv(pUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);
	//glPointSize(3.0);

	glUniform4fv(colorUniform, 1, (GLfloat *)whiteColor);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements1), pointElements1, GL_DYNAMIC_DRAW);
	glDrawElements(GL_LINES,22, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	//3

	modelViewMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	translationMatrix = translate(2.5f, 1.0f, -5.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;

	glUniformMatrix4fv(mvUniform,
		1,
		GL_FALSE,
		modelViewMatrix);
	glUniformMatrix4fv(pUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);
	//glPointSize(3.0);

	glUniform4fv(colorUniform, 1, (GLfloat *)whiteColor);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements2), pointElements2, GL_DYNAMIC_DRAW);
	glDrawElements(GL_LINES, 16, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	
	modelViewMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	translationMatrix = translate(-3.0f, -1.0f, -5.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;

	glUniformMatrix4fv(mvUniform,
		1,
		GL_FALSE,
		modelViewMatrix);
	glUniformMatrix4fv(pUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);
	glUniform4fv(colorUniform, 1,(GLfloat *)whiteColor);
	glPointSize(3.0);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements3), pointElements3, GL_DYNAMIC_DRAW);
	glDrawElements(GL_LINES, 26, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	//2

	modelViewMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	translationMatrix = translate(0.0f, -1.0f, -5.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;

	glUniformMatrix4fv(mvUniform,
		1,
		GL_FALSE,
		modelViewMatrix);
	glUniformMatrix4fv(pUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);

	glUniform4fv(colorUniform, 1, (GLfloat *)whiteColor);
	//glPointSize(3.0);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements4), pointElements4, GL_DYNAMIC_DRAW);
	glDrawElements(GL_LINES, 18, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	//3

	modelViewMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	translationMatrix = translate(2.5f, -1.0f, -5.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;

	glUniformMatrix4fv(mvUniform,
		1,
		GL_FALSE,
		modelViewMatrix);
	glUniformMatrix4fv(pUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);



	glBindVertexArray(vao);

	GLfloat redColor[] = { 1.0f,0.0f, 0.0f,1.0f };
	glUniform4fv(colorUniform, 1, (GLfloat *)redColor);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements5_1), pointElements5_1, GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, 21, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	glUniform4fv(colorUniform, 1, (GLfloat *)whiteColor);
	


	modelViewMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	translationMatrix = translate(2.5f, -1.0f, -5.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;

	glUniformMatrix4fv(mvUniform,
		1,
		GL_FALSE,
		modelViewMatrix);
	glUniformMatrix4fv(pUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);



	glBindVertexArray(vao);

	GLfloat blueColor[] = { 0.0f,0.0f, 1.0f,1.0f };
	glUniform4fv(colorUniform, 1, (GLfloat *)blueColor);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements5_3), pointElements5_3, GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, 21, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	glUniform4fv(colorUniform, 1, (GLfloat *)whiteColor);

	modelViewMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	translationMatrix = translate(2.5f, -1.0f, -5.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;

	glUniformMatrix4fv(mvUniform,
		1,
		GL_FALSE,
		modelViewMatrix);
	glUniformMatrix4fv(pUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);



	glBindVertexArray(vao);

	GLfloat greenColor[] = { 0.0f,1.0f, 0.0f,1.0f };
	glUniform4fv(colorUniform, 1, (GLfloat *)greenColor);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements5_2), pointElements5_2, GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, 21, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	modelViewMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	translationMatrix = translate(2.5f, -1.0f, -5.0f);
	modelViewMatrix = modelViewMatrix*translationMatrix;
	glUniformMatrix4fv(mvUniform,
		1,
		GL_FALSE,
		modelViewMatrix);
	glUniformMatrix4fv(pUniform,
		1,
		GL_FALSE,
		perspectiveProjectionMatrix);

	glUniform4fv(colorUniform, 1, (GLfloat *)whiteColor);
	//glPointSize(3.0);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements2), pointElements2, GL_DYNAMIC_DRAW);
	glDrawElements(GL_LINES, 16, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);
	glXSwapBuffers(gpDisplay,gWindow);
}

void Initialize(void)
{
	//Function Prototype
	void resize(int, int);
	void unInitialize();
	GLenum result;

	//code
	gGLXCreateContextAttribsARB=(GLXCreateContextAttribsARBProc)
			glXGetProcAddressARB((GLubyte*)"glXCreateContextAttribsARB");
	if(gGLXCreateContextAttribsARB==NULL)
	{
		printf("glXGetProcAddressARB failed");
		unInitialize();
	}
	const GLint glAttribs[]={GLX_CONTEXT_MAJOR_VERSION_ARB,4,
				GLX_CONTEXT_MINOR_VERSION_ARB,2,
				GLX_CONTEXT_PROFILE_MASK_ARB,GLX_CONTEXT_CORE_PROFILE_BIT_ARB,None};

	gGLXContext=gGLXCreateContextAttribsARB(gpDisplay,gGlXFBConfig,0,true,glAttribs);
	if(gGLXContext)
	{
		const GLint Attribs[]={
				GLX_CONTEXT_MAJOR_VERSION_ARB,1,
				GLX_CONTEXT_MINOR_VERSION_ARB,0,
				None		
				};		
	}	
	if(!glXIsDirect(gpDisplay,gGLXContext))
	{
		printf("No hardware Rendering context");
	}
	else
	{
		printf("Obtained Context for hardware Rendering");
	}
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	
	result = glewInit();
	if (result != GLEW_OK)
	{
		printf("GLEW not done");
	}


	//Vertex Shader
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//Write Vertex Shader Object
	const char *vertexShaderSourceCode =
		"#version 420 core" \
		"\n" \
		"in vec4 vPosition;" \
		"uniform mat4 u_mv_matrix;" \
		"uniform mat4 u_p_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position=u_p_matrix*u_mv_matrix*vPosition;" \
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
				printf("%s\n", szLogInfo);
				free(szLogInfo);
				unInitialize();
				
				exit(0);
			}
		}
	}


	//Fragment Shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//Write Vertex Shader Object
	const char *fragmentShaderSourceCode =
		"#version 420 core" \
		"\n" \
		"uniform vec4 u_Color;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor=u_Color;" \
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
				printf("%s\n", szLogInfo);
				free(szLogInfo);
				unInitialize();
				
				exit(0);
			}
		}
	}

	//Create Program
	gShaderProgramObject = glCreateProgram();
	glAttachShader(gShaderProgramObject, gVertexShaderObject);
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	glBindAttribLocation(gShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");

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
				printf("%s\n", szLogInfo);
				free(szLogInfo);
				unInitialize();
				
				exit(0);
			}
		}
	}

	mvUniform = glGetUniformLocation(gShaderProgramObject, "u_mv_matrix");
	pUniform = glGetUniformLocation(gShaderProgramObject, "u_p_matrix");
	colorUniform = glGetUniformLocation(gShaderProgramObject, "u_Color");
	
	float pointVertices[] = { 
							0.0,0.75,0.0,
							0.25,0.75,0.0,
							0.50,0.75,0.0,
							0.75,0.75,0.0,

							0.0,0.5,0.0,
							0.25,0.5,0.0,
							0.50,0.5,0.0,
							0.75,0.5,0.0,

							0.0,0.25,0.0,
							0.25,0.25,0.0,
							0.50,0.25,0.0,
							0.75,0.25,0.0,

							0.0,0.0,0.0,
							0.25,0.0,0.0,
							0.50,0.0,0.0,
							0.75,0.0,0.0,
	};


	//Rectangle vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//Position
	glGenBuffers(1, &vbo_Position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_Position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pointVertices), pointVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//Elements
	glGenBuffers(1, &vbo_Element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_Element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pointElements), NULL, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	perspectiveProjectionMatrix = mat4::identity();

	resize(giWindowWidth,giWindowHeight);
}
void update()
{}
void unInitialize(void)
{

	if (vbo_Position)
	{
		glDeleteBuffers(1, &vbo_Position);
		vbo_Position = 0;
	}


	if (vbo_Element)
	{
		glDeleteBuffers(1, &vbo_Element);
		vbo_Element = 0;
	}

	if (vao)
	{
		glDeleteBuffers(1, &vao);
		vao = 0;
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

	GLXContext currentGLXContext;
	currentGLXContext=glXGetCurrentContext();

	if(currentGLXContext!=NULL && currentGLXContext==gGLXContext)
	{
		glXMakeCurrent(gpDisplay,0,0);
	}
	
	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay,gGLXContext);
	}
	if(gWindow)
	{
		XDestroyWindow(gpDisplay,gWindow);
	}
	if(gColormap)
	{
		XFreeColormap(gpDisplay,gColormap);;		
	}
	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo=NULL;
	}
	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}
	
}

