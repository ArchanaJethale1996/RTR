
//headers
#import<Foundation/Foundation.h>
#import<Cocoa/Cocoa.h>

#import<QuartzCore/CVDIsplayLink.h>

#import<OpenGL/gl3.h>
#import<OpenGL/gl3ext.h>
#import"vmath.h"

#include"SphereH.h"

enum
{
    AMC_ATTRIBUTE_POSITION=0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXTURE0,
};

//vertices for sphere
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumElements, gNumVertices;

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
    
    GLuint gVertexShaderObjectPerVertex;
    GLuint gFragmentShaderObjectPerVertex;
    GLuint gShaderProgramObjectPerVertex;
    
    GLuint gVertexShaderObjectPerFragment;
    GLuint gFragmentShaderObjectPerFragment;
    GLuint gShaderProgramObjectPerFragment;
    Sphere *sphere;
    GLuint vao;
    GLuint vbo_position,vbo_normal,vbo_texture,vbo_index;
    GLuint mUniformPerVertex, vUniformPerVertex, pUniformPerVertex, LDUniformPerVertex, KDUniformPerVertex, LAUniformPerVertex, KAUniformPerVertex, KSUniformPerVertex, LSUniformPerVertex, LightPositionUniformPerVertex, MaterialShininessUniformPerVertex, LKeyIsPressedUniformPerVertex;
    GLuint mUniformPerFragment, vUniformPerFragment, pUniformPerFragment, LDUniformPerFragment, KDUniformPerFragment, LAUniformPerFragment, KAUniformPerFragment, KSUniformPerFragment, LSUniformPerFragment, LightPositionUniformPerFragment, MaterialShininessUniformPerFragment, LKeyIsPressedUniformPerFragment;
    vmath::mat4 perspectiveProjectionMatrix;
    float rotateSphere;
    bool gAnimate,gFragment;
    int gLighting;
    
    float lightAmbient[4];
    float lightDiffused[4];
    float lightSpecular[4];
    float lightPosition[4];
    
    float materialAmbient[4];
    float materialDiffused[4];
    float materialSpecular[4];
    float materialShininess;
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
    lightAmbient[0] = 0.0f;
    lightAmbient[1] =0.0f;
    lightAmbient[2] =0.0f;
    lightAmbient[3] =1.0f ;
    lightDiffused[0] = 1.0f;
    lightDiffused[1] = 1.0f;
    lightDiffused[2] = 1.0f;
    lightDiffused[3] = 1.0f;
    lightSpecular[0] = 1.0f;
    lightSpecular[1] = 1.0f;
    lightSpecular[2] = 1.0f;
    lightSpecular[3] = 1.0f;
    lightPosition[0] = 100.0f;
    lightPosition[1] = 100.0f;
    lightPosition[2] = 100.0f;
    lightPosition[3] = 1.0f;
    
    materialAmbient[0] =  0.0f;
    materialAmbient[1] =  0.0f;
    materialAmbient[2] =  0.0f;
    materialAmbient[3] =  1.0f;
    materialDiffused[0] = 1.0f;
    materialDiffused[1] = 1.0f;
    materialDiffused[2] = 1.0f;
    materialDiffused[3] = 1.0f;
    materialSpecular[0] = 1.0f;
    materialSpecular[1] = 1.0f;
    materialSpecular[2] = 1.0f;
    materialSpecular[3] = 1.0f;
    materialShininess = 250.0f;
    gAnimate = false;
    gLighting = 0;
    gFragment=false;
    rotateSphere = 0.0f;
    //code
    //openGL Info
    fprintf(gpFile,"OpenGL version : %s\n",glGetString(GL_VERSION));
    fprintf(gpFile,"GLSL version : %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    [[self openGLContext]makeCurrentContext];
    
    GLint swapInt=1;
    [[self openGLContext]setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    ////////////////////////////////////////////////////
    //Per Vertex
    
    
    //Vertex Shader
    gVertexShaderObjectPerVertex = glCreateShader(GL_VERTEX_SHADER);
    
    //Write Vertex Shader Object
    GLchar *vertexShaderSourceCodePerVertex =
    "#version 410 core" \
    "\n" \
    "in vec4 vPosition;" \
    "in vec3 vNormal;" \
    "uniform mat4 u_m_matrix;" \
    "uniform mat4 u_v_matrix;" \
    "uniform mat4 u_p_matrix;" \
    "uniform vec3 u_Ld;" \
    "uniform vec3 u_Kd;" \
    "uniform vec3 u_Ls;" \
    "uniform vec3 u_Ks;" \
    "uniform vec3 u_La;" \
    "uniform vec3 u_Ka;" \
    "uniform vec4 u_Light_Position;" \
    "uniform float u_MaterialShininess;" \
    "uniform int u_LKeyIsPressed;" \
    "out vec3 phong_ads_light;" \
    "void main(void)" \
    "{" \
    "if(u_LKeyIsPressed==1)" \
    "{" \
    "vec4 eye_coordinates=u_v_matrix*u_m_matrix*vPosition;" \
    
    "vec3 tNorm=normalize(mat3(u_v_matrix*u_m_matrix)*vNormal);" \
    "vec3 lightDirection=normalize(vec3(u_Light_Position-eye_coordinates));" \
    "float tndotld=max(dot(lightDirection,tNorm),0.0);" \
    "vec3 ReflectionVector=reflect(-lightDirection,tNorm);" \
    "vec3 viewerVector=normalize(vec3(-eye_coordinates.xyz));" \
    "vec3 ambient=u_La*u_Ka;" \
    "vec3 diffused=u_Ld*u_Kd*tndotld;" \
    "vec3 specular=u_Ls*u_Ks*pow(max(dot(ReflectionVector,viewerVector),0.0),u_MaterialShininess);" \
    "phong_ads_light = ambient+diffused+specular;" \
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
                glGetShaderInfoLog(gVertexShaderObjectPerVertex,iInfoLogLength,&written,szLogInfo);
                fprintf(gpFile,"Vertex Shader Compilation Log : %s\n",szLogInfo);
                free(szLogInfo);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    
    //Fragment Shader
    gFragmentShaderObjectPerVertex = glCreateShader(GL_FRAGMENT_SHADER);
    
    //Write Vertex Shader Object
    GLchar *fragmentShaderSourceCodePerVertex =
    "#version 410 core" \
    "\n" \
    "in vec3 phong_ads_light;" \
    "out vec4 FragColor;" \
    "void main(void)" \
    "{" \
    "FragColor=vec4(phong_ads_light,1.0);" \
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
                glGetShaderInfoLog(gFragmentShaderObjectPerVertex,iInfoLogLength,&written,szLogInfo);
                fprintf(gpFile,"Fragment Shader Compilation Log : %s\n",szLogInfo);
                free(szLogInfo);
                [self release];
                [NSApp terminate:self];
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
                glGetProgramInfoLog(gShaderProgramObjectPerVertex,iInfoLogLength,&written,szLogInfo);
                fprintf(gpFile,"Shader Program Link Log : %s\n",szLogInfo);
                free(szLogInfo);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    mUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_m_matrix");
    vUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_v_matrix");
    pUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_p_matrix");
    LDUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ld");
    KDUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Kd");
    LAUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_La");
    KAUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ka");
    LSUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ls");
    KSUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ks");
    MaterialShininessUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_MaterialShininess");
    LightPositionUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Light_Position");
    LKeyIsPressedUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LKeyIsPressed");
    
    
    ///////////////////////////////////////////////////
    //Per Fragment
    
    //Vertex Shader
    gVertexShaderObjectPerFragment = glCreateShader(GL_VERTEX_SHADER);
    
    //Write Vertex Shader Object
    GLchar *vertexShaderSourceCodePerFragment =
    "#version 410 core" \
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
                glGetShaderInfoLog(gVertexShaderObjectPerFragment,iInfoLogLength,&written,szLogInfo);
                fprintf(gpFile,"Vertex Shader Compilation Log : %s\n",szLogInfo);
                free(szLogInfo);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    
    //Fragment Shader
    gFragmentShaderObjectPerFragment = glCreateShader(GL_FRAGMENT_SHADER);
    
    //Write Vertex Shader Object
    GLchar *fragmentShaderSourceCodePerFragment =
    "#version 410 core" \
    "\n" \
    "in vec3 tNormVertexShader;" \
    "in vec4 eye_coordinatesVertexShader;" \
    "uniform vec3 u_Ld;" \
    "uniform vec3 u_Kd;" \
    "uniform vec3 u_Ls;" \
    "uniform vec3 u_Ks;" \
    "uniform vec3 u_La;" \
    "uniform vec3 u_Ka;" \
    "uniform vec4 u_Light_Position;" \
    "uniform float u_MaterialShininess;" \
    "uniform int u_LKeyIsPressed;" \
    "out vec4 FragColor;" \
    "void main(void)" \
    "{" \
    "if(u_LKeyIsPressed==1)" \
    "{" \
    "vec3 tNorm=normalize(tNormVertexShader);"
    "vec3 lightDirection=normalize(vec3(u_Light_Position-eye_coordinatesVertexShader));" \
    "float tndotld=max(dot(lightDirection,tNorm),0.0);" \
    "vec3 ReflectionVector=reflect(-lightDirection,tNorm);" \
    "vec3 viewerVector=normalize(vec3(-eye_coordinatesVertexShader.xyz));" \
    "vec3 ambient=u_La*u_Ka;" \
    "vec3 diffused=u_Ld*u_Kd*tndotld;" \
    "vec3 specular=u_Ls*u_Ks*pow(max(dot(ReflectionVector,viewerVector),0.0),u_MaterialShininess);" \
    "vec3 phong_ads_light = ambient+diffused+specular;" \
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
                glGetShaderInfoLog(gFragmentShaderObjectPerFragment,iInfoLogLength,&written,szLogInfo);
                fprintf(gpFile,"Fragment Shader Compilation Log : %s\n",szLogInfo);
                free(szLogInfo);
                [self release];
                [NSApp terminate:self];
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
                glGetProgramInfoLog(gShaderProgramObjectPerFragment,iInfoLogLength,&written,szLogInfo);
                fprintf(gpFile,"Shader Program Link Log : %s\n",szLogInfo);
                free(szLogInfo);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    mUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_m_matrix");
    vUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_v_matrix");
    pUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_p_matrix");
    LDUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ld");
    KDUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Kd");
    LAUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_La");
    KAUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ka");
    LSUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ls");
    KSUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ks");
    MaterialShininessUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_MaterialShininess");
    LightPositionUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Light_Position");
    LKeyIsPressedUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LKeyIsPressed");
    
    sphere=new Sphere();
    sphere->getSphereVertexData();
    
    gNumVertices = sphere->getNumberOfSphereVertices();
    gNumElements = sphere->getNumberOfSphereElements();
    
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    // vbo for position
    
    glGenBuffers(1,&vbo_position);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_position);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(sphere->model_vertices),
                 sphere->model_vertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION,3,GL_FLOAT,GL_FALSE,0,NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    printf("%d",sizeof(sphere->model_vertices));
    // vbo for normals
    glGenBuffers(1,&vbo_normal);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_normal);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(sphere->model_normals),
                 sphere->model_normals,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL,3,GL_FLOAT,GL_FALSE,0,NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    
    printf("%d",sizeof(sphere->model_normals));
    // vbo for texture
    glGenBuffers(1,&vbo_texture);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_texture);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(sphere->model_textures),
                 sphere->model_textures,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXTURE0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    
    printf("%d",sizeof(sphere->model_textures));
    
    // vbo for index
    glGenBuffers(1,&vbo_index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo_index);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(sphere->model_elements),
                 sphere->model_elements,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    
    printf("%d",sizeof(sphere->model_elements));
    glBindVertexArray(0);
    
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    
    perspectiveProjectionMatrix=vmath::mat4::identity();
    
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink,&MyDisplayLinkCallback,self);
    CGLContextObj cglContext=(CGLContextObj)[[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat=(CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink,cglContext,cglPixelFormat);
    CVDisplayLinkStart(displayLink);
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
    if (gFragment == true)
    {
        glUseProgram(gShaderProgramObjectPerFragment);
        vmath::mat4 modelMatrix=vmath::mat4::identity();
        vmath::mat4 viewMatrix=vmath::mat4::identity();
        vmath::mat4 translationMatrix=vmath::mat4::identity();
        vmath::mat4 rotationMatrix=vmath::mat4::identity();
        translationMatrix=vmath::translate(0.0f,0.0f,-3.0f);
        modelMatrix=modelMatrix*translationMatrix;
        rotationMatrix=vmath::rotate(rotateSphere, rotateSphere, rotateSphere);
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
            
            glUniform4fv(LightPositionUniformPerFragment, 1, (GLfloat*)lightPosition);
            glUniform3fv(LAUniformPerFragment, 1, (GLfloat*)lightAmbient);
            glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
            glUniform3fv(LDUniformPerFragment, 1, (GLfloat*)lightDiffused);
            glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
            glUniform3fv(LSUniformPerFragment, 1, (GLfloat*)lightSpecular);
            glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
            glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
        }
        else
        {
            glUniform1i(LKeyIsPressedUniformPerFragment, gLighting);
        }
        
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
        glDrawElements(GL_TRIANGLES, 2280,GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        glUseProgram(0);
        
    }
    else
    {
        glUseProgram(gShaderProgramObjectPerVertex);
        vmath::mat4 modelMatrix=vmath::mat4::identity();
        vmath::mat4 viewMatrix=vmath::mat4::identity();
        vmath::mat4 translationMatrix=vmath::mat4::identity();
        vmath::mat4 rotationMatrix=vmath::mat4::identity();
        translationMatrix=vmath::translate(0.0f,0.0f,-3.0f);
        modelMatrix=modelMatrix*translationMatrix;
        rotationMatrix=vmath::rotate(rotateSphere, rotateSphere, rotateSphere);
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
            glUniform1i(LKeyIsPressedUniformPerVertex,gLighting);
            
            glUniform4fv(LightPositionUniformPerVertex, 1, (GLfloat*)lightPosition);
            glUniform3fv(LAUniformPerVertex, 1, (GLfloat*)lightAmbient);
            glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
            glUniform3fv(LDUniformPerVertex, 1, (GLfloat*)lightDiffused);
            glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
            glUniform3fv(LSUniformPerVertex, 1, (GLfloat*)lightSpecular);
            glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
            glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
        }
        else
        {
            glUniform1i(LKeyIsPressedUniformPerVertex, gLighting);
        }
        
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
        glDrawElements(GL_TRIANGLES, 2280,GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        glUseProgram(0);
        
    }
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    if (gAnimate)
    {
        rotateSphere -= 0.1f;
        if (rotateSphere < 0)
        {
            rotateSphere = 360.0f;
        }
    }
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
        case 'F':
        case 'f':
            gFragment=!gFragment;
            break;
        case 'M':
        case 'm':
            [[self window]toggleFullScreen:self];
            break;
        case 'L':
        case 'l':
            if (gLighting == 0)
                gLighting = 1;
            else
                gLighting = 0;
            break;
        case 'A':
        case 'a':
            gAnimate = !gAnimate;
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
    if(vao)
    {
        glDeleteVertexArrays(1,&vao);
        vao=0;
    }
    
    if(vbo_position)
    {
        glDeleteBuffers(1,&vbo_position);
        vbo_position=0;
    }
    
    if(vbo_normal)
    {
        glDeleteBuffers(1,&vbo_normal);
        vbo_normal=0;
    }
    
    if(vbo_texture)
    {
        glDeleteBuffers(1,&vbo_texture);
        vbo_texture=0;
    }
    
    if(vbo_index)
    {
        glDeleteBuffers(1,&vbo_index);
        vbo_index=0;
    }
    
    glDetachShader(gShaderProgramObjectPerVertex,gVertexShaderObjectPerVertex);
    glDetachShader(gShaderProgramObjectPerVertex,gFragmentShaderObjectPerVertex);
    glDeleteShader(gVertexShaderObjectPerVertex);
    gVertexShaderObjectPerVertex=0;
    
    glDeleteShader(gFragmentShaderObjectPerVertex);
    gFragmentShaderObjectPerVertex=0;
    
    glDeleteProgram(gShaderProgramObjectPerVertex);
    gShaderProgramObjectPerVertex=0;
    
    glDetachShader(gShaderProgramObjectPerFragment,gVertexShaderObjectPerFragment);
    glDetachShader(gShaderProgramObjectPerFragment,gFragmentShaderObjectPerFragment);
    glDeleteShader(gVertexShaderObjectPerFragment);
    gVertexShaderObjectPerFragment=0;
    
    glDeleteShader(gFragmentShaderObjectPerFragment);
    gFragmentShaderObjectPerFragment=0;
    
    glDeleteProgram(gShaderProgramObjectPerFragment);
    gShaderProgramObjectPerFragment=0;
    
    
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


