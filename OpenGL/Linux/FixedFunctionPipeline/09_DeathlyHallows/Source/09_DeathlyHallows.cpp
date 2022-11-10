#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<math.h>

#include<X11/Xlib.h>			//smilar to windows.h
#include<X11/Xutil.h>			//XVisualInfo
#include<X11/XKBlib.h>		//keyboard utilization header
#include<X11/keysym.h>		//key symbol

//openGL
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>
using namespace std;

#define PI 3.14169
#define NUM_POINTS 1000

bool bFullScreen=true;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;

Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;
GLXContext gGLXContext;

//Animation Variables and function
void DrawTriangle();
void DrawCircle(GLfloat);
void DrawLine(GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2);

GLfloat CircleRotate=0.0f;
bool triangleComplete = false;
bool circleComplete=false;
bool lineComplete=false;
float xTranslateLine = 0.0f, yTranslateLine = 1.0f;
float xTranslateTriangle = 2.0f, yTranslateTriangle = 0.0f;
float xTranslateCircle = -2.0f, yTranslateCircle = -1.0f;

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
	
	//ToggleFullscreen();
	//bFullScreen=false;
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
				GLX_RED_SIZE,1,
				GLX_GREEN_SIZE,1,
				GLX_BLUE_SIZE,1,
				GLX_ALPHA_SIZE,1,
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

	glXSwapBuffers(gpDisplay,gWindow);
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

void Initialize(void)
{
	//Function Prototype
	void resize(int, int);
	void ToggleFullscreen(void);
	//code
	gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	resize(giWindowWidth,giWindowHeight);
	ToggleFullscreen();
	bFullScreen=false;
}

void update()
{
	//triangle
	if (xTranslateTriangle > 0.0f)
	{
		xTranslateTriangle -= 0.0005f;
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
			xTranslateCircle += 0.0001f;
			yTranslateCircle += 0.0001f;
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
			yTranslateLine -= 0.0001;
		}
		else
		{
			lineComplete = true;
		}
	}
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

