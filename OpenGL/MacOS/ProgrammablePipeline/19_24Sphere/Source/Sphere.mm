
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
-(void)draw24SpherePerFragment;
-(void)draw24SpherePerVertex;
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
    GLuint mUniformPerVertex, vUniformPerVertex, pUniformPerVertex, KDUniformPerVertex, KAUniformPerVertex, KSUniformPerVertex, MaterialShininessUniformPerVertex, LKeyIsPressedUniformPerVertex;
    GLuint LDUniformZeroPerVertex, LAUniformZeroPerVertex, LSUniformZeroPerVertex, LightPositionUniformZeroPerVertex;
    
    GLuint mUniformPerFragment, vUniformPerFragment, pUniformPerFragment, KDUniformPerFragment, KAUniformPerFragment, KSUniformPerFragment, MaterialShininessUniformPerFragment, LKeyIsPressedUniformPerFragment;
    GLuint LDUniformZeroPerFragment, LAUniformZeroPerFragment, LSUniformZeroPerFragment, LightPositionUniformZeroPerFragment;
    
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
    
    GLfloat angleOfXRotation;
    GLfloat angleOfYRotation;
    GLfloat angleOfZRotation;
    GLint KeyPressed;
    int gWidth;
    int gHeight;
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
    lightPosition[0] = 0.0f;
    lightPosition[1] = 3.0f;
    lightPosition[2] = 3.0f;
    lightPosition[3] = 1.0f;
    
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
    "uniform vec3 u_LdZero;" \
    "uniform vec3 u_Kd;" \
    "uniform vec3 u_LsZero;" \
    "uniform vec3 u_Ks;" \
    "uniform vec3 u_LaZero;" \
    "uniform vec3 u_Ka;" \
    "uniform vec4 u_Light_PositionZero;" \
    "uniform float u_MaterialShininess;" \
    "uniform int u_LKeyIsPressed;" \
    "out vec3 phong_ads_light;" \
    "void main(void)" \
    "{" \
    "if(u_LKeyIsPressed==1)" \
    "{" \
    "vec4 eye_coordinates=u_v_matrix*u_m_matrix*vPosition;" \
    
    "vec3 tNorm=normalize(mat3(u_v_matrix*u_m_matrix)*vNormal);" \
    "vec3 lightDirectionZero=normalize(vec3(u_Light_PositionZero-eye_coordinates));" \
    "float tndotldZero=max(dot(lightDirectionZero,tNorm),0.0);" \
    "vec3 ReflectionVectorZero=reflect(-lightDirectionZero,tNorm);" \
    "vec3 viewerVector=normalize(vec3(-eye_coordinates.xyz));" \
    "vec3 ambientZero=u_LaZero*u_Ka;" \
    "vec3 diffusedZero=u_LdZero*u_Kd*tndotldZero;" \
    "vec3 specularZero=u_LsZero*u_Ks*pow(max(dot(ReflectionVectorZero,viewerVector),0.0),u_MaterialShininess);" \
    "phong_ads_light = ambientZero+diffusedZero+specularZero;" \
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
    LDUniformZeroPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LdZero");
    LAUniformZeroPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LaZero");
    LSUniformZeroPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_LsZero");
    LightPositionUniformZeroPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Light_PositionZero");
    
    KDUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Kd");
    KAUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ka");
    KSUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_Ks");
    MaterialShininessUniformPerVertex = glGetUniformLocation(gShaderProgramObjectPerVertex, "u_MaterialShininess");
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
    "uniform vec3 u_LdZero;" \
    "uniform vec3 u_Kd;" \
    "uniform vec3 u_LsZero;" \
    "uniform vec3 u_Ks;" \
    "uniform vec3 u_LaZero;" \
    "uniform vec3 u_Ka;" \
    "uniform vec4 u_Light_PositionZero;" \
    "uniform float u_MaterialShininess;" \
    "uniform int u_LKeyIsPressed;" \
    "out vec4 FragColor;" \
    "void main(void)" \
    "{" \
    "if(u_LKeyIsPressed==1)" \
    "{" \
    "vec3 tNorm=normalize(tNormVertexShader);"
    "vec3 lightDirectionZero=normalize(vec3(u_Light_PositionZero-eye_coordinatesVertexShader));" \
    "float tndotldZero=max(dot(lightDirectionZero,tNorm),0.0);" \
    "vec3 ReflectionVectorZero=reflect(-lightDirectionZero,tNorm);" \
    "vec3 viewerVector=normalize(vec3(-eye_coordinatesVertexShader.xyz));" \
    "vec3 ambientZero=u_LaZero*u_Ka;" \
    "vec3 diffusedZero=u_LdZero*u_Kd*tndotldZero;" \
    "vec3 specularZero=u_LsZero*u_Ks*pow(max(dot(ReflectionVectorZero,viewerVector),0.0),u_MaterialShininess);" \
    "vec3 phong_ads_light = ambientZero+diffusedZero+specularZero;" \
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
    LDUniformZeroPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LdZero");
    KDUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Kd");
    LAUniformZeroPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LaZero");
    KAUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ka");
    LSUniformZeroPerFragment= glGetUniformLocation(gShaderProgramObjectPerFragment, "u_LsZero");
    KSUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Ks");
    MaterialShininessUniformPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_MaterialShininess");
    LightPositionUniformZeroPerFragment = glGetUniformLocation(gShaderProgramObjectPerFragment, "u_Light_PositionZero");
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
    gWidth=width;
    gHeight=height;
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
-(void)draw24SpherePerFragment
{
    GLfloat materialAmbient[4];
    GLfloat materialDiffused[4];
    GLfloat materialSpecular[4];
    GLfloat materialShininess;
    
    vmath::mat4 modelMatrix;
    vmath::mat4 viewMatrix;
    vmath::mat4 translationMatrix;
    vmath::mat4 scaleMatrix;
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix= vmath::mat4::identity();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //1st sphere 1st col emrald
    materialAmbient[0] = 0.0215f;
    materialAmbient[1] = 0.1745f;
    materialAmbient[2] = 0.0215f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.07568f;
    materialDiffused[1] = 0.61424f;
    materialDiffused[2] = 0.07568f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.633f;
    materialSpecular[1] = 0.727811f;
    materialSpecular[2] = 0.633f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.6f * 128;
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 0, (gHeight / 4) * 3,gWidth/6,gHeight/6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    
    
    ////2nd sphere on  1st col,jade
    
    materialAmbient[0] = 0.135f;
    materialAmbient[1] = 0.2225f;
    materialAmbient[2] = 0.1575f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.54f;
    materialDiffused[1] = 0.89f;
    materialDiffused[2] = 0.63f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.316228f;
    materialSpecular[1] = 0.316228f;
    materialSpecular[2] = 0.316228f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.1f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 1, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd sphere on  1st col,obsidian
    materialAmbient[0] = 0.05375f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.06625f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.18275f;
    materialDiffused[1] = 0.17f;
    materialDiffused[2] = 0.22525f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.332741f;
    materialSpecular[1] = 0.328634f;
    materialSpecular[2] = 0.346435f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.3f * 128;
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth/ 6) * 2, (gHeight / 4) * 3,  gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////4th sphere on  1st col,pearl
    
    materialAmbient[0] = 0.25;
    materialAmbient[1] = 0.20725f;
    materialAmbient[2] = 0.20725f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 1.0f;
    materialDiffused[1] = 0.829f;
    materialDiffused[2] = 0.829f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.296648f;
    materialSpecular[1] = 0.296648f;
    materialSpecular[2] = 0.296648f;
    materialSpecular[3] = 1.0f;
    
    
    materialShininess = 0.88f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 3, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////5th sphere on  1st col,ruby
    
    materialAmbient[0] = 0.1745f;
    materialAmbient[1] = 0.01175f;
    materialAmbient[2] = 0.01175f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.61424f;
    materialDiffused[1] = 0.04136f;
    materialDiffused[2] = 0.04136f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.727811f;
    materialSpecular[1] = 0.626959f;
    materialSpecular[2] = 0.626959f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.6f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 4, (gHeight / 4) * 3,gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    ////6th sphere on  1st col,turquoise
    
    materialAmbient[0] = 0.1f;
    materialAmbient[1] = 0.18725f;
    materialAmbient[2] = 0.1745f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.396f;
    materialDiffused[1] = 0.074151f;
    materialDiffused[2] = 0.69102f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.297254f;
    materialSpecular[1] = 0.30829f;
    materialSpecular[2] = 0.306678f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.1f * 128;
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 5, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    
    ////2nd Col
    
    //1st sphere 2nd col brass
    materialAmbient[0] = 0.329412f;
    materialAmbient[1] = 0.223529f;
    materialAmbient[2] = 0.027451f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.780392f;
    materialDiffused[1] = 0.568627f;
    materialDiffused[2] = 0.113725f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.992157f;
    materialSpecular[1] = 0.941176f;
    materialSpecular[2] = 0.807843f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.21794872f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 0, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////2nd sphere on  2nd col,bronze
    
    materialAmbient[0] = 0.2125f;
    materialAmbient[1] = 0.1275f;
    materialAmbient[2] = 0.054f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.714f;
    materialDiffused[1] = 0.4284f;
    materialDiffused[2] = 0.18144f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.393548f;
    materialSpecular[1] = 0.271906f;
    materialSpecular[2] = 0.166721f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.2f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 1, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd sphere on  2nd col,chrome
    
    materialAmbient[0] = 0.25f;
    materialAmbient[1] = 0.25f;
    materialAmbient[2] = 0.25f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.4f;
    materialDiffused[1] = 0.4f;
    materialDiffused[2] = 0.4f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.774597f;
    materialSpecular[1] = 0.774597f;
    materialSpecular[2] = 0.774597f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.6f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 2, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////4th sphere on  2nd col,copper
    
    materialAmbient[0] = 0.19125f;
    materialAmbient[1] = 0.0735f;
    materialAmbient[2] = 0.0225f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.7038f;
    materialDiffused[1] = 0.27048f;
    materialDiffused[2] = 0.0828f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.256777f;
    materialSpecular[1] = 0.137622f;
    materialSpecular[2] = 0.086014f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.1f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 3, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////5th sphere on  2nd col,gold
    
    materialAmbient[0] = 0.2472f;
    materialAmbient[1] = 0.1995f;
    materialAmbient[2] = 0.0745f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.75164f;
    materialDiffused[1] = 0.60648f;
    materialDiffused[2] = 0.22648f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.628281f;
    materialSpecular[1] = 0.555802f;
    materialSpecular[2] = 0.366065f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.4f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 4, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////6th sphere on  2nd col,silver
    
    materialAmbient[0] = 0.19225f;
    materialAmbient[1] = 0.19225f;
    materialAmbient[2] = 0.19225f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5074f;
    materialDiffused[1] = 0.5074f;
    materialDiffused[2] = 0.5074f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.508273f;
    materialSpecular[1] = 0.508273f;
    materialSpecular[2] = 0.508273f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.4f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 5, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd col
    //1st sphere 3rd col black
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.0f;
    materialDiffused[1] = 0.0f;
    materialDiffused[2] = 0.0f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.5f;
    materialSpecular[1] = 0.5;
    materialSpecular[2] = 0.5f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 0, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    ////2nd sphere on  3rd col,cyan
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.1f;
    materialAmbient[2] = 0.06f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.0f;
    materialDiffused[1] = 0.50980392f;
    materialDiffused[2] = 0.5098392f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.50196078f;
    materialSpecular[1] = 0.50196078f;
    materialSpecular[2] = 0.50196078f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 1, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd sphere on  3rd col,green
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.1f;
    materialDiffused[1] = 0.35f;
    materialDiffused[2] = 0.1f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.45f;
    materialSpecular[1] = 0.55f;
    materialSpecular[2] = 0.45f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 2, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    //
    ////4th sphere on  3rd col,red
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.0f;
    materialDiffused[2] = 0.0f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.7f;
    materialSpecular[1] = 0.6f;
    materialSpecular[2] = 0.6f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 3, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    ////5th sphere on  3rd col,white
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.55f;
    materialDiffused[1] = 0.55f;
    materialDiffused[2] = 0.55f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.70f;
    materialSpecular[1] = 0.70f;
    materialSpecular[2] = 0.70f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 4, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////6th sphere on  3rd col,yellow plastic
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.0f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.60f;
    materialSpecular[1] = 0.60f;
    materialSpecular[2] = 0.50f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 5, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    ////4th col
    ////1st sphere 3rd col black
    materialAmbient[0] = 0.02f;
    materialAmbient[1] = 0.02f;
    materialAmbient[2] = 0.02f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.01f;
    materialDiffused[1] = 0.01f;
    materialDiffused[2] = 0.01f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.4f;
    materialSpecular[1] = 0.4f;
    materialSpecular[2] = 0.4f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.78125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 0, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////2nd sphere on  3rd col,cyan
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.05f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.4f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.5f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.04f;
    materialSpecular[1] = 0.7f;
    materialSpecular[2] = 0.7f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 1, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd sphere on  3rd col,green
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.4f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.4f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.04f;
    materialSpecular[1] = 0.7f;
    materialSpecular[2] = 0.04f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 2, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    //////4th sphere on  3rd col,red
    
    materialAmbient[0] = 0.05f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.4f;
    materialDiffused[2] = 0.4f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.7f;
    materialSpecular[1] = 0.04f;
    materialSpecular[2] = 0.04f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 3, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    //////5th sphere on  3rd col,white
    
    materialAmbient[0] = 0.05f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.05f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.5f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.70f;
    materialSpecular[1] = 0.70f;
    materialSpecular[2] = 0.70f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    
    glViewport((gWidth / 6) * 4, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////6th sphere on  3rd col,yellow plastic
    
    materialAmbient[0] = 0.05f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.4f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.70f;
    materialSpecular[1] = 0.70f;
    materialSpecular[2] = 0.04f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerFragment, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerFragment, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerFragment, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerFragment, materialShininess);
    
    glViewport((gWidth / 6) * 5, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
}
-(void)draw24SpherePerVertex
{
    GLfloat materialAmbient[4];
    GLfloat materialDiffused[4];
    GLfloat materialSpecular[4];
    GLfloat materialShininess;
    
    vmath::mat4 modelMatrix;
    vmath::mat4 viewMatrix;
    vmath::mat4 translationMatrix;
    vmath::mat4 scaleMatrix;
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //1st sphere 1st col emrald
    materialAmbient[0] = 0.0215f;
    materialAmbient[1] = 0.1745f;
    materialAmbient[2] = 0.0215f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.07568f;
    materialDiffused[1] = 0.61424f;
    materialDiffused[2] = 0.07568f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.633f;
    materialSpecular[1] = 0.727811f;
    materialSpecular[2] = 0.633f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.6f * 128;
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 0, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    
    
    ////2nd sphere on  1st col,jade
    
    materialAmbient[0] = 0.135f;
    materialAmbient[1] = 0.2225f;
    materialAmbient[2] = 0.1575f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.54f;
    materialDiffused[1] = 0.89f;
    materialDiffused[2] = 0.63f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.316228f;
    materialSpecular[1] = 0.316228f;
    materialSpecular[2] = 0.316228f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.1f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 1, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd sphere on  1st col,obsidian
    materialAmbient[0] = 0.05375f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.06625f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.18275f;
    materialDiffused[1] = 0.17f;
    materialDiffused[2] = 0.22525f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.332741f;
    materialSpecular[1] = 0.328634f;
    materialSpecular[2] = 0.346435f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.3f * 128;
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 2, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////4th sphere on  1st col,pearl
    
    materialAmbient[0] = 0.25;
    materialAmbient[1] = 0.20725f;
    materialAmbient[2] = 0.20725f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 1.0f;
    materialDiffused[1] = 0.829f;
    materialDiffused[2] = 0.829f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.296648f;
    materialSpecular[1] = 0.296648f;
    materialSpecular[2] = 0.296648f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.88f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 3, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////5th sphere on  1st col,ruby
    
    materialAmbient[0] = 0.1745f;
    materialAmbient[1] = 0.01175f;
    materialAmbient[2] = 0.01175f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.61424f;
    materialDiffused[1] = 0.04136f;
    materialDiffused[2] = 0.04136f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.727811f;
    materialSpecular[1] = 0.626959f;
    materialSpecular[2] = 0.626959f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.6f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 4, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    ////6th sphere on  1st col,turquoise
    
    materialAmbient[0] = 0.1f;
    materialAmbient[1] = 0.18725f;
    materialAmbient[2] = 0.1745f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.396f;
    materialDiffused[1] = 0.074151f;
    materialDiffused[2] = 0.69102f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.297254f;
    materialSpecular[1] = 0.30829f;
    materialSpecular[2] = 0.306678f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.1f * 128;
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 5, (gHeight / 4) * 3, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    
    ////2nd Col
    
    //1st sphere 2nd col brass
    materialAmbient[0] = 0.329412f;
    materialAmbient[1] = 0.223529f;
    materialAmbient[2] = 0.027451f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.780392f;
    materialDiffused[1] = 0.568627f;
    materialDiffused[2] = 0.113725f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.992157f;
    materialSpecular[1] = 0.941176f;
    materialSpecular[2] = 0.807843f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.21794872f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 0, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////2nd sphere on  2nd col,bronze
    
    materialAmbient[0] = 0.2125f;
    materialAmbient[1] = 0.1275f;
    materialAmbient[2] = 0.054f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.714f;
    materialDiffused[1] = 0.4284f;
    materialDiffused[2] = 0.18144f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.393548f;
    materialSpecular[1] = 0.271906f;
    materialSpecular[2] = 0.166721f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.2f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 1, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd sphere on  2nd col,chrome
    
    materialAmbient[0] = 0.25f;
    materialAmbient[1] = 0.25f;
    materialAmbient[2] = 0.25f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.4f;
    materialDiffused[1] = 0.4f;
    materialDiffused[2] = 0.4f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.774597f;
    materialSpecular[1] = 0.774597f;
    materialSpecular[2] = 0.774597f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.6f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 2, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////4th sphere on  2nd col,copper
    
    materialAmbient[0] = 0.19125f;
    materialAmbient[1] = 0.0735f;
    materialAmbient[2] = 0.0225f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.7038f;
    materialDiffused[1] = 0.27048f;
    materialDiffused[2] = 0.0828f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.256777f;
    materialSpecular[1] = 0.137622f;
    materialSpecular[2] = 0.086014f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.1f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 3, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////5th sphere on  2nd col,gold
    
    materialAmbient[0] = 0.2472f;
    materialAmbient[1] = 0.1995f;
    materialAmbient[2] = 0.0745f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.75164f;
    materialDiffused[1] = 0.60648f;
    materialDiffused[2] = 0.22648f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.628281f;
    materialSpecular[1] = 0.555802f;
    materialSpecular[2] = 0.366065f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.4f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 4, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////6th sphere on  2nd col,silver
    
    materialAmbient[0] = 0.19225f;
    materialAmbient[1] = 0.19225f;
    materialAmbient[2] = 0.19225f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5074f;
    materialDiffused[1] = 0.5074f;
    materialDiffused[2] = 0.5074f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.508273f;
    materialSpecular[1] = 0.508273f;
    materialSpecular[2] = 0.508273f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.4f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 5, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd col
    //1st sphere 3rd col black
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.0f;
    materialDiffused[1] = 0.0f;
    materialDiffused[2] = 0.0f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.5f;
    materialSpecular[1] = 0.5;
    materialSpecular[2] = 0.5f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 0, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    ////2nd sphere on  3rd col,cyan
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.1f;
    materialAmbient[2] = 0.06f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.0f;
    materialDiffused[1] = 0.50980392f;
    materialDiffused[2] = 0.5098392f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.50196078f;
    materialSpecular[1] = 0.50196078f;
    materialSpecular[2] = 0.50196078f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 1, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd sphere on  3rd col,green
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.1f;
    materialDiffused[1] = 0.35f;
    materialDiffused[2] = 0.1f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.45f;
    materialSpecular[1] = 0.55f;
    materialSpecular[2] = 0.45f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 2, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    //
    ////4th sphere on  3rd col,red
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.0f;
    materialDiffused[2] = 0.0f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.7f;
    materialSpecular[1] = 0.6f;
    materialSpecular[2] = 0.6f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 3, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    ////5th sphere on  3rd col,white
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.55f;
    materialDiffused[1] = 0.55f;
    materialDiffused[2] = 0.55f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.70f;
    materialSpecular[1] = 0.70f;
    materialSpecular[2] = 0.70f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 4, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////6th sphere on  3rd col,yellow plastic
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.0f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.60f;
    materialSpecular[1] = 0.60f;
    materialSpecular[2] = 0.50f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.25f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 5, (gHeight / 4) * 1, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    ////4th col
    ////1st sphere 3rd col black
    materialAmbient[0] = 0.02f;
    materialAmbient[1] = 0.02f;
    materialAmbient[2] = 0.02f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.01f;
    materialDiffused[1] = 0.01f;
    materialDiffused[2] = 0.01f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.4f;
    materialSpecular[1] = 0.4f;
    materialSpecular[2] = 0.4f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.78125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 0, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////2nd sphere on  3rd col,cyan
    
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.05f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.4f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.5f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.04f;
    materialSpecular[1] = 0.7f;
    materialSpecular[2] = 0.7f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 1, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////3rd sphere on  3rd col,green
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.4f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.4f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.04f;
    materialSpecular[1] = 0.7f;
    materialSpecular[2] = 0.04f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 2, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    //////4th sphere on  3rd col,red
    
    materialAmbient[0] = 0.05f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.4f;
    materialDiffused[2] = 0.4f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.7f;
    materialSpecular[1] = 0.04f;
    materialSpecular[2] = 0.04f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 3, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    //////5th sphere on  3rd col,white
    
    materialAmbient[0] = 0.05f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.05f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.5f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.70f;
    materialSpecular[1] = 0.70f;
    materialSpecular[2] = 0.70f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    
    glViewport((gWidth / 6) * 4, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    
    ////6th sphere on  3rd col,yellow plastic
    
    materialAmbient[0] = 0.05f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;
    
    materialDiffused[0] = 0.5f;
    materialDiffused[1] = 0.5f;
    materialDiffused[2] = 0.4f;
    materialDiffused[3] = 1.0f;
    
    materialSpecular[0] = 0.70f;
    materialSpecular[1] = 0.70f;
    materialSpecular[2] = 0.04f;
    materialSpecular[3] = 1.0f;
    
    materialShininess = 0.078125f * 128;
    
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    scaleMatrix = vmath::mat4::identity();
    
    glUniform3fv(KAUniformPerVertex, 1, (GLfloat*)materialAmbient);
    glUniform3fv(KDUniformPerVertex, 1, (GLfloat*)materialDiffused);
    glUniform3fv(KSUniformPerVertex, 1, (GLfloat*)materialSpecular);
    glUniform1f(MaterialShininessUniformPerVertex, materialShininess);
    
    glViewport((gWidth / 6) * 5, (gHeight / 4) * 0, gWidth / 6, gHeight / 6);
    translationMatrix = vmath::translate(0.0f, 0.0f, -1.0f);
    
    modelMatrix = modelMatrix*translationMatrix;
    scaleMatrix = vmath::scale(0.7f, 0.7f, 0.7f);
    modelMatrix = modelMatrix*scaleMatrix;
    
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
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_index);
    glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
}

-(void)drawView
{
    [[self openGLContext]makeCurrentContext];
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if (gFragment == false)
    {
        glUseProgram(gShaderProgramObjectPerVertex);
        if (gLighting == 1)
        {
            glUniform1i(LKeyIsPressedUniformPerVertex,
                        gLighting);
            
            if (KeyPressed == 1)
            {
                lightPosition[0] = 0.0f;
                lightPosition[1] = sinf(rotateSphere) * 100.0f - 3.0f;
                lightPosition[2] = cosf(rotateSphere) * 100.0f - 3.0f;
            }
            
            if (KeyPressed == 2)
            {
                lightPosition[0] = sinf(rotateSphere) * 100.0f - 3.0f;
                lightPosition[1] = 0.0f;
                lightPosition[2] = cosf(rotateSphere) * 100.0f - 3.0f;
            }
            
            if (KeyPressed == 3)
            {
                lightPosition[0] = sinf(rotateSphere) * 100.0f - 3.0f;
                lightPosition[1] = cosf(rotateSphere) * 100.0f - 3.0f;
                lightPosition[2] = 0.0f;
            }
            glUniform4fv(LightPositionUniformZeroPerVertex, 1, (GLfloat*)lightPosition);
            glUniform3fv(LAUniformZeroPerVertex, 1, (GLfloat*)lightAmbient);
            glUniform3fv(LDUniformZeroPerVertex, 1, (GLfloat*)lightDiffused);
            glUniform3fv(LSUniformZeroPerVertex, 1, (GLfloat*)lightSpecular);
            
            [self draw24SpherePerVertex];
        }
        else
        {
            
            vmath::mat4 modelMatrix;
            vmath::mat4 viewMatrix;
            vmath::mat4 translationMatrix;
            
            modelMatrix = vmath::mat4::identity();
            viewMatrix = vmath::mat4::identity();
            translationMatrix = vmath::mat4::identity();
            
            translationMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
            modelMatrix = modelMatrix*translationMatrix;
            
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
            
            glUniform1i(LKeyIsPressedUniformPerVertex, gLighting);
            [self draw24SpherePerVertex];
        }
        glUseProgram(0);
        
    }
    else
    {
        glUseProgram(gShaderProgramObjectPerFragment);
        if (gLighting == 1)
        {
            glUniform1i(LKeyIsPressedUniformPerFragment,
                        gLighting);
            
            if (KeyPressed == 1)
            {
                lightPosition[0] = 0.0f;
                lightPosition[1] = sinf(rotateSphere) * 100.0f - 3.0f;
                lightPosition[2] = cosf(rotateSphere) * 100.0f - 3.0f;
            }
            
            if (KeyPressed == 2)
            {
                lightPosition[0] = sinf(rotateSphere) * 100.0f - 3.0f;
                lightPosition[1] = 0.0f;
                lightPosition[2] = cosf(rotateSphere) * 100.0f - 3.0f;
            }
            
            if (KeyPressed == 3)
            {
                lightPosition[0] = sinf(rotateSphere) * 100.0f - 3.0f;
                lightPosition[1] = cosf(rotateSphere) * 100.0f - 3.0f;
                lightPosition[2] = 0.0f;
            }
            glUniform4fv(LightPositionUniformZeroPerFragment, 1, (GLfloat*)lightPosition);
            glUniform3fv(LAUniformZeroPerFragment, 1, (GLfloat*)lightAmbient);
            glUniform3fv(LDUniformZeroPerFragment, 1, (GLfloat*)lightDiffused);
            glUniform3fv(LSUniformZeroPerFragment, 1, (GLfloat*)lightSpecular);
            
            [self draw24SpherePerFragment];
        }
        else
        {
            
            vmath::mat4 modelMatrix;
            vmath::mat4 viewMatrix;
            vmath::mat4 translationMatrix;
            
            modelMatrix = vmath::mat4::identity();
            viewMatrix = vmath::mat4::identity();
            translationMatrix = vmath::mat4::identity();
            
            translationMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
            modelMatrix = modelMatrix*translationMatrix;
            
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
            
            glUniform1i(LKeyIsPressedUniformPerFragment, gLighting);
            [self draw24SpherePerFragment];
        }
        glUseProgram(0);
        
    }
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    //if (gAnimate)
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



