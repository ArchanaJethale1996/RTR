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
void DrawPlane();
void update();
void resetTransitions();
void DrawFlagPlaneLines();

//variables
bool gI1Complete = false;
float xI1Translate = -3.5f;
float yI1Translate = 0.0f;

bool gNComplete = false;
float xNTranslate = -1.3f + 0.5f;
float yNTranslate = 2.5f;

bool gDComplete = false;
float xDBlend = 0.0f;

bool gI2Complete = false;
float xI2Translate = 0.0f;
float yI2Translate = -3.0f;

bool gAComplete = false;
float xATranslate = 3.5f;
float yATranslate = 0.0f;

bool gPlaneComplete = false;
float xPlaneTranslate = -4.0f;
float xPlaneBlend = 1.0f;

float anglem = 3.000f;
bool gBeginI1=false, gBeginA=false, gBeginN=false, gBeginI2 = false, gBeginD = false, gBeginPlane=false;
int counter=0;

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
		counter=counter+1;
		if(counter%5==0)
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
		
		//DrawPlane();

		//glLoadIdentity();
		//glTranslatef(xPlaneTranslate, (GLfloat)cos(anglem)+0.1f, -4.0f);
		//DrawPlane();

		glLoadIdentity();
		glTranslatef(xPlaneTranslate, (GLfloat)-cos(anglem) + 0.1f, -4.0f);
		DrawPlane();

		//Draw Lines
		if (xPlaneTranslate >= -1.5f + 0.9f)
		{
			glLoadIdentity();
			glTranslatef(0.0f, 0.1f, -4.0f);
			DrawFlagPlaneLines();
		}
	}
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
	//I1 translate

	if (xI1Translate <= -1.4f)
	{
		xI1Translate += 0.002f;
		yI1Translate = 0.0f;
	}
	else
	{
		gI1Complete = true;
	}
	
	if (gI1Complete)
	{
		//A Translate
		if (xATranslate >= -1.26f + 4 * 0.5f)
		{
			xATranslate -= 0.002f;
			yATranslate = 0.0f;
		}
		else
		{
			gAComplete = true;
		}
	}

	if (gAComplete)
	{
		//N Translate
		if (yNTranslate >= 0.01f)
		{
			xNTranslate = -1.34f + 0.5f;
			yNTranslate -= 0.002f;
		}
		else
		{
			gNComplete = true;
		}
	}

	if (gNComplete)
	{
		//I2 Translate
		if (yI2Translate <= 0.0f)
		{
			xI2Translate = -1.35f + 3 * 0.5f;
			yI2Translate += 0.002f;
		}
		else
		{
			gI2Complete = true;
		}
	}

	if (gI2Complete)
	{
		//D fade Translate
		if (xDBlend <= 1.0f)
		{
			xDBlend += 0.0005f;
		}
		else
		{
			gDComplete = true;
		}
	}

	if (gDComplete)
	{
		if (xPlaneTranslate <= 4.5f)
		{
			xPlaneTranslate += 0.0015f;
			if (anglem > 3.142f / 2.0f)
			{
				anglem -= 0.01f;
			}

			if (xPlaneTranslate > 2.3f)
			{
				if (anglem < 3.00f )
				{
					anglem += 0.02f;
				}
			}
		}
		else
		{

			gPlaneComplete = true;
		}

		if (gPlaneComplete)
		{
printf("plane complete");
			if (xPlaneBlend >= 0.0f)
			{
				xPlaneBlend -= 0.04f;
			}
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
	
	glBegin(GL_QUADS);
		glColor3f(0.462f, 0.788f, 0.831f);
		glVertex3f(-0.5f, -0.3f, 0.0f);
		glColor3f(0.729f, 0.866f, 0.933f);
		glVertex3f(0.0f, -0.3f, 0.0f);
		glColor3f(0.729f, 0.866f, 0.933f);
		glVertex3f(0.2f, 0.0f, 0.0f);
		glColor3f(0.462f, 0.788f, 0.831f);
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

	glColor3f(0.462f, 0.788f, 0.831f);
	glBegin(GL_TRIANGLES);
		glVertex3f(-0.2f, 0.0f, 0.0f);
		glVertex3f(-0.1f, 0.30f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();

	glColor3f(0.462f, 0.788f, 0.831f);
	glBegin(GL_TRIANGLES);
		glVertex3f(-0.5f, -0.3f, 0.0f);
		glVertex3f(-0.6f, -0.45f, 0.0f);
		glVertex3f(-0.6f, -0.15f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
		glColor4f(0.0745f, 0.533f, 0.0313f, rand());
		glVertex3f(-0.6f, -0.45f, 0.0f);

		glColor4f(0.0745f, 0.533f, 0.0313f, rand());
		glVertex3f(-0.6f, -0.35f, 0.0f);

		glColor4f(0.0745f, 0.533f, 0.0313f, rand());
		glVertex3f(-1.0f, -0.35f, 0.0f);

		glColor4f(0.0745f, 0.533f, 0.0313f, rand());
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

