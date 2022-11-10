#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>			//smilar to windows.h
#include<X11/Xutil.h>			//XVisualInfo
#include<X11/XKBlib.h>		//keyboard utilization header
#include<X11/keysym.h>		//key symbol

//openGL
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>
using namespace std;

bool bFullScreen=false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;

Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;
GLXContext gGLXContext;

//lights variabless
bool bLight = false;
GLfloat lightAmbient[] = { 0.5f,0.5f,0.5f,1.0f };
GLfloat lightDiffused[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 0.0f,0.0f,2.0f,1.0f };

GLfloat materialAmbient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat materialDiffused[]={1.0f,1.0f,1.0f,1.0f};
GLfloat materialSpecular[]={1.0f,1.0f,1.0f,1.0f};
GLfloat materialShininess[]={128.0f};

GLUquadric *quadric=NULL;

//animation variable
GLfloat angleCube=0.0f;

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
						case XK_l:
						case XK_L:
							if(bLight==false)
							{
								glEnable(GL_LIGHTING);
								bLight=true;
							}	
							else
							{
								glDisable(GL_LIGHTING);
								bLight=false;
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

void CreateWindow()
{
	//function prototype
	void unInitialize();

	//variable declarations
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;
	static int frameBufferAttribs[]={GLX_RGBA,
				GLX_RED_SIZE,8,
				GLX_GREEN_SIZE,8,
				GLX_BLUE_SIZE,8,
				GLX_ALPHA_SIZE,8,
				GLX_DEPTH_SIZE,24,
				GLX_DOUBLEBUFFER,				
				None};
	//code
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
			printf("ERROR: Uable to Open X Display");
			unInitialize();
			exit(1);
	}

	defaultScreen=XDefaultScreen(gpDisplay);
	gpXVisualInfo=glXChooseVisual(gpDisplay,defaultScreen,frameBufferAttribs);
	if(gpXVisualInfo==NULL)
	{
		printf("ERROR: XMatchVisualInfo failed");
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
	if(height==0)
		height=1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-0.70f);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	quadric=gluNewQuadric();
	gluSphere(quadric,0.2,50,50);
	glXSwapBuffers(gpDisplay,gWindow);
}

void Initialize(void)
{
	//Function Prototype
	void resize(int, int);

	//code
	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);	

	glLightfv(GL_LIGHT1,GL_AMBIENT,lightAmbient);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,lightDiffused);
	glLightfv(GL_LIGHT1,GL_POSITION,lightPosition);

	glEnable(GL_LIGHT1);

	glMaterialfv(GL_FRONT,GL_AMBIENT,materialAmbient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffused);
	glMaterialfv(GL_FRONT,GL_SPECULAR,materialSpecular);
	glMaterialfv(GL_FRONT,GL_SHININESS,materialShininess);
	resize(giWindowWidth,giWindowHeight);
}
void update()
{
	angleCube=angleCube+0.05f;
	if(angleCube>=360)
		angleCube=0;
}
void unInitialize(void)
{
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

