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

//lights variables
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

	resize(giWindowWidth,giWindowHeight);
}
void update()
{
	if (KeyPressed == 1)
	{
		angleOfXRotation += 0.05f;
		if (angleOfXRotation >= 360.0f)
		{
			angleOfXRotation = 0.0f;
		}
	}
	if (KeyPressed == 2)
	{
		angleOfYRotation += 0.05f;
		if (angleOfYRotation >= 360.0f)
		{
			angleOfYRotation = 0.0f;
		}
	}
	if (KeyPressed == 3)
	{
		angleOfZRotation += 0.05f;
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
	gluSphere(quadric[0], 1.0f, 30, 30);


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
	
	for (int i = 0; i < 24; i++)
	{
		if (quadric[i])
		{
			gluDeleteQuadric(quadric[i]);
			quadric[i] = NULL;
		}
	}
}

