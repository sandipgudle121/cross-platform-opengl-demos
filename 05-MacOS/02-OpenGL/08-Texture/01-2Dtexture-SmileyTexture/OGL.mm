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
    SSG_ATTRIBUTE_COLOR = 1,
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

    uint vao; 
    uint vbo_position;
    uint vbo_texcord;
    uint texture_smiley;

    uint mvpMatrixUniform;
    uint textureSamplerUniform;
    mat4 perspectiveProjectionMatrix;
}

-(id)initWithFrame:(NSRect)frame
{
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
    "   #version 410 core                                               \n" \
    "   in vec4 aPosition;                                              \n" \
    "   in vec2 aTexCoord;                                              \n" \
    "   out vec2 oTexCoord;                                             \n" \
    "   uniform mat4 uMVPMatrix;                                        \n" \
    "   void main(void)                                                 \n" \
    "   {                                                               \n" \
    "       gl_Position = uMVPMatrix * aPosition;                       \n" \
    "       oTexCoord = aTexCoord;                                      \n" \
    "   }                                                               \n";

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
    "   #version 410 core                                               \n" \
    "   in vec2 oTexCoord;                                              \n" \
    "   uniform sampler2D uTextureSampler;                              \n" \
    "   out vec4 FragColor;                                             \n" \
    "   void main(void)                                                 \n" \
    "   {                                                               \n" \
    "       FragColor = texture(uTextureSampler, oTexCoord);            \n" \
    "   }                                                               \n";
    
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

    /* Get Shader Uniform Locations */
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");

    const float square_position[] = 
    {
        1.0f,1.0f,0.0f,
        -1.0f,1.0f,0.0f,
        -1.0f,-1.0f,0.0f,
        1.0f,-1.0f,0.0f
    };
    
    const float square_texcord[] = 
    { 
        1, 1,
        0, 1,
        0, 0,
        1, 0
    };

    /* VAO_square (Vertex Array Object ) */
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);

    // VBO for POSITION
    glGenBuffers(1, &vbo_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_position), square_position, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO for TEXCOORD
    glGenBuffers(1, &vbo_texcord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_texcord), square_texcord, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_TEXCOORD);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Set Depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set ClearColor
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Create Texture
    texture_smiley = [self loadGLTexture: "Smiley.bmp"];
    if(texture_smiley == 0)
    {
        fprintf(gpFile,"Failed to Smiley Texture\n");
        return(-1);
    }

    // Initialize Perspective Projection Matrix
    perspectiveProjectionMatrix = vmath::mat4::identity();

    [self resize:WIN_WIDTH :WIN_HEIGHT];

    return(0);
} 

// Texture Function
-(GLuint)loadGLTexture:(const char*)szImageFileName
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
    GLuint texture;

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
    perspectiveProjectionMatrix = vmath::perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
}

-(void) display
{
    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);

    mat4 modelViewMatrix = mat4::identity();
    modelViewMatrix = vmath::translate(0.0f,0.0f,-6.0f);
    mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_smiley);
    glUniform1i(textureSamplerUniform, 0);

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0 , 4 );
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

    if(vbo_texcord)
    {
        glDeleteBuffers(1, &vbo_texcord);
        vbo_texcord = 0;
    }

    if(vbo_position)
    {
        glDeleteBuffers(1, &vbo_position);
        vbo_position = 0;
    }

    if(vao)
    {
        glDeleteVertexArrays(1,&vao);
        vao = 0;
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

