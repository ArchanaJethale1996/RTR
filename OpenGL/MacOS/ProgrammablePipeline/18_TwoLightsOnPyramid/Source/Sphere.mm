
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
    
    GLuint gVertexShaderObjectPerVertex;
    GLuint gFragmentShaderObjectPerVertex;
    GLuint gShaderProgramObjectPerVertex;
    
    GLuint gVertexShaderObjectPerFragment;
    GLuint gFragmentShaderObjectPerFragment;
    GLuint gShaderProgramObjectPerFragment;
    
    GLuint vao_Pyramid;
    GLuint vbo_Position_Pyramid;
    GLuint vbo_Normal_Pyramid;
    
    GLuint mUniformPerVertex, vUniformPerVertex, pUniformPerVertex,KDUniformPerVertex,  KAUniformPerVertex, KSUniformPerVertex, MaterialShininessUniformPerVertex, LKeyIsPressedUniformPerVertex;
    GLuint     LDUniformOnePerVertex, LAUniformOnePerVertex, LSUniformOnePerVertex, LightPositionUniformOnePerVertex, LDUniformTwoPerVertex, LAUniformTwoPerVertex, LSUniformTwoPerVertex, LightPositionUniformTwoPerVertex;
    
    GLuint mUniformPerFragment, vUniformPerFragment, pUniformPerFragment, KDUniformPerFragment, KAUniformPerFragment, KSUniformPerFragment, MaterialShininessUniformPerFragment, LKeyIsPressedUniformPerFragment;
    GLuint     LDUniformOnePerFragment, LAUniformOnePerFragment, LSUniformOnePerFragment, LightPositionUniformOnePerFragment, LDUniformTwoPerFragment, LAUniformTwoPerFragment, LSUniformTwoPerFragment, LightPositionUniformTwoPerFragment;
    
    vmath::mat4 perspectiveProjectionMatrix;
    float rotatePyramid;
    bool gAnimate,gFragment;
    int gLighting;
    
    float lightAmbientZero[3];
    float lightDiffusedZero[3];
    float lightSpecularZero[3];
    float lightPositionZero[4];
    
    float lightAmbientOne[3];
    float lightDiffusedOne[3];
    float lightSpecularOne[3];
    float lightPositionOne[4];
    
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
    lightAmbientZero[0] = 1.0f;
    lightAmbientZero[1] = 0.0f;
    lightAmbientZero[2] = 0.0f;
    
    lightDiffusedZero[0] = 1.0f;
    lightDiffusedZero[1] = 0.0f;
    lightDiffusedZero[2] = 0.0f;
    
    lightSpecularZero[0] = 1.0f;
    lightSpecularZero[1] = 0.0f;
    lightSpecularZero[2] = 0.0f;
    
    lightPositionZero[0] = -2.0f;
    lightPositionZero[1] = 0.0f;
    lightPositionZero[2] = 0.0f;
    lightPositionZero[3] = 1.0f;
    
    //light1
    lightAmbientOne[0] = 0.0f;
    lightAmbientOne[1] = 0.0f;
    lightAmbientOne[2] = 1.0f;
    
    lightDiffusedOne[0] = 0.0f;
    lightDiffusedOne[1] = 0.0f;
    lightDiffusedOne[2] = 1.0f;
    
    lightSpecularOne[0] = 0.0f;
    lightSpecularOne[1] = 0.0f;
    lightSpecularOne[2] = 1.0f;
    
    lightPositionOne[0] = 2.0f;
    lightPositionOne[1] = 0.0f;
    lightPositionOne[2] = 0.0f;
    lightPositionOne[3] = 1.0f;
    
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
    materialShininess = 128.0f;
    gAnimate = false;
    gLighting = 0;
    gFragment=false;
    rotatePyramid = 0.0f;
    //code
    //openGL Info
    fprintf(gpFile,"OpenGL version : %s\n",glGetString(GL_VERSION));
    fprintf(gpFile,"GLSL version : %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    [[self openGLContext]makeCurrentContext];
    
    GLint swapInt=1;
    [[self openGLContext]setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    //////////////////////////////////////////////////////////////
    //Per Vertex
    //////////////////////////////////////////////////////////////
    
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
    GLchar* szInfoLog = NULL;
    
    glGetShaderiv(gVertexShaderObjectPerVertex, GL_COMPILE_STATUS, &iShaderComileStatus);
    if (iShaderComileStatus == GL_FALSE)
    {
        glGetShaderiv(gVertexShaderObjectPerVertex, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gVertexShaderObjectPerVertex,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Vertex Shader Compilation Log : %s\n",szInfoLog);
                free(szInfoLog);
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
    szInfoLog = NULL;
    
    glGetShaderiv(gFragmentShaderObjectPerVertex, GL_COMPILE_STATUS, &iShaderComileStatus);
    if (iShaderComileStatus == GL_FALSE)
    {
        glGetShaderiv(gFragmentShaderObjectPerVertex, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog= (GLchar *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gFragmentShaderObjectPerVertex,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Fragment Shader Compilation Log : %s\n",szInfoLog);
                free(szInfoLog);
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
    szInfoLog= NULL;
    
    glGetProgramiv(gShaderProgramObjectPerVertex, GL_LINK_STATUS, &iShaderLinkStatus);
    
    if (iShaderLinkStatus == GL_FALSE)
    {
        glGetProgramiv(gShaderProgramObjectPerVertex, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog= (GLchar *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(gShaderProgramObjectPerVertex,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Shader Program Link Log : %s\n",szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
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
    szInfoLog = NULL;
    
    glGetShaderiv(gVertexShaderObjectPerFragment, GL_COMPILE_STATUS, &iShaderComileStatus);
    if (iShaderComileStatus == GL_FALSE)
    {
        glGetShaderiv(gVertexShaderObjectPerFragment, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gVertexShaderObjectPerFragment,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Vertex Shader Compilation Log : %s\n",szInfoLog);
                free(szInfoLog);
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
    szInfoLog = NULL;
    
    glGetShaderiv(gFragmentShaderObjectPerFragment, GL_COMPILE_STATUS, &iShaderComileStatus);
    if (iShaderComileStatus == GL_FALSE)
    {
        glGetShaderiv(gFragmentShaderObjectPerFragment, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gFragmentShaderObjectPerFragment,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Fragment Shader Compilation Log : %s\n",szInfoLog);
                free(szInfoLog);
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
    szInfoLog = NULL;
    
    glGetProgramiv(gShaderProgramObjectPerFragment, GL_LINK_STATUS, &iShaderLinkStatus);
    if (iShaderLinkStatus == GL_FALSE)
    {
        glGetProgramiv(gShaderProgramObjectPerFragment, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (GLchar *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(gShaderProgramObjectPerFragment,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Shader Program Link Log : %s\n",szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
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
        
        vmath::mat4 modelMatrix;
        vmath::mat4 viewMatrix;
        vmath::mat4 translationMatrix;
        vmath::mat4 rotationMatrix;
        
        modelMatrix = vmath::mat4::identity();
        viewMatrix =  vmath::mat4::identity();
        translationMatrix = vmath::mat4::identity();
        rotationMatrix = vmath::mat4::identity();
        
        translationMatrix =  vmath::translate(0.0f, 0.0f, -5.0f);
        modelMatrix = modelMatrix*translationMatrix;
        rotationMatrix =  vmath::rotate(rotatePyramid, 0.0f, 1.0f, 0.0f);
        
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
            
            glUniform4fv(LightPositionUniformOnePerFragment, 1, (GLfloat*)lightPositionZero);
            glUniform3fv(LAUniformOnePerFragment, 1, (GLfloat*)lightAmbientZero);
            glUniform3fv(LDUniformOnePerFragment, 1, (GLfloat*)lightDiffusedZero);
            glUniform3fv(LSUniformOnePerFragment, 1, (GLfloat*)lightSpecularZero);
            
            glUniform4fv(LightPositionUniformTwoPerFragment, 1, (GLfloat*)lightPositionOne);
            glUniform3fv(LAUniformTwoPerFragment, 1, (GLfloat*)lightAmbientOne);
            glUniform3fv(LDUniformTwoPerFragment, 1, (GLfloat*)lightDiffusedOne);
            glUniform3fv(LSUniformTwoPerFragment, 1, (GLfloat*)lightSpecularOne);
            
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
    else
    {
        glUseProgram(gShaderProgramObjectPerVertex);
        
        vmath::mat4 modelMatrix;
        vmath::mat4 viewMatrix;
        vmath::mat4 translationMatrix;
        vmath::mat4 rotationMatrix;
        
        modelMatrix =  vmath::mat4::identity();
        viewMatrix =  vmath::mat4::identity();
        translationMatrix =  vmath::mat4::identity();
        rotationMatrix =  vmath::mat4::identity();
        
        translationMatrix =  vmath::translate(0.0f, 0.0f, -5.0f);
        modelMatrix = modelMatrix*translationMatrix;
        rotationMatrix =  vmath::rotate(rotatePyramid, 0.0f, 1.0f, 0.0f);
        
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
            
            glUniform4fv(LightPositionUniformOnePerVertex, 1, (GLfloat*)lightPositionZero);
            glUniform3fv(LAUniformOnePerVertex, 1, (GLfloat*)lightAmbientZero);
            glUniform3fv(LDUniformOnePerVertex, 1, (GLfloat*)lightDiffusedZero);
            glUniform3fv(LSUniformOnePerVertex, 1, (GLfloat*)lightSpecularZero);
            
            glUniform4fv(LightPositionUniformTwoPerVertex, 1, (GLfloat*)lightPositionOne);
            glUniform3fv(LAUniformTwoPerVertex, 1, (GLfloat*)lightAmbientOne);
            glUniform3fv(LDUniformTwoPerVertex, 1, (GLfloat*)lightDiffusedOne);
            glUniform3fv(LSUniformTwoPerVertex, 1, (GLfloat*)lightSpecularOne);
            
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
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    if (gAnimate)
    {
        rotatePyramid -= 0.1f;
        if (rotatePyramid < 0)
        {
            rotatePyramid = 360.0f;
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


