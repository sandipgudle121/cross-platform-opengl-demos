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
    SSG_ATTRIBUTE_COLOR
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
    
    GLuint vao_H_Lines;
    GLuint vao_V_Lines;
    GLuint vao_green_line;
    GLuint vao_red_line;
    GLuint vao_triangle;
    GLuint vao_square;
    GLuint vao_circle;

    GLuint vbo_position_H_Lines;
    GLuint vbo_position_V_Lines;
    GLuint vbo_position_green_line;
    GLuint vbo_position_red_line;
    GLuint vbo_position_triangle;
    GLuint vbo_position_square;
    GLuint vbo_position_circle;

    GLuint vbo_color;
    GLuint vbo_color_green_line;
    GLuint vbo_color_red_line;
    GLuint vbo_color_hollow_triangle;
    GLuint vbo_color_hollow_square;
    GLuint vbo_color_hollow_circle;

    BOOL  TRIANGLE;
    BOOL  CIRCLE;
    BOOL  SQUARE;
    BOOL  GRAPH;

    int circle_points;

    uint mvpMatrixUniform;
    mat4 perspectiveProjectionMatrix;
}

-(id)initWithFrame:(NSRect)frame
{
    circle_points = 360;

    TRIANGLE = NO;
    CIRCLE = NO;
    SQUARE = NO;
    GRAPH = NO;

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
    [self myUpdate];
    [pool release];

    return(kCVReturnSuccess);
}


-(int)initialize
{
    // Code
    [self printGLInfo];

    const GLchar* vertexShaderSourceCode =
    "   #version 410 core                               \n" \
    "   in vec4 aPosition;                              \n" \
    "   in vec4 aColor;                                 \n" \
    "   out vec4 oColor;                                \n" \
    "   uniform mat4 uMVPMatrix;                        \n" \
    "   void main(void)                                 \n" \
    "   {                                               \n" \
    "       gl_Position = uMVPMatrix * aPosition;       \n" \
    "       oColor = aColor;                            \n" \
    "   }                                               \n";

    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
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
    "   #version 410 core                                      \n" \
    "   in vec4 oColor;                                        \n" \
    "   out vec4 FragColor;                                    \n" \
    "   void main(void)                                        \n" \
    "   {                                                      \n" \
    "       FragColor = oColor;                                \n" \
    "   }                                                      \n";
    
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
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_COLOR, "aColor");
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

    /* Get Shader Uniform Locations */
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");

    const GLfloat red_Line[] = 
    {
        -2.0f,0.0f,0.0f,
        2.0f,0.0f,0.0f
    };

    const GLfloat green_Line[] = 
    {
        0.0f, 2.0f, 0.0f,
        0.0f, -2.0f, 0.0f
    };

    const GLfloat horizontal_Lines[] = 
    {
        -2.0f,0.0f,0.0f,
        2.0f,0.0f,0.0f
    };

    const GLfloat vertical_Lines[] = 
    {
        0.0f, 2.0f, 0.0f,
        0.0f, -2.0f, 0.0f
    };                                    
    
    const GLfloat triangle_color[] = 
    {
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f
    };
    
    const GLfloat green_line_color[] = 
    {
        0.0f,1.0f,0.0f,
        0.0f,1.0f,0.0f,
        0.0f,1.0f,0.0f
    };

    const GLfloat red_line_color[] = 
    {
        1.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f
    };
    
    const GLfloat trianle[] = 
    {
        0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
    };
    
    const GLfloat square[] = 
    {
        1.0f,1.0f,0.0f,
        -1.0f,1.0f,0.0f,
        -1.0f,-1.0f,0.0f,
        1.0f,-1.0f,0.0f
    };

    const GLfloat hollow_circle[] = 
    {
        1.0f,1.0f,0.0f,
        -1.0f,1.0f,0.0f,
        -1.0f,-1.0f,0.0f,
        1.0f,-1.0f,0.0f
    };

    const GLfloat hollow_triangle_color[] = 
    {
        1.0f,1.0f,0.0f,
        1.0f,1.0f,0.0f,
        1.0f,1.0f,0.0f
    }; 
    
    const GLfloat hollow_square_color[] = 
    {
        1.0f,1.0f,0.0f,
        1.0f,1.0f,0.0f,
        1.0f,1.0f,0.0f,
        1.0f,1.0f,0.0f
    };
    
    GLfloat circleVertices[circle_points * 3];
    
    /* VAO_H_Lines (Vertex Array Object ) */
    glGenVertexArrays(1,&vao_H_Lines);
    glBindVertexArray(vao_H_Lines);

    /* VBO for Position for horizontal lines */
    glGenBuffers(1, &vbo_position_H_Lines);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_H_Lines);
    glBufferData(GL_ARRAY_BUFFER, sizeof(horizontal_Lines), horizontal_Lines, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* VBO for Color */
    glGenBuffers(1, &vbo_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_color), triangle_color, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // VAO_V_Lines (Vertex Array Object)
    glGenVertexArrays(1,&vao_V_Lines);
    glBindVertexArray(vao_V_Lines);

    /* VBO for Position for vertical lines */
    glGenBuffers(1, &vbo_position_V_Lines);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_V_Lines);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertical_Lines), vertical_Lines, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* VBO for Color */
    glGenBuffers(1, &vbo_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_color), triangle_color, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // vao for green line
    glGenVertexArrays(1, &vao_green_line);
    glBindVertexArray(vao_green_line);

    // vbo position for green line
    glGenBuffers(1, &vbo_position_green_line);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_green_line);
    glBufferData(GL_ARRAY_BUFFER, sizeof(green_Line), green_Line, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // vbo color for gren line color
    glGenBuffers(1, &vbo_color_green_line);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_green_line);
    glBufferData(GL_ARRAY_BUFFER, sizeof(green_line_color), green_line_color, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // vao for red line
    glGenVertexArrays(1, &vao_red_line);
    glBindVertexArray(vao_red_line);

    // vbo position for green line
    glGenBuffers(1, &vbo_position_red_line);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_red_line);
    glBufferData(GL_ARRAY_BUFFER, sizeof(red_Line), red_Line, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // vbo color for red line color
    glGenBuffers(1, &vbo_color_red_line);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_red_line);
    glBufferData(GL_ARRAY_BUFFER, sizeof(red_line_color), red_line_color, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);



    // vao for hollow triangle
    glGenVertexArrays(1, &vao_triangle);
    glBindVertexArray(vao_triangle);

    // vbo position for  hollow triangle
    glGenBuffers(1, &vbo_position_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_triangle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(trianle), trianle, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // vbo color for hollow triangle
    glGenBuffers(1, &vbo_color_hollow_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_hollow_triangle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hollow_triangle_color), hollow_triangle_color, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // vao for hollow square
    glGenVertexArrays(1, &vao_square);
    glBindVertexArray(vao_square);

    // vbo position for  hollow square
    glGenBuffers(1, &vbo_position_square);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_square);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // vbo color for hollow square
    glGenBuffers(1, &vbo_color_hollow_square);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_hollow_square);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hollow_square_color), hollow_square_color, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for (int i = 0; i < circle_points; ++i) 
    {
        float angle = 2 * M_PI * i / circle_points;
        circleVertices[i * 3] = cos(angle); // x 
        circleVertices[i * 3 + 1] = sin(angle); // y
        circleVertices[i * 3 + 2] = 0.0f; // z 
    }

    // Circle
    // VAO_circle (Vertex Array Object ) 
    glGenVertexArrays(1, &vao_circle);
    glBindVertexArray(vao_circle);

    // VBO for  circle position
    glGenBuffers(1, &vbo_position_circle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_circle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // vbo for circle colo
    glVertexAttrib3f(SSG_ATTRIBUTE_COLOR, 1.0f,1.0f,0.0f);
    glBindVertexArray(0);

    // Set Depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

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

    if(GRAPH == TRUE)
    {
        for( int i = 0; i < 80; i++)
        {
            glUseProgram(shaderProgramObject);

            // Transformation
            mat4 modelViewMatrix = vmath::translate(0.0f, 2.0f - i * 0.05f, -5.0f);
            mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

            // Push above mvp into vertex shaders's mvp uniform
            glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
            glBindVertexArray(vao_H_Lines);
            glLineWidth(1);
            glDrawArrays(GL_LINES, 0 , 3);
            glBindVertexArray(0);
            glUseProgram(0);
        }

        for( int i = 0; i < 81; i++)
        {
            glUseProgram(shaderProgramObject);

            // Transformation
            mat4 modelViewMatrix = vmath::translate(2.0f - i * 0.05f, 0.0f, -5.0f);
            mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

            // Push above mvp into vertex shaders's mvp uniform
            glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
            glBindVertexArray(vao_V_Lines);
            glLineWidth(1);
            glDrawArrays(GL_LINES, 0 , 3);
            glBindVertexArray(0);
            glUseProgram(0);
        }

        // 5th thick line
        for( int i = 0; i < 17; i++)
        {
            glUseProgram(shaderProgramObject);

            // Transformation
            mat4 modelViewMatrix = vmath::translate(0.0f, 2.0f - i * 0.25f, -5.0f);
            mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

            // Push above mvp into vertex shaders's mvp uniform
            glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
            glBindVertexArray(vao_H_Lines);
            glLineWidth(2);
            glDrawArrays(GL_LINES, 0 , 3);
            glBindVertexArray(0);
            glUseProgram(0);
        }

        for( int i = 0; i < 17; i++)
        {
            glUseProgram(shaderProgramObject);

            // Transformation
            mat4 modelViewMatrix = vmath::translate(2.0f - i * 0.25f, 0.0f, -5.0f);
            mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

            // Push above mvp into vertex shaders's mvp uniform
            glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
            glBindVertexArray(vao_V_Lines);
            glLineWidth(2);
            glDrawArrays(GL_LINES, 0 , 3);
            glBindVertexArray(0);
            glUseProgram(0);
        }
    
        // Middle vertical green line
        glUseProgram(shaderProgramObject);

        // Transformation
        mat4 modelViewMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
        mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

        // Push above mvp into vertex shaders's mvp uniform
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
        glBindVertexArray(vao_green_line);
        glLineWidth(2);
        glDrawArrays(GL_LINES, 0 , 3);
        glBindVertexArray(0);
        glUseProgram(0);


        // Middle horizontal red line
        glUseProgram(shaderProgramObject);

        // Transformation
        modelViewMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
        modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

        // Push above mvp into vertex shaders's mvp uniform
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
        glBindVertexArray(vao_red_line);
        glLineWidth(2);
        glDrawArrays(GL_LINES, 0 , 3);
        glBindVertexArray(0);
        glUseProgram(0);
    }
    
    // Call hollow square function
    if(SQUARE == TRUE)
    {
        [self square];
    }

    // Call hollow triangle function
    if(TRIANGLE == TRUE)
    {
        [self triangle];
    }

    // Hollow circle
    if(CIRCLE == TRUE)
    {
        glUseProgram(shaderProgramObject);

        mat4 modelViewMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
        mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very important

        // Push above mvp into vertex shaders's mvp uniform
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
        glBindVertexArray(vao_circle);
        glPointSize(3);
        glDrawArrays(GL_LINE_LOOP, 0, circle_points);

        glBindVertexArray(0);
        glUseProgram(0);
    }
}

-(void) triangle
{
    glUseProgram(shaderProgramObject);

    // Transformation
    mat4 modelViewMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
    mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

    // Push above mvp into vertex shaders's mvp uniform
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
    glBindVertexArray(vao_triangle);
    glLineWidth(2);
    glDrawArrays(GL_LINE_LOOP, 0 , 3);
    glBindVertexArray(0);
    glUseProgram(0);
}

-(void) square
{
    glUseProgram(shaderProgramObject);

    mat4 modelViewMatrix = vmath::translate(0.0f,0.0f,-5.0f);
    mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

    // Push above mvp into vertex shaders's mvp uniform 
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
    glBindVertexArray(vao_square);
    glDrawArrays(GL_LINE_LOOP, 0 , 4 );
    glBindVertexArray(0);
    glUseProgram(0);
}

-(void) myUpdate
{
    // Code
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

    if(vbo_position_triangle) 
    {
        glDeleteBuffers(1, &vbo_position_triangle);
        vbo_position_triangle = 0;
    }

    // Delete vao of triangle
    if(vao_triangle) 
    {
        glDeleteVertexArrays(1, &vao_triangle);
        vao_triangle = 0; 
    }   

    // Delete vbo color of triangle
    if(vbo_color_hollow_triangle)
    {
        glDeleteBuffers(1, &vbo_color_hollow_triangle);
        vbo_color_hollow_triangle = 0;
    }
    
    // Delete VBO of Position for square
    if(vbo_position_square) 
    {
        glDeleteBuffers(1, &vbo_position_square);
        vbo_position_square = 0;
    }

    // Delete vao of square
    if(vao_square) 
    {
        glDeleteVertexArrays(1, &vao_square);
        vao_square = 0; 
    }   

    // Delete vbo color of square
    if(vbo_color_hollow_square)
    {
        glDeleteBuffers(1, &vbo_color_hollow_square);
        vbo_color_hollow_square = 0;
    }

    // Delete VBO of Position for circle
    if(vbo_position_circle) 
    {
        glDeleteBuffers(1, &vbo_position_circle);
        vbo_position_circle = 0;
    }

    // Delete VAO_circle
    if(vao_circle) 
    {
        glDeleteVertexArrays(1, &vao_circle);
        vao_circle = 0; 
    }   

    // Delete vbo color of green line
    if(vbo_color_green_line)
    {
        glDeleteBuffers(1, &vbo_color_green_line);
        vbo_color_green_line = 0;
    }

    /* Delete VBO of Color */
    if(vbo_color)
    {
        glDeleteBuffers(1, &vbo_color);
        vbo_color = 0;
    }

    // Delete vbo position for green line 
    if(vbo_position_green_line)
    {
        glDeleteBuffers(1, &vbo_position_green_line);
        vbo_position_green_line = 0;
    }

    // Delete VBO of Position for vertical lines
    if(vbo_position_V_Lines)
    {
        glDeleteBuffers(1, &vbo_position_V_Lines);
        vbo_position_V_Lines = 0;
    }

    /* Delete VBO of Position */
    if(vbo_position_H_Lines)
    {
        glDeleteBuffers(1, &vbo_position_H_Lines);
        vbo_position_H_Lines = 0;
    }

    // Delete VAO_v_Lines
    if(vao_V_Lines)
    {
        glDeleteVertexArrays(1,&vao_V_Lines);
        vao_V_Lines = 0;
    }

    /* Delete VAO_H_Lines */
    if(vao_H_Lines)
    {
        glDeleteVertexArrays(1,&vao_H_Lines);
        vao_H_Lines = 0;
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
        
        case 'T':
        case 't':
            if(TRIANGLE == TRUE)
            {
                TRIANGLE = FALSE;
            }    
            else if(TRIANGLE == FALSE)
            {
                TRIANGLE = TRUE;
            }
            break;

        case 'S':
        case 's':
            if(SQUARE == TRUE)
            {
                SQUARE = FALSE;
            }
            else if(SQUARE == FALSE)
            {
                SQUARE = TRUE;
            }
            break;

        case 'C':
        case 'c':
            if(CIRCLE == TRUE)
            {
                CIRCLE = FALSE;
            }
            else if(CIRCLE == FALSE)
            {
                CIRCLE = TRUE;
            } 
            break;

        case 'G' :
        case 'g' :
            if(GRAPH == TRUE)
            {
                GRAPH = FALSE;
            }                
            else if(GRAPH == FALSE)
            {
                GRAPH = TRUE;
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
