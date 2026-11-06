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
    SSG_ATTRIBUTE_POSITION = 0,
    SSG_ATTRIBUTE_COLOR,
    SSG_ATTRIBUTE_NORMAL,
    SSG_ATTRIBUTE_TEXCOORD
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

    uint vao_cube;
    uint vbo_position_cube;
    uint vbo_color_cube;
    uint vbo_texcoord_cube;
    uint VBO;

    uint texture_marble;
    float cube_angle;
    BOOL bLightingEnabled;
    BOOL bAnimationEnabled;

    uint light_Diffused_Uniform;
    uint light_Ambient_Uniform;
    uint light_Position_Uniform;
    uint light_Specular_Uniform;

    float lightDiffused[4];
    float lightAmbient[4];    
    float lightSpecular[4]; 
    float lightPosition[4];        

    float materialDiffused[4];   
    float materialAmbient[4];
    float materialSpecular[4];
    float materialShininess;

    uint material_Diffused_Uniform;
    uint material_Ambient_Uniform;
    uint material_Specular_Uniform;
    uint material_Shininess_Uniform;

    uint keyPressedUniform;
    uint modelMatrixUniform;
    uint projectionMatrixUniform;
    uint viewMatrixUniform;

    uint textureSamplerUniform;

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
            NSOpenGLPFADepthSize, 32,
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
    "   #version 410 core                                                               \n" \
    "   in vec4 aPosition;                                                              \n" \
    "   in vec2 aTexCoord;                                                              \n" \
    "   in vec3 aNormal;                                                                \n" \
    "   in vec4 aColor;                                                                 \n" \
    "   out vec4 oColor;                                                                \n" \
    "   out vec2 oTexCoord;                                                             \n" \
    "   uniform mat4 uViewMatrix;                                                       \n" \
    "   uniform mat4 uModelMatrix;                                                      \n" \
    "   uniform mat4 uProjectionMatrix;                                                 \n" \
    "   uniform vec4 uLight_Position;                                                   \n" \
    "   uniform int uKeyPressed;                                                        \n" \
    "   out vec3 oTransformed_Normals;                                                  \n" \
    "   out vec3 oLight_Direction;                                                      \n" \
    "   out vec3 oViewer_Vector;                                                        \n" \
    "   void main(void)                                                                 \n" \
    "   {                                                                               \n" \
    "       if(uKeyPressed == 1)                                                        \n" \
    "       {                                                                           \n" \
    "           vec4 eye_Coordinates = uViewMatrix * uModelMatrix * aPosition;          \n" \
    "           oTransformed_Normals = mat3(uViewMatrix * uModelMatrix) * aNormal;      \n" \
    "           oLight_Direction = vec3(uLight_Position - eye_Coordinates);             \n" \
    "           oViewer_Vector = -eye_Coordinates.xyz;                                  \n" \
    "       }                                                                           \n" \
    "       else                                                                        \n" \
    "       {                                                                           \n" \
    "           oTransformed_Normals = vec3(0.0, 0.0, 0.0);                             \n" \
    "           oLight_Direction = vec3(0.0, 0.0, 0.0);                                 \n" \
    "           oViewer_Vector = vec3(0.0, 0.0, 0.0);                                   \n" \
    "       }                                                                           \n" \
    "       oColor = aColor;                                                            \n" \
    "       oTexCoord = aTexCoord;                                                      \n" \
    "       gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;   \n" \
    "   }                                                                               \n";

    uint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);
    glCompileShader(vertexShaderObject);

    GLint status = 0;
    GLint infoLoglength = 0;
    char *szInfoLog = NULL;

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
    "   #version 410 core                                                                                                                                               \n" \
    "   in vec2 oTexCoord;                                                                                                                                              \n" \
    "   in vec4 oColor;                                                                                                                                                 \n" \
    "   in vec3 oTransformed_Normals;                                                                                                                                   \n" \
    "   in vec3 oLight_Direction;                                                                                                                                       \n" \
    "   in vec3 oViewer_Vector;                                                                                                                                         \n" \
    "   uniform vec3 uLight_Ambient;                                                                                                                                    \n" \
    "   uniform vec3 uLight_Specular;                                                                                                                                   \n" \
    "   uniform vec3 uLight_Diffused;                                                                                                                                   \n" \
    "   uniform vec3 uMaterial_Diffused;                                                                                                                                \n" \
    "   uniform vec3 uMaterial_Ambient;                                                                                                                                 \n" \
    "   uniform vec3 uMaterial_Specular;                                                                                                                                \n" \
    "   uniform float uMaterial_Shininess;                                                                                                                              \n" \
    "   uniform int uKeyPressed;                                                                                                                                        \n" \
    "   uniform sampler2D uTextureSampler;                                                                                                                              \n" \
    "   out vec4 FragColor;                                                                                                                                             \n" \
    "   void main(void)                                                                                                                                                 \n" \
    "   {                                                                                                                                                               \n" \
    "       vec3 phong_ADS_Light;                                                                                                                                       \n" \
    "       if(uKeyPressed == 1)                                                                                                                                        \n" \
    "       {                                                                                                                                                           \n" \
    "           vec3 normalized_Transformed_Normals = normalize(oTransformed_Normals);                                                                                  \n" \
    "           vec3 normalized_Light_Direction = normalize(oLight_Direction);                                                                                          \n" \
    "           vec3 normalized_Viewer_Vector = normalize(oViewer_Vector);                                                                                              \n" \
    "           vec3 lightAmbient = uLight_Ambient * uMaterial_Ambient;                                                                                                 \n" \
    "           vec3 lightDiffused = uLight_Diffused * uMaterial_Diffused * max(dot(normalized_Light_Direction, normalized_Transformed_Normals), 0.0);                  \n" \
    "           vec3 reflection_Vector = reflect(-normalized_Light_Direction, normalized_Transformed_Normals);                                                          \n" \
    "           vec3 lightSpecular = uLight_Specular * uMaterial_Specular * pow(max(dot(reflection_Vector, normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );     \n" \
    "           phong_ADS_Light = lightAmbient + lightDiffused + lightSpecular;                                                                                         \n" \
    "       }                                                                                                                                                           \n" \
    "       else                                                                                                                                                        \n" \
    "       {                                                                                                                                                           \n" \
    "           phong_ADS_Light = vec3(1.0, 1.0, 1.0);                                                                                                                  \n" \
    "       }                                                                                                                                                           \n" \
    "       vec3 texColor = vec3(texture(uTextureSampler, oTexCoord));                                                                                                  \n" \
    "       FragColor = vec4(texColor * vec3(oColor) * phong_ADS_Light, 1.0);                                                                                           \n" \
    "   }                                                                                                                                                               \n";
    
    uint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
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

    // Shader Program
    shaderProgramObject = glCreateProgram();
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_COLOR, "aColor");
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_NORMAL, "aNormal");
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_TEXCOORD, "aTexCoord");
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

    light_Diffused_Uniform = glGetUniformLocation(shaderProgramObject, "uLight_Diffused");
    light_Ambient_Uniform = glGetUniformLocation(shaderProgramObject, "uLight_Ambient");
    light_Specular_Uniform = glGetUniformLocation(shaderProgramObject, "uLight_Specular");
    light_Position_Uniform = glGetUniformLocation(shaderProgramObject, "uLight_Position");

    material_Diffused_Uniform = glGetUniformLocation(shaderProgramObject, "uMaterial_Diffused");
    material_Ambient_Uniform = glGetUniformLocation(shaderProgramObject, "uMaterial_Ambient");
    material_Specular_Uniform = glGetUniformLocation(shaderProgramObject, "uMaterial_Specular");
    material_Shininess_Uniform = glGetUniformLocation(shaderProgramObject, "uMaterial_Shininess");

    keyPressedUniform = glGetUniformLocation(shaderProgramObject, "uKeyPressed");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");

    GLfloat cube_PCNT[] =
    {
	    // front
	    // position				// color			 // normals				// texcoords
	     1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	1.0f, 1.0f,
	    -1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	0.0f, 1.0f,
	    -1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	0.0f, 0.0f,
	    1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	1.0f, 0.0f,
						 
	    // right			 
	    // position				// color			 // normals				// texcoords
	     1.0f,  1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
	     1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
	     1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
	     1.0f, -1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
						 
	    // back				 
	    // position				// color			 // normals				// texcoords
	     1.0f,  1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	1.0f, 1.0f,
	    -1.0f,  1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	0.0f, 1.0f,
	    -1.0f, -1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	0.0f, 0.0f,
	     1.0f, -1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	1.0f, 0.0f,
						 
    	// left				 
	    // position				// color			 // normals				// texcoords
	    -1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
	    -1.0f,  1.0f, -1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
	    -1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
	    -1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
						 
	    // top				 
	    // position				// color			 // normals				// texcoords
	     1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	1.0f, 1.0f,
	    -1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	0.0f, 1.0f,
	    -1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	0.0f, 0.0f,
	     1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	1.0f, 0.0f,
						 
	    // bottom			 
	    // position				// color			 // normals				// texcoords
	     1.0f, -1.0f,  1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	1.0f, 1.0f,
	    -1.0f, -1.0f,  1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	0.0f, 1.0f,
	    -1.0f, -1.0f, -1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	0.0f, 0.0f,
	     1.0f, -1.0f, -1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	1.0f, 0.0f,
    };

    // VAO
    glGenVertexArrays(1,&vao_cube);
    glBindVertexArray(vao_cube);

    // VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 24 * 11 * sizeof(float), cube_PCNT, GL_STATIC_DRAW);      // 24 * 11 * sizeof(float) it is nothing but size of cube_PCNT array
    
    // POSITION
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);

    // COLOR
    glVertexAttribPointer(SSG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(SSG_ATTRIBUTE_COLOR);

    // NORMAL
    glVertexAttribPointer(SSG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(SSG_ATTRIBUTE_NORMAL);

    // TEXCOORD
    glVertexAttribPointer(SSG_ATTRIBUTE_TEXCOORD, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(SSG_ATTRIBUTE_TEXCOORD);

    // Unbind VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind VAO
    glBindVertexArray(0);

    // Set Depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set ClearColor
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Light Properties
    lightDiffused[0] = 1.0f;
    lightDiffused[1] = 1.0f;
    lightDiffused[2] = 1.0f;
    lightDiffused[3] = 1.0f;

    lightAmbient[0] = 0.1f;
    lightAmbient[1] = 0.1f;
    lightAmbient[2] = 0.1f;
    lightAmbient[3] = 1.0f;

    lightSpecular[0] = 1.0f; 
    lightSpecular[1] = 1.0f;
    lightSpecular[2] = 1.0f;
    lightSpecular[3] = 1.0f;

    lightPosition[0] = 0.0f; 
    lightPosition[1] = 0.0f;
    lightPosition[2] = 2.0f;
    lightPosition[3] = 1.0f;

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

    // Create Texture
    texture_marble = [self loadGLTexture: "marble.bmp"];
    if(texture_marble == 0)
    {
        fprintf(gpFile,"Failed to marble Texture\n");
        return(-1);
    }

    // Initialize Perspective Projection Matrix
    perspectiveProjectionMatrix = vmath::mat4::identity();

    [self resize:WIN_WIDTH :WIN_HEIGHT];

    return(0);
} 

// Texture Function
-(uint)loadGLTexture:(const char*)szImageFileName
{
    // Code
    // Step 1: Create NSString based image file name.
    NSBundle *appBundle = [NSBundle mainBundle];
    NSString *appDirPath = [appBundle bundlePath];
    NSString *parentDirPath = [appDirPath stringByDeletingLastPathComponent];
    NSString *imageFileNameWithPath = [NSString stringWithFormat:@"%@/%s",parentDirPath,szImageFileName];

    // Step 2: Create NSImageObject.
    NSImage *image = [[NSImage alloc]initWithContentsOfFile:imageFileNameWithPath];
    if(image == nil)
    {
        fprintf(gpFile,"Create NSImageObject Failed\n");
        [self uninitialize];
        [self release];
        [NSApp terminate:self];
    }

    // Step 3: Create CGImageObject from above NSImageObject.
    CGImageRef cgImage = [image CGImageForProposedRect:nil context:nil hints:nil];

    // Step 4: Get Width and Height of image using above CGImage.
    int imageWidth = (int)CGImageGetWidth(cgImage);
    int imageHeight = (int)CGImageGetHeight(cgImage); 
    
    // Step 5: Get CGDataProviderRef from above CGImage.
    CGDataProviderRef imageDataProviderRef = CGImageGetDataProvider(cgImage);

    // Step 6: Get CFDataRect from above CGDataProviderRef.
    CFDataRef imageDataRef = CGDataProviderCopyData(imageDataProviderRef);

    // Step 7: Get void* format of image data for image data from above CFDataRef.
    const void* imageData = CFDataGetBytePtr(imageDataRef);

    // Step 8: Use usual OpenGL functions to create texture like... glGenTextures(), glBindTexture(), glPixelStorei(), glTexParameteri(), glTexImage2D(), glGenerateMimmap().
    uint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Step 9: Call CFRelease() to release the Data ref object that we created in 6th step.
    CFRelease(imageDataRef);

    return(texture);
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
    perspectiveProjectionMatrix = vmath::perspective(45.0f,(float)width/(float)height,0.1f,100.0f);
}

-(void) display
{
    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);

    // Cube
    mat4 viewMatrix = mat4::identity();
    mat4 modelMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    mat4 scaleMatrix = mat4::identity();
    mat4 rotationMatrix1 = mat4::identity();
    mat4 rotationMatrix2 = mat4::identity();
    mat4 rotationMatrix3 = mat4::identity();
    mat4 rotationMatrix = mat4::identity();

    translationMatrix = vmath::translate(0.0f, 0.0f, -7.0f);
    scaleMatrix = vmath::scale(1.0f, 1.0f, 1.0f);
    rotationMatrix1 = vmath::rotate(cube_angle, 1.0f, 0.0f, 0.0f);
    rotationMatrix2 = vmath::rotate(cube_angle, 0.0f, 1.0f, 0.0f);
    rotationMatrix3 = vmath::rotate(cube_angle, 0.0f, 0.0f, 1.0f);
    rotationMatrix = rotationMatrix1 * rotationMatrix2 * rotationMatrix3;
    modelMatrix = translationMatrix * scaleMatrix * rotationMatrix;
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_marble);
    glUniform1i(textureSamplerUniform, 0);

    // Push above mvp into vertex shaders's mvp uniform
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == TRUE)
    {
        glUniform1i(keyPressedUniform, 1);

        glUniform3fv(light_Diffused_Uniform, 1, lightDiffused);
        glUniform3fv(light_Ambient_Uniform, 1, lightAmbient);
        glUniform3fv(light_Specular_Uniform, 1, lightSpecular);
        glUniform4fv(light_Position_Uniform, 1, lightPosition);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular);
        glUniform1f(material_Shininess_Uniform, materialShininess);

    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }    
    
    glBindVertexArray(vao_cube);
    glDrawArrays(GL_TRIANGLE_FAN, 0 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 4 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 8 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 12 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 16 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 20 , 4 );
    glBindVertexArray(0);
    glUseProgram(0);
}

-(void) myUpdate
{
    // Code
    cube_angle = cube_angle - 1.5f;
    if(cube_angle <= 0.0f)
        cube_angle = cube_angle + 360.0f;
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
            uint *pShaders = (uint*)malloc(numShaders * sizeof(uint));
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

    // Delete kundali texture texcords
    if(vbo_texcoord_cube)
    {
        glDeleteBuffers(1, &vbo_texcoord_cube);
        vbo_texcoord_cube = 0;
    }

    // DELETE VBO of Color
    if(vbo_color_cube)
    {
        glDeleteBuffers(1, &vbo_color_cube);
        vbo_color_cube = 0;
    }

    // Delete VBO of Position
    if(vbo_position_cube)
    {
        glDeleteBuffers(1, &vbo_position_cube);
        vbo_position_cube = 0;
    }

    // DELETE VAO_cube
    if(vao_cube)
    {
        glDeleteVertexArrays(1,&vao_cube);
        vao_cube = 0;
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
        
        case 'L' :
        case 'l' :
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

