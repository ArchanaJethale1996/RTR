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

GLuint gVertexShaderObjectPerVertex;
GLuint gFragmentShaderObjectPerVertex;
GLuint gShaderProgramObjectPerVertex;

GLuint gVertexShaderObjectPerFragment;
GLuint gFragmentShaderObjectPerFragment;
GLuint gShaderProgramObjectPerFragment;

bool gAnimate = false;
int gLighting = 0;

bool bPerVertex=true;

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

mat4 perspectiveProjectionMatrix;

enum
{
	AMC_ATTRIBUTE_POSITION=0,
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

GLuint samplerUniform;
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
						case XK_M:
						case XK_m:
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
						case XK_V:
						case XK_v:
							bPerVertex = !bPerVertex;
							break;
						case XK_L:
						case XK_l:
							if (gLighting == 0)
								gLighting = 1;
							else
								gLighting = 0;
							break;
						case XK_A:
						case XK_a:
							gAnimate = !gAnimate;
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

//////////////////////////////////////////////////////////////
	//Per Vertex
	//////////////////////////////////////////////////////////////

	//Vertex Shader
	gVertexShaderObjectPerVertex = glCreateShader(GL_VERTEX_SHADER);

	//Write Vertex Shader Object
	const char *vertexShaderSourceCodePerVertex =
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
				printf("%s\n", szLogInfo);
				free(szLogInfo);
				unInitialize();
				exit(0);
			}
		}
	}


	//Fragment Shader
	gFragmentShaderObjectPerVertex = glCreateShader(GL_FRAGMENT_SHADER);

	//Write Vertex Shader Object
	const char *fragmentShaderSourceCodePerVertex =
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
				printf("%s\n", szLogInfo);
				free(szLogInfo);
				unInitialize();
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
				printf("%s\n", szLogInfo);
				free(szLogInfo);
				unInitialize();
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
	const char *vertexShaderSourceCodePerFragment =
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
				printf("%s\n", szLogInfo);
				free(szLogInfo);
				unInitialize();
				exit(0);
			}
		}
	}


	//Fragment Shader
	gFragmentShaderObjectPerFragment = glCreateShader(GL_FRAGMENT_SHADER);

	//Write Vertex Shader Object
	const char *fragmentShaderSourceCodePerFragment =
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
				printf("%s\n", szLogInfo);
				free(szLogInfo);
				unInitialize();
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
				printf("%s\n", szLogInfo);
				free(szLogInfo);
				unInitialize();
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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	perspectiveProjectionMatrix = mat4::identity();
	glDisable(GL_CULL_FACE);

	resize(giWindowWidth,giWindowHeight);
}
void update()
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

