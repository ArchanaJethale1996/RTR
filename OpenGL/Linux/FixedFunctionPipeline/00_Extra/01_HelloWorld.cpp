#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>			//smilar to windows.h
#include<X11/Xutil.h>			//XVisualInfo
#include<X11/XKBlib.h>		//keyboard utilization header
#include<X11/keysym.h>		//key symbol

using namespace std;

bool bFullScreen=false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;

Colormap gColormap;
Window gWindow;
int giWindowWidth=800;
int giWindowHeight=600;

int main(void)
{
	//function prototyppe	
	void CreateWindow(void);
	void ToggleFullscreen(void);
	void unInitialize();

	//variable declaration
	int winWidth=giWindowWidth;
	int winHeight=giWindowHeight;
	static GC gc;
	static XGCValues gcValues;
	static XColor TextColor;
	char str[255]="Hello World !!";
	static int strLength,strWidth,strHeight;
	static XFontStruct *pxFontStruct=NULL;

	//Code
	CreateWindow();

	//Message Loop
	XEvent event;
	KeySym keysym;

	while(1)
	{
		XNextEvent(gpDisplay,&event);
		switch(event.type)
		{
			case MapNotify:
				pxFontStruct=XLoadQueryFont(gpDisplay,"-bitstream-courier 10 pitch-bold-i-normal--0-0-0-0-m-0-adobe-standard");
				if(pxFontStruct==NULL)
				{
					printf("Error: LoadQueryFont failed");
					exit(0);				
				}
				break;
			case KeyPress:
				keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
				switch(keysym)
				{
					case XK_Escape:
						XFreeGC(gpDisplay,gc);
						XUnloadFont(gpDisplay,pxFontStruct->fid);
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
				break;
			case Expose:
				gc=XCreateGC(gpDisplay,gWindow,0,&gcValues);
				XSetFont(gpDisplay,gc,pxFontStruct->fid);
			XAllocNamedColor(gpDisplay,gColormap,"green",&TextColor,&TextColor);
				XSetForeground(gpDisplay,gc,TextColor.pixel);
				strLength=strlen(str);
				strWidth=XTextWidth(pxFontStruct,str,strLength);
				strHeight=pxFontStruct->ascent+pxFontStruct->descent;
				XDrawString(gpDisplay,gWindow,gc,(winWidth-strWidth)/2,(winHeight-strHeight)/2,str,strLength);
				break;
			case 33:
				XFreeGC(gpDisplay,gc);
				XUnloadFont(gpDisplay,pxFontStruct->fid);
				unInitialize();
				exit(0);
			default:
				break;
		}		
		
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

	//code
	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
			printf("ERROR: Uable to Open X Display");
			unInitialize();
			exit(1);
	}

	defaultScreen=XDefaultScreen(gpDisplay);
	defaultDepth=DefaultDepth(gpDisplay,defaultScreen);
	gpXVisualInfo=(XVisualInfo *)malloc(sizeof(XVisualInfo));
	if(gpXVisualInfo==NULL)
	{
		printf("ERROR: Unable o allocate memory for XVisual");
		unInitialize();
		exit(1);
	}

	XMatchVisualInfo(gpDisplay,defaultScreen,defaultDepth,TrueColor,gpXVisualInfo);
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

void unInitialize(void)
{
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
