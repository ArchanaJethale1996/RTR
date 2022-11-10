
//headers
#import<Foundation/Foundation.h>
#import<Cocoa/Cocoa.h>

#import<QuartzCore/CVDIsplayLink.h>

#import<OpenGL/gl3.h>
#import<OpenGL/gl3ext.h>
#import"vmath.h"

enum
{
    AMC_ATTRIBUTE_POSITION=0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXTURE0,
};
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,const CVTimeStamp *pNow,const CVTimeStamp *pOutputTime,CVOptionFlags flagsIn,CVOptionFlags *pFlagsOut,void *pDisplayLinkContext);

FILE *gpFile=NULL;

//interface declarations
@interface AppDelegate:NSObject<NSApplicationDelegate,NSWindowDelegate>
@end

@interface GLView:NSOpenGLView
@end

//Entry point function
int main(int argc,const char *argv[])
{
    NSAutoreleasePool *pPool=[[NSAutoreleasePool alloc]init];
    
    NSApp=[NSApplication sharedApplication];
    
    [NSApp setDelegate:[[AppDelegate alloc]init]];
    
    [NSApp run];
    [pPool release];
    return(0);
}

@implementation AppDelegate
{
@private
    NSWindow *window;
    GLView *glView;
}

-(void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //code
    //log file
    NSBundle *mainBundle=[NSBundle mainBundle];
    NSString *appDirName=[mainBundle bundlePath];
    NSString *parentDirPath=[appDirName stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath=[NSString stringWithFormat:@"%@/Log.txt",parentDirPath];
    const char* pszLogFileNameWithPath=[logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];
    
    gpFile=fopen(pszLogFileNameWithPath,"w");
    if(gpFile==NULL)
    {
        printf("Cannot create Log file\n");
        [self release];
        [NSApp terminate:self];
    }
    
    fprintf(gpFile,"Program is started successfully\n");
    NSRect win_rect;
    win_rect=NSMakeRect(0.0,0.0,800.0,600.0);
    
    window=[[NSWindow alloc] initWithContentRect:win_rect styleMask:NSWindowStyleMaskClosable|NSWindowStyleMaskTitled|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];
    [window setTitle:@"macOS OpenGL Window"];
    [window center];
    
    glView=[[GLView alloc]initWithFrame:win_rect];
    [window setContentView:glView];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    //code
    fprintf(gpFile,"Program is termianted successsfully\n");
    
    if(gpFile)
    {
        fclose(gpFile);
        gpFile=NULL;
    }
}

-(void)applicatonWillClose:(NSNotification *)notification
{
    [NSApp terminate:self];
}

-(void)dealloc
{
    [glView release];
    [window release];
    [super dealloc];
}
@end

@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;
    
    GLuint vertexShaderObject;
    GLuint fragmentShaderObject;
    GLuint shaderProgramObject;
    
    GLuint vao_Rectangle;
    GLuint vbo_Position_Rectangle;
    GLuint vbo_Texture_Rectangle;
    GLuint mvpUniform;
    GLuint texture_Smiley;
    
    GLuint samplerUniform;
    vmath::mat4 perspectiveProjectionMatrix;
    int keyPressed;
}

-(id)initWithFrame:(NSRect)frame
{
    //code
    self=[super initWithFrame:frame];
    if(self)
    {
        [[self window]setContentView:self];
        
        NSOpenGLPixelFormatAttribute attrs[]={
            NSOpenGLPFAOpenGLProfile,
            NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAScreenMask,CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize,24,
            NSOpenGLPFADepthSize,24,
            NSOpenGLPFAAlphaSize,8,
            NSOpenGLPFADoubleBuffer,
            0
        };
        
        NSOpenGLPixelFormat *pixelFormat=[[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
        if(pixelFormat==nil)
        {
            fprintf(gpFile,"No valid OpenGL pixel Format is available");
            [self release];
            [NSApp terminate:self];
        }
        NSOpenGLContext *glContext=[[[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil] autorelease];
        [self setPixelFormat:pixelFormat];
        [self setOpenGLContext:glContext];
        
        
    }
    return(self);
}

-(CVReturn)getFrameForTime:(const CVTimeStamp *)pOutputTime
{
    NSAutoreleasePool *pool=[[NSAutoreleasePool alloc]init];
    [self drawView];
    [pool release];
    return(kCVReturnSuccess);
}

-(void)prepareOpenGL
{
    //code
    keyPressed=0;
    //openGL Info
    fprintf(gpFile,"OpenGL version : %s\n",glGetString(GL_VERSION));
    fprintf(gpFile,"GLSL version : %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    [[self openGLContext]makeCurrentContext];
    
    GLint swapInt=1;
    [[self openGLContext]setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    //Vertex shader
    vertexShaderObject=glCreateShader(GL_VERTEX_SHADER);
    
    const GLchar* vertexShaderSourceCode=
    "#version 410" \
    "\n" \
    "in vec4 vPosition;" \
    "in vec2 vTexCoord;" \
    "uniform mat4 u_mvp_matrix;"
    "out vec2 out_TexCoord;"
    "void main(void)" \
    "{" \
    "gl_Position=u_mvp_matrix*vPosition;" \
    "out_TexCoord=vTexCoord;" \
    "}";
    glShaderSource(vertexShaderObject,1,(const GLchar**)&vertexShaderSourceCode,NULL);
    
    glCompileShader(vertexShaderObject);
    GLint iInfoLogLength=0;
    GLint iShaderCompileStatus=0;
    char* szInfoLog=NULL;
    glGetShaderiv(vertexShaderObject,GL_COMPILE_STATUS,&iShaderCompileStatus);
    if(iShaderCompileStatus==GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength>0)
        {
            szInfoLog=(char*)malloc(iInfoLogLength);
            if(szInfoLog!=NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Vertex Shader Compilation Log : %s\n",szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    //Fragment shader
    fragmentShaderObject=glCreateShader(GL_FRAGMENT_SHADER);
    
    const GLchar* fragmentShaderSourceCode="#version 410" \
    "\n" \
    "in vec2 out_TexCoord;" \
    "uniform sampler2D u_sampler;" \
    "out vec4 FragColor;" \
    "void main(void)" \
    "{" \
    "FragColor=texture(u_sampler,out_TexCoord);" \
    "}";
    glShaderSource(fragmentShaderObject,1,(const GLchar**)&fragmentShaderSourceCode,NULL);
    
    glCompileShader(fragmentShaderObject);
    iInfoLogLength=0;
    iShaderCompileStatus=0;
    szInfoLog=NULL;
    glGetShaderiv(fragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompileStatus);
    if(iShaderCompileStatus==GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength>0)
        {
            szInfoLog=(char *)malloc(iInfoLogLength);
            if(szInfoLog!=NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Fragment Shader Compilation Log : %s\n",szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    
    //shader program
    shaderProgramObject=glCreateProgram();
    glAttachShader(shaderProgramObject,vertexShaderObject);
    glAttachShader(shaderProgramObject,fragmentShaderObject);
    
    glBindAttribLocation(shaderProgramObject,AMC_ATTRIBUTE_POSITION,"vPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXTURE0, "vTexCoord");
    
    glLinkProgram(shaderProgramObject);
    GLint iShaderProgramLinkStatus=0;
    glGetProgramiv(shaderProgramObject,GL_LINK_STATUS,&iShaderProgramLinkStatus);
    if(iShaderProgramLinkStatus==GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength>0)
        {
            szInfoLog=(char *)malloc(iInfoLogLength);
            if(iInfoLogLength>0)
            {
                GLsizei written;
                glGetProgramInfoLog(shaderProgramObject,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Shader Program Link Log : %s\n",szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    mvpUniform=glGetUniformLocation(shaderProgramObject,"u_mvp_matrix");
    samplerUniform= glGetUniformLocation(shaderProgramObject, "u_sampler");
    
    texture_Smiley=[self loadTextureFromBMPFile:"Smiley.bmp"];
    
    const GLfloat rectangleVertices[] = {
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
    };
    
    //Rectangle
    glGenVertexArrays(1, &vao_Rectangle);
    glBindVertexArray(vao_Rectangle);
    //Position
    glGenBuffers(1, &vbo_Position_Rectangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_Position_Rectangle);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(rectangleVertices),
                 rectangleVertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //Texture
    glGenBuffers(1, &vbo_Texture_Rectangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_Texture_Rectangle);
    glBufferData(GL_ARRAY_BUFFER,
                 8*sizeof(GLfloat),
                 NULL,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXTURE0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindVertexArray(0);
    
    
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    
    perspectiveProjectionMatrix=vmath::mat4::identity();
    
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink,&MyDisplayLinkCallback,self);
    CGLContextObj cglContext=(CGLContextObj)[[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat=(CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink,cglContext,cglPixelFormat);
    CVDisplayLinkStart(displayLink);
}

-(GLuint)loadTextureFromBMPFile:(const char *)texFileName
{
    printf("%s",texFileName);
    NSBundle *mainBundle=[NSBundle mainBundle];
    NSString *appDirName=[mainBundle bundlePath];
    NSString *parentDirPath=[appDirName stringByDeletingLastPathComponent];
    NSString *textureFileNameWithPath=[NSString stringWithFormat:@"%@/%s",parentDirPath,texFileName];
    
    NSImage *bmpImage=[[NSImage alloc] initWithContentsOfFile:textureFileNameWithPath];
    if(!bmpImage)
    {
        NSLog(@"cant't find %@",textureFileNameWithPath);
        return(0);
    }
    
    CGImageRef cgImage=[bmpImage CGImageForProposedRect:nil context:nil hints:nil];
    
    int w=(int)CGImageGetWidth(cgImage);
    int h=(int)CGImageGetHeight(cgImage);
    
    CFDataRef imageData=CGDataProviderCopyData(CGImageGetDataProvider(cgImage));
    void *pixels=(void *)CFDataGetBytePtr(imageData);
    
    GLuint bmpTexture;
    glGenTextures(1,&bmpTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    
    glBindTexture(GL_TEXTURE_2D,bmpTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    CFRelease(imageData);
    return(bmpTexture);
}

-(void)reshape
{
    //code
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    NSRect rect=[self bounds];
    GLfloat width=rect.size.width;
    GLfloat height=rect.size.height;
    
    if(height==0)
        height=1;
    
    glViewport(0,0,(GLsizei)width,(GLsizei)height);
    
    perspectiveProjectionMatrix=vmath::perspective(45.0f,width/height,0.1f,100.0f);
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

-(void)drawRect:(NSRect)dirtyRect
{
    [self drawView];
}

-(void)drawView
{
    [[self openGLContext]makeCurrentContext];
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);
    vmath::mat4 modelViewMatrix=vmath::mat4::identity();
    vmath::mat4 translationMatrix=vmath::mat4::identity();
    vmath::mat4 rotationMatrix=vmath::mat4::identity();
    vmath::mat4 modelViewProjectionMatrix=vmath::mat4::identity();
    
    translationMatrix=vmath::translate(0.0f, 0.0f, -3.0f);
    modelViewMatrix=modelViewMatrix*translationMatrix;
    modelViewProjectionMatrix=perspectiveProjectionMatrix*modelViewMatrix;
    
    glUniformMatrix4fv(mvpUniform,1,GL_FALSE,modelViewProjectionMatrix);
    glBindTexture(GL_TEXTURE_2D,texture_Smiley);
    
    
    glUniformMatrix4fv(mvpUniform,
                       1,
                       GL_FALSE,
                       modelViewProjectionMatrix);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_Texture_Rectangle);
    
    if (keyPressed == 0)
    {
        GLfloat texCood[] = { 0.5f, 0.5f,
            0.5f, 0.5f,
            0.5f, 0.5f,
            0.5,0.5 };
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(texCood),
                     texCood,
                     GL_DYNAMIC_DRAW);
    }
    if (keyPressed == 1)
    {
        GLfloat texCood[] = { 2.0f, 2.0f,
            0.0f, 2.0f,
            0.0f, 0.0f,
            2.0f, 0.0f };
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(texCood),
                     texCood,
                     GL_DYNAMIC_DRAW);
        
    }
    if (keyPressed == 2)
    {
        GLfloat texCood[] ={ 0.5f, 0.5f,
            0.0f, 0.5f,
            0.0f, 0.0f,
            0.5f, 0.0f
        };
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(texCood),
                     texCood,
                     GL_DYNAMIC_DRAW);
        
    }
    
    if (keyPressed == 3)
    {
        GLfloat texCood[] ={ 1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
        };
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(texCood),
                     texCood,
                     GL_DYNAMIC_DRAW);
        
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindVertexArray(vao_Rectangle);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    
    glUseProgram(0);
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}
-(BOOL)acceptsFirstResponder
{
    [[self window]makeFirstResponder:self];
    return(YES);
}

-(void)keyDown:(NSEvent *)theEvent
{
    int key=(int)[[theEvent characters]characterAtIndex:0];
    switch(key)
    {
        case 27:
            [self release];
            [NSApp terminate:self];
            break;
        case 49:
            keyPressed = 0;
            break;
        case 50:
            keyPressed = 1;
            break;
        case 51:
            keyPressed = 2;
            break;
        case 52:
            keyPressed = 3;
            break;
        case 'F':
        case 'f':
            [[self window]toggleFullScreen:self];
            break;
        default:
            break;
    }
}

-(void)mouseDown:(NSEvent *)theEvent
{
    //code
}

-(void)mouseDragged:(NSEvent *)theEvent
{
    //code
}

-(void)rightMouseDown:(NSEvent *)theEvent
{
    //code
}

-(void) dealloc{
    //code
    if (vbo_Position_Rectangle)
    {
        glDeleteBuffers(1, &vbo_Position_Rectangle);
        vbo_Position_Rectangle = 0;
    }
    if (vbo_Texture_Rectangle)
    {
        glDeleteBuffers(1, &vbo_Texture_Rectangle);
        vbo_Texture_Rectangle = 0;
    }
    if (vao_Rectangle)
    {
        glDeleteBuffers(1, &vao_Rectangle);
        vao_Rectangle = 0;
    }
    
    
    glDetachShader(shaderProgramObject,vertexShaderObject);
    glDetachShader(shaderProgramObject,fragmentShaderObject);
    glDeleteShader(vertexShaderObject);
    vertexShaderObject=0;
    glDeleteShader(fragmentShaderObject);
    fragmentShaderObject=0;
    
    glDeleteProgram(shaderProgramObject);
    shaderProgramObject=0;
    
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
    [super dealloc];
}
@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,const CVTimeStamp *pNow,const CVTimeStamp *pOutputTime,CVOptionFlags flagsIn,CVOptionFlags *pFlagsOut,void *pDisplayLinkContext)
{
    CVReturn result=[(GLView *)pDisplayLinkContext getFrameForTime:pOutputTime];
    return(result);
}

