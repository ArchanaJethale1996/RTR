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

//Animated variable and Function
void drawIAlphabet();
void drawNAlphabet();
void drawDAlphabet();
void drawAAlphabet();

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	
	//Draw I
	glLoadIdentity();
	glTranslatef(-1.25f, 0.0f, -3.0f);
	drawIAlphabet();

	//Draw N
	glLoadIdentity();
	glTranslatef(-1.25 + 0.5f, 0.0f, -3.0f);
	drawNAlphabet();

	//Draw D
	glLoadIdentity();
	glTranslatef(-1.25 + 2*0.5f, 0.0f, -3.0f);
	drawDAlphabet();

	//Draw I
	glLoadIdentity();
	glTranslatef(-1.25 + 3 * 0.5f, 0.0f, -3.0f);
	drawIAlphabet();


	//Draw A
	glLoadIdentity();
	glTranslatef(-1.25 + 4*0.5f, 0.0f, -3.0f);
	drawAAlphabet();


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
	resize(giWindowWidth,giWindowHeight);
}
void update()
{
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


void DrawFontLines()
{
	glBegin(GL_QUADS);
		glColor3f(1.0f, 0.60f, 0.2f);
		glVertex2f(0.05f, 0.8f);

		glColor3f(1.0f, 0.60f, 0.2f);
		glVertex2f(-0.05f, 0.8f);

		glColor3f(0.0745f, 0.533f, 0.0313f);
		glVertex2f(-0.05f, -0.8f);

		glColor3f(0.0745f, 0.533f, 0.0313f);
		glVertex2f(0.05f, -0.8f);
	glEnd();
}


void DrawFontHorizotalLines()
{
	glBegin(GL_QUADS);
		glVertex2f(0.1f, 0.05f);
		glVertex2f(-0.1f, 0.05f);
		glVertex2f(-0.1f, -0.05f);
		glVertex2f(0.1f, -0.05f);
	glEnd();
}


void drawIAlphabet()
{
	glTranslatef(0.20f, 0.0f, 0.0f);
	DrawFontLines();
}
void drawNAlphabet()
{
	glTranslatef(0.1f, 0.0f, 0.0f);
	DrawFontLines();
	glTranslatef(0.25f, 0.0f, 0.0f);
	DrawFontLines();
	glTranslatef(-0.25/2.0f, 0.0f, 0.0f);
	glRotatef(5.0f, 0.0f, 0.0f, 1.0f);
	DrawFontLines();
}
void drawDAlphabet()
{
	glTranslatef(0.1f, 0.0f, 0.0f);
	DrawFontLines();
	glTranslatef(0.25f, 0.0f, 0.0f);
	DrawFontLines();

	glColor3f(1.0f, 0.60f, 0.2f);
	glTranslatef(-0.25f/2.0f, 0.75f, 0.0f);
	DrawFontHorizotalLines();

	glColor3f(0.0745f, 0.533f, 0.0313f);
	glTranslatef(0.0f, -0.75f*2.0f, 0.0f);
	DrawFontHorizotalLines();
}
void drawAAlphabet()
{
	glTranslatef(0.1f, 0.0f, 0.0f);
	glRotatef(-5.0f, 0.0f, 0.0f, 1.0f);
	DrawFontLines();

	glRotatef(5.0f, 0.0f, 0.0f, 1.0f);

	glTranslatef(0.15f, 0.0f, 0.0f);
	glRotatef(5.0f, 0.0f, 0.0f, 1.0f);
	DrawFontLines();

	glRotatef(-5.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.25f, -0.2f, 0.0f);

	glColor3f(1.0f, 0.60f, 0.2f);
	glTranslatef((0.4f / 2.0f)-0.02f, 0.1f, 0.0f);
	DrawFontHorizotalLines();

	glColor3f(1.00f, 1.00f, 1.00f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	DrawFontHorizotalLines();

	glColor3f(0.0745f, 0.533f, 0.0313f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	DrawFontHorizotalLines();

	glTranslatef(-0.5f / 2.0f, 0.1f, 0.0f);

}

