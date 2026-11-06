#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>

#include"vmath.h"
using namespace vmath;

// MACROS
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Global variable declaration
FILE *gpFile = NULL;

enum 
{
    SSG_ATTRIBUTE_POSITION = 1,
    SSG_ATTRIBUTE_NORMAL
};

// Global function declarations
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*, CVOptionFlags, CVOptionFlags*, void*);

@interface AppDelegate:NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface GLView:NSOpenGLView
@end

// Main function
int main(int argc, char *argv[])
{
    // Code
    // Create auto release pool for memory management 
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    // Create global shared NSApp object
    NSApp = [NSApplication sharedApplication];

    // Set its deligate to our own custom app deligate 
    [NSApp setDelegate:[[AppDelegate alloc]init]];

    // Start NSApp run loop
    [NSApp run];

    // Let auto release pool release all pending objects in our application
    [pool release];

    return(0);
}

// Implementation of our custom AppDelegate
@implementation AppDelegate
{
    NSWindow *window;
    GLView *glView;
}

-(void)applicationDidFinishLaunching:(NSNotification *) notification
{
    // Code
    // Create log file
    NSBundle *appBundle = [NSBundle mainBundle];
    NSString *appDirPath = [appBundle bundlePath];
    NSString *parentDirPath = [appDirPath stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/Log.txt",parentDirPath];
    const char *pszLogFileNameWithPath = [logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];

    gpFile = fopen(pszLogFileNameWithPath, "w");
    if(gpFile == NULL)
    {
        NSLog(@"Log file cannot be created\n");
        [self release];
        [NSApp terminate:self];
    }
    else
    {
        fprintf(gpFile,"Program started successfully\n");
    }

    // Declare rectangle for our border window
    NSRect win_rect = NSMakeRect(0.0, 0.0, WIN_WIDTH, WIN_HEIGHT);

    // Create the window
    window = [[NSWindow alloc]initWithContentRect: win_rect
                                        styleMask:(NSWindowStyleMaskTitled |     
                                        NSWindowStyleMaskClosable |
                                        NSWindowStyleMaskMiniaturizable |
                                        NSWindowStyleMaskResizable)
                                        backing:NSBackingStoreBuffered
                                        defer:NO];
    
    // Give title to window
    [window setTitle:@"SSG:Cocoa Window"];

    // Centre the window
    [window center];

    // Painting background window to black
    [window setBackgroundColor:[NSColor blackColor]];

    // Create the custom view
    glView = [[GLView alloc]initWithFrame:win_rect];

    // Set this newly created custom view as view of our newly created custom window
    [window setContentView:glView];
    [window setDelegate:self];

    // Actualy show the window, give it keyboard focus and make it top on Z-order
    [window makeKeyAndOrderFront:self];
}

// Terminate the application
-(void)applicationWillTerminate:(NSNotification *)notification
{
    // Code
    // Log file closing code
    if(gpFile)
    {
        fprintf(gpFile,"Program ended successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

-(void)windowWillClose:(NSNotification *)notification
{
  // Code
  [NSApp terminate:self];  
}

// Free memory
-(void)dealloc
{
    [super dealloc];
    [glView release];
    [window release];
}
@end

// Implementation of custom view interface
@implementation GLView
{
    CVDisplayLinkRef displayLink;

    uint shaderProgramObject;
    uint vao_pyramid; 
    uint vbo_pyramid;
    uint vbo_normal;
    float pyramid_angle;

    BOOL bLightingEnabled;
    BOOL bAnimationEnabled;

    uint light_Diffused_Uniform[2];
    uint light_Ambient_Uniform[2];
    uint light_Position_Uniform[2];
    uint light_Specular_Uniform[2];

    uint material_Diffused_Uniform;
    uint material_Ambient_Uniform;
    uint material_Specular_Uniform;
    uint material_Shininess_Uniform;

    uint modelMatrixUniform;
    uint projectionMatrixUniform;
    uint viewMatrixUniform;
    uint keyPressedUniform;

    float lightAmbient_1[4];
    float lightSpecular_1[4];
    float lightDiffused_1[4];
    float lightPosition_1[4];

    float lightAmbient_2[4];
    float lightSpecular_2[4];
    float lightDiffused_2[4];
    float lightPosition_2[4];

    float materialDiffused[4];   
    float materialAmbient[4];
    float materialSpecular[4]; 
    float materialShininess;

    mat4 perspectiveProjectionMatrix;
}

-(id)initWithFrame:(NSRect)frame
{
    bLightingEnabled = NO;
    bAnimationEnabled = NO;
    // Code
    self = [super initWithFrame:frame];
    
    if(self)
    {
        // Declare OpenGL attributes
        NSOpenGLPixelFormatAttribute attributes[] =
        {
            NSOpenGLPFAOpenGLProfile,NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAScreenMask,
            CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFADoubleBuffer,
            0
        };
        
        // Create Pixel Format using above attributes
        NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc]initWithAttributes:attributes]autorelease];
        if(pixelFormat == nil)
        {
            fprintf(gpFile,"Creating Pixel Format Failed\n");
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
        }

        // Create OpenGL Context using above pixel format
        NSOpenGLContext *glContext = [[[NSOpenGLContext alloc]initWithFormat:pixelFormat shareContext:nil]autorelease];
        if(glContext == nil)
        {
            fprintf(gpFile,"Create Context Failed\n");
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
        }

        // Set Pixel Format
        [self setPixelFormat:pixelFormat];

        // Set OpenGL Context
        [self setOpenGLContext:glContext];
    }

    return(self);
}

-(void)prepareOpenGL
{
    [super prepareOpenGL];
    [[self openGLContext]makeCurrentContext];

    int swapInterval = 1;
    [[self openGLContext]setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
    int result = [self initialize];

    // Create and start diplay link
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];
    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext]CGLContextObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
}

-(void)reshape
{
    // Code
    [super reshape];
    [[self openGLContext]makeCurrentContext];
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    NSRect rect = [self bounds];
    int width = rect.size.width;
    int height = rect.size.height;
    [self resize:width :height];
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

// Paint (WM_PAINT)
-(void)drawRect:(NSRect)dirtyRect
{
    // Code
    // Call Rendering function here too to avoid flickering
    [self drawView];
}

// Rendering Function
-(void)drawView
{
    // Code
    [[self openGLContext]makeCurrentContext];
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);

    // Call Display
    [self display];

    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

// Function called per frame
-(CVReturn)getFrameForTime:(const CVTimeStamp *)outputTime
{
    // Code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // Call Rendering Function here too
    [self drawView];
    if(bAnimationEnabled == TRUE)
    {
        [self myUpdate];
    }
    
    [pool release];

    return(kCVReturnSuccess);
}


-(int)initialize
{
    // Code
    [self printGLInfo];

    const GLchar* vertexShaderSourceCode =
    "   #version 410 core                                                                                                                                           \n" \
    "   in vec4 aPosition;                                                                                                                                          \n" \
    "   in vec3 aNormal;                                                                                                                                            \n" \
    "   uniform mat4 uViewMatrix;                                                                                                                                   \n" \
    "   uniform mat4 uModelMatrix;                                                                                                                                  \n" \
    "   uniform mat4 uProjectionMatrix;                                                                                                                             \n" \
    "   uniform vec3 uLight_Ambient[2];                                                                                                                             \n" \
    "   uniform vec3 uLight_Specular[2];                                                                                                                            \n" \
    "   uniform vec3 uLight_Diffused[2];                                                                                                                            \n" \
    "   uniform vec4 uLight_Position[2];                                                                                                                            \n" \
    "   uniform vec3 uMaterial_Diffused;                                                                                                                            \n" \
    "   uniform vec3 uMaterial_Ambient;                                                                                                                             \n" \
    "   uniform vec3 uMaterial_Specular;                                                                                                                            \n" \
    "   uniform float uMaterial_Shininess;                                                                                                                          \n" \
    "   uniform int uKeyPressed;                                                                                                                                    \n" \
    "   out vec3 oPhong_ADS_Light = vec3(0.0);                                                                                                                      \n" \
    "   void main(void)                                                                                                                                             \n" \
    "   {                                                                                                                                                           \n" \
    "       if(uKeyPressed == 1)                                                                                                                                    \n" \
    "       {                                                                                                                                                       \n" \
    "           vec4 eye_Coordinates = uViewMatrix * uModelMatrix * aPosition;                                                                                      \n" \
    "           vec3 transformed_Normals = normalize(mat3(uViewMatrix * uModelMatrix) * aNormal);                                                                   \n" \
    "           vec3 lightAmbient[2];                                                                                                                               \n" \
    "           vec3 lightDiffused[2];                                                                                                                              \n" \
    "           vec3 lightSpecular[2];                                                                                                                              \n" \
    "           vec3 reflection_Vector[2];                                                                                                                          \n" \
    "           vec3 light_Direction[2];                                                                                                                            \n" \
    "           for( int i = 0; i < 2; i++ )                                                                                                                        \n" \
    "           {                                                                                                                                                   \n" \
    "               lightAmbient[i] = uLight_Ambient[i] * uMaterial_Ambient;                                                                                        \n" \
    "               light_Direction[i] = normalize(vec3(uLight_Position[i] - eye_Coordinates));                                                                     \n" \
    "               lightDiffused[i] = uLight_Diffused[i] * uMaterial_Diffused * max(dot(light_Direction[i], transformed_Normals), 0.0);                            \n" \
    "               reflection_Vector[i] = reflect(-light_Direction[i], transformed_Normals);                                                                       \n" \
    "               vec3 viewer_Vector = normalize(-eye_Coordinates.xyz);                                                                                           \n" \
    "               lightSpecular[i] = uLight_Specular[i] * uMaterial_Specular * pow(max(dot(reflection_Vector[i], viewer_Vector), 0.0), uMaterial_Shininess);      \n" \
    "               oPhong_ADS_Light = oPhong_ADS_Light + lightAmbient[i] + lightDiffused[i] + lightSpecular[i];                                                    \n" \
    "           }                                                                                                                                                   \n" \
    "       }                                                                                                                                                       \n" \
    "       else                                                                                                                                                    \n" \
    "       {                                                                                                                                                       \n" \
    "           oPhong_ADS_Light = vec3(0.0f, 0.0f, 0.0f);                                                                                                          \n" \
    "       }                                                                                                                                                       \n" \
    "       gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;                                                                               \n" \
    "   }                                                                                                                                                           \n";

    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);
    glCompileShader(vertexShaderObject);

    GLint status = 0;
    GLint infoLoglength = 0;
    char *szInfoLog = 0;

    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE )
    {
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {

            szInfoLog = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetShaderInfoLog(vertexShaderObject, infoLoglength, NULL, szInfoLog);
                fprintf(gpFile,"Vertex shader complilation : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        [self uninitialize];
        [self release];
        [NSApp terminate:self];
    }

    /* Fragment Shader */
    const GLchar* fragmentShaderSourceCode =
    "   #version 410 core                                                           \n" \
    "   in vec3 oPhong_ADS_Light;                                                   \n" \
    "   uniform int uKeyPressed;                                                    \n" \
    "   out vec4 FragColor;                                                         \n" \
    "   void main(void)                                                             \n" \
    "   {                                                                           \n" \
    "       if(uKeyPressed == 1)                                                    \n" \
    "       {                                                                       \n" \
    "           FragColor = vec4(oPhong_ADS_Light, 1.0);                            \n" \
    "       }                                                                       \n" \
    "       else                                                                    \n" \
    "       {                                                                       \n" \
    "           FragColor = vec4(1.0, 1.0, 1.0, 1.0);                               \n" \
    "       }                                                                       \n" \
    "   }                                                                           \n";
    
    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObject,1, (const GLchar**)&fragmentShaderSourceCode, NULL);
    glCompileShader(fragmentShaderObject);

    status = 0;
    infoLoglength = 0;
    szInfoLog = 0;

    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {
            szInfoLog  = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetShaderInfoLog(fragmentShaderObject, infoLoglength, NULL, szInfoLog);
                fprintf(gpFile,"fragment shader compilation : %s \n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        [self uninitialize];
        [self release];
        [NSApp terminate:self];
    }

    /* Shader Program */
    shaderProgramObject = glCreateProgram();
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_NORMAL, "aNormal");
    glLinkProgram(shaderProgramObject);

    status = 0;
    infoLoglength = 0;
    szInfoLog = 0;

    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        glGetProgramiv( shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {
            szInfoLog = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObject, infoLoglength, NULL,szInfoLog);
                fprintf(gpFile,"shader program linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        [self uninitialize];
        [self release];
        [NSApp terminate:self];
    }

    // Get Shader Uniform Locations
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");

    light_Diffused_Uniform[0] = glGetUniformLocation(shaderProgramObject, "uLight_Diffused[0]");
    light_Specular_Uniform[0] = glGetUniformLocation(shaderProgramObject, "uLight_Specular[0]");
    light_Ambient_Uniform[0] = glGetUniformLocation(shaderProgramObject, "uLight_Ambient[0]");
    light_Position_Uniform[0] = glGetUniformLocation(shaderProgramObject, "uLight_Position[0]");

    light_Diffused_Uniform[1] = glGetUniformLocation(shaderProgramObject, "uLight_Diffused[1]");
    light_Ambient_Uniform[1] = glGetUniformLocation(shaderProgramObject, "uLight_Ambient[1]");
    light_Specular_Uniform[1] = glGetUniformLocation(shaderProgramObject, "uLight_Specular[1]");
    light_Position_Uniform[1] = glGetUniformLocation(shaderProgramObject, "uLight_Position[1]");

    material_Diffused_Uniform = glGetUniformLocation(shaderProgramObject, "uMaterial_Diffused");
    material_Ambient_Uniform = glGetUniformLocation(shaderProgramObject, "uMaterial_Ambient");
    material_Specular_Uniform = glGetUniformLocation(shaderProgramObject, "uMaterial_Specular");
    material_Shininess_Uniform = glGetUniformLocation(shaderProgramObject, "uMaterial_Shininess");

    keyPressedUniform = glGetUniformLocation(shaderProgramObject, "uKeyPressed");

    const GLfloat pyramid_position[] = 
    {
        // front
        0.0f,  1.0f,  0.0f, // front-top
        -1.0f, -1.0f,  1.0f, // front-left
        1.0f, -1.0f,  1.0f, // front-right

        // right   
        0.0f,  1.0f,  0.0f, // right-top
        1.0f, -1.0f,  1.0f, // right-left
        1.0f, -1.0f, -1.0f, // right-right

        // back
        0.0f,  1.0f,  0.0f, // back-top
        1.0f, -1.0f, -1.0f, // back-left
        -1.0f, -1.0f, -1.0f, // back-right

        // left
        0.0f,  1.0f,  0.0f, // left-top
        -1.0f, -1.0f, -1.0f, // left-left
        -1.0f, -1.0f,  1.0f, // left-right

    };

    const GLfloat pyramidNormals[] =
    {
        // front
        0.000000f, 0.447214f,  0.894427f, // front-top
        0.000000f, 0.447214f,  0.894427f, // front-left
        0.000000f, 0.447214f,  0.894427f, // front-right

        // right			    
        0.894427f, 0.447214f,  0.000000f, // right-top
        0.894427f, 0.447214f,  0.000000f, // right-left
        0.894427f, 0.447214f,  0.000000f, // right-right

        // back
        0.000000f, 0.447214f, -0.894427f, // back-top
        0.000000f, 0.447214f, -0.894427f, // back-left
        0.000000f, 0.447214f, -0.894427f, // back-right

        // left
        -0.894427f, 0.447214f,  0.000000f, // left-top
        -0.894427f, 0.447214f,  0.000000f, // left-left
        -0.894427f, 0.447214f,  0.000000f, // left-right
    };
    
    // VAO Pyramid
    glGenVertexArrays(1, &vao_pyramid);
    glBindVertexArray(vao_pyramid);

    // VBO for POSITION
    glGenBuffers(1, &vbo_pyramid);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_position), pyramid_position, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO for NORMAL
    glGenBuffers(1, &vbo_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Set Depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Light - 1 
    lightAmbient_1[0] = 0.0f;
    lightAmbient_1[1] = 0.0f;
    lightAmbient_1[2] = 0.0f;
    lightAmbient_1[3] = 1.0f;

    lightDiffused_1[0] = 1.0f;
    lightDiffused_1[1] = 0.0f;
    lightDiffused_1[2] = 0.0f; 
    lightDiffused_1[3] = 1.0f;

    lightSpecular_1[0] = 1.0f; 
    lightSpecular_1[1] = 0.0f;
    lightSpecular_1[2] = 0.0f;
    lightSpecular_1[3] = 1.0f;

    lightPosition_1[0] = -2.0f;
    lightPosition_1[1] = 0.0f;
    lightPosition_1[2] = 0.0f;
    lightPosition_1[3] = 1.0f;
    
    // Light - 2
    lightAmbient_2[0] = 0.0f;
    lightAmbient_2[1] = 0.0f;
    lightAmbient_2[2] = 0.0f;
    lightAmbient_2[3] = 1.0f;

    lightDiffused_2[0] = 0.0f;
    lightDiffused_2[1] = 0.0f;
    lightDiffused_2[2] = 1.0f; 
    lightDiffused_2[3] = 1.0f;

    lightSpecular_2[0] = 0.0f; 
    lightSpecular_2[1] = 0.0f;
    lightSpecular_2[2] = 1.0f;
    lightSpecular_2[3] = 1.0f;

    lightPosition_2[0] = 2.0f;
    lightPosition_2[1] = 0.0f;
    lightPosition_2[2] = 0.0f;
    lightPosition_2[3] = 1.0f;

    // Material Properties
    materialDiffused[0] = 1.0f;
    materialDiffused[1] = 1.0f;
    materialDiffused[2] = 1.0f;
    materialDiffused[3] = 1.0f;

    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;

    materialSpecular[0] = 1.0f;
    materialSpecular[1] = 1.0f;
    materialSpecular[2] = 1.0f;
    materialSpecular[3] = 1.0f;

    materialShininess = 128.0f;

    // Set ClearColor
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Initialize Perspective Projection Matrix
    perspectiveProjectionMatrix = vmath::mat4::identity();

    [self resize:WIN_WIDTH :WIN_HEIGHT];

    return(0);
} 

-(void)printGLInfo
{
    int numExtensions;
    int i;
    
    fprintf(gpFile,"OpenGL Vender : %s\n",glGetString(GL_VENDOR));
    fprintf(gpFile,"OpenGL Renderer : %s\n",glGetString(GL_RENDERER));
    fprintf(gpFile,"OpenGL Version : %s \n",glGetString(GL_VERSION));
    fprintf(gpFile,"OpenGL Shading language version(GLSL) : %s \n",glGetString(GL_SHADING_LANGUAGE_VERSION));

    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(i = 0; i < numExtensions; i++)
    {
        fprintf(gpFile,"%s\n",glGetStringi(GL_EXTENSIONS, i));
    }
}

-(void) resize:(int)width :(int)height
{
    // Code
    if(height <= 0)
    {
        height = 1;
    }

    glViewport(0,0,(GLsizei)width,(GLsizei)height);

    // Set Perspective Projection Matrix
    perspectiveProjectionMatrix = vmath::perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
}

-(void) display
{
    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);

    // PYRAMID
    mat4 modelMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    mat4 rotationMatrix = mat4::identity();

    translationMatrix = vmath::translate(0.0f,0.0f,-5.0f);
    rotationMatrix = vmath::rotate(pyramid_angle, 0.0f,1.0f,0.0f);
    modelMatrix = translationMatrix * rotationMatrix;
   
    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == TRUE)
    {
        glUniform1i(keyPressedUniform, 1);

        glUniform3fv(light_Ambient_Uniform[0], 1, lightAmbient_1);
        glUniform3fv(light_Diffused_Uniform[0], 1, lightDiffused_1);
        glUniform3fv(light_Specular_Uniform[0], 1, lightSpecular_1);
        glUniform4fv(light_Position_Uniform[0], 1, lightPosition_1);

        glUniform3fv(light_Ambient_Uniform[1], 1, lightAmbient_2);
        glUniform3fv(light_Diffused_Uniform[1], 1, lightDiffused_2);
        glUniform3fv(light_Specular_Uniform[1], 1, lightSpecular_2);
        glUniform4fv(light_Position_Uniform[1], 1, lightPosition_2);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular);
        glUniform1f(material_Shininess_Uniform, materialShininess);

    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_pyramid);
    glDrawArrays(GL_TRIANGLES, 0 , 12 );
    glBindVertexArray(0);
    glUseProgram(0);
}

-(void) myUpdate
{
    // Code
    pyramid_angle = pyramid_angle + 1.5f;
    if(pyramid_angle >= 360.0f)
    {
        pyramid_angle = pyramid_angle - 360.0f;
    }
}

-(void) uninitialize
{
    // Code
    if(shaderProgramObject)
    {
        glUseProgram(shaderProgramObject);
        GLint numShaders = 0;

        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0)
        {
            GLuint *pShaders = (GLuint*)malloc(numShaders * sizeof(GLuint));
            if(pShaders != NULL)
            {
                glGetAttachedShaders(shaderProgramObject, numShaders, NULL, pShaders);
                for(GLint i = 0; i < numShaders; i++)
                {
                    glDetachShader(shaderProgramObject, pShaders[i]);
                    glDeleteShader(pShaders[i] );
                    pShaders[i] = 0;
                }
        
                free(pShaders);
                pShaders = NULL;
            }
        }

        glUseProgram(0);
        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
    }

    // Delete VBO of Position
    if(vbo_pyramid)
    {
        glDeleteBuffers(1, &vbo_pyramid);
        vbo_pyramid = 0;
    }

    if(vbo_normal)
    {
        glDeleteBuffers(1, &vbo_normal);
        vbo_normal = 0;
    }

    // Delete VAO
    if(vao_pyramid)
    {
        glDeleteVertexArrays(1, &vao_pyramid);
        vao_pyramid = 0;
    }
}

-(BOOL)acceptsFirstResponder
{
    // Code
    [[self window]makeFirstResponder:self];

    return(YES);
}

-(void)keyDown:(NSEvent *)event
{
    // Code
    int key = (int) [[event characters]characterAtIndex:0];

    switch(key)
    {
        case 27:
            [self uninitialize];
            [self release];
            [NSApp terminate:self];
            break;

        case 'f':
        case 'F':
            [[self window]toggleFullScreen:self];
            break;

        case 'l' :
        case 'L' :
            if(bLightingEnabled == FALSE)
            {
                bLightingEnabled = TRUE;
            }
            else
            {
                bLightingEnabled = FALSE;
            }
            break;
        
        case 'a' :
        case 'A' :
            if(bAnimationEnabled == FALSE)
            {
                bAnimationEnabled = TRUE;
            }
            else
            {
                bAnimationEnabled = FALSE;
            }
            break;
        
        default :
            break;
    }
}

-(void)mouseDown:(NSEvent *)event
{
    // Code
}

-(void)rightMouseDown:(NSEvent *)event
{
    // Code
}

-(void)dealloc
{
    [super dealloc];

    [self uninitialize];
    
    if(displayLink)
    {
        CVDisplayLinkStop(displayLink);
        CVDisplayLinkRelease(displayLink);
        displayLink = nil;
    }
}
@end

// Definition of Global Callback Function
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext)
{
    // Code
    CVReturn result = [(GLView *)displayLinkContext getFrameForTime:outputTime];
    return(result);
}
