//
//  MyView.m
//  Window
//
//  Created by Sandip Gudle on 31/10/24.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3//glext.h>
#import "GLESView.h"

#include"vmath.h"
using namespace vmath;

// MACROS
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

enum
{
    SSG_ATTRIBUTE_POSITION = 0,
    SSG_ATTRIBUTE_COLOR
};

@implementation GLESView
{
    EAGLContext *eaglContext;
    
    GLuint custumFrameBuffer;
    GLuint colorRenderBuffer;
    GLuint depthRenderBuffer;
    
    id displayLink;
    NSInteger framesPerSecond;
    BOOL isDisplayLink;
    
    uint shaderProgramObject;
    
    GLuint vao_cube;
    GLuint vbo_position_cube;
    GLuint vbo_color_cube;
    GLfloat cube_angle;
    
    uint mvpMatrixUniform;
    mat4 perspectiveProjectionMatrix;
}

-(id) initWithFrame:(CGRect)frame
{
    // Code
    self = [super initWithFrame:frame];
    if(self)
    {
        // Set background to black
        [self setBackgroundColor:[UIColor blackColor]];
        
        // Create CAGL Layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)[super layer];
        
        [eaglLayer setOpaque:YES];
        
        NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        [eaglLayer setDrawableProperties:dictionary];
        
        // Create eagl context
        eaglContext = [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if(eaglContext == nil)
        {
            printf("OpenGLES context creation failed\n");
            [self uninitialize];
            [self release];
            exit(0);
        }
        
        // Set this context as current context
        [EAGLContext setCurrentContext:eaglContext];
        
        // Create custom framebuffer
        glGenFramebuffers(1, &custumFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, custumFrameBuffer);
        
        // Create color render buffer
        glGenRenderbuffers(1, &colorRenderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
        
        // Give storage to color renderbuffer using iOS method
        [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        
        // Assign above colorbuffer to custom colorbuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderBuffer);
        
        // Depth renderbuffer
        // Find width of colorbuffer
        GLint width;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        
        // Find height of colorbuffer
        GLint height;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
        
        // Create and bind with depth render buffer
        glGenRenderbuffers(1, &depthRenderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
        
        // Give storage to depth renderbuffer using OpenGL method
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        
        // Give this depth renderbuffer to custom frambuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
    
        // Check Status of newly created custom buffer
        GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Framebuffer status is not complete\n");
            [self uninitialize];
            [self release];
            exit(0);
        }
        
        // Initialize frames per second
        framesPerSecond = 60;       // 60 is recommended for iOS 8.2
        
        // Initialize display link existing variable
        isDisplayLink = NO;
        
        // Call our initialize method
        int result = [self initialize];
        if(result != 0)
        {
            printf("Initialization failed\n");
            [self uninitialize];
            [self release];
            exit(0);
        }
        
        //--------------------- Single Tap ------------------
        // Create single tap gesture recognizer object
        UITapGestureRecognizer *singleTapGestureRecongnizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onSingleTap:)];
        
        // Set number of taps
        [singleTapGestureRecongnizer setNumberOfTapsRequired:1];
        
        // Set number of fingers
        [singleTapGestureRecongnizer setNumberOfTouchesRequired:1];
        
        // Set the delegate as self
        [singleTapGestureRecongnizer setDelegate:self];
        
        // Add this gesture recognizer to self
        [self addGestureRecognizer:singleTapGestureRecongnizer];
        
        //------------------- Double Tap -----------------
        UITapGestureRecognizer *doubleTapGestureRecongnizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onDoubleTap:)];
        
        [doubleTapGestureRecongnizer setNumberOfTapsRequired:2];
        [doubleTapGestureRecongnizer setNumberOfTouchesRequired:1];
        [doubleTapGestureRecongnizer setDelegate:self];
        [self addGestureRecognizer:doubleTapGestureRecongnizer];
        
        // Double Tap should fail for two single tap
        [singleTapGestureRecongnizer requireGestureRecognizerToFail:doubleTapGestureRecongnizer];
        
        //------------------- Long Press -----------------
        UILongPressGestureRecognizer *longPressGestureRecongnizer = [[UILongPressGestureRecognizer alloc]initWithTarget:self action:@selector(onLongPress:)];
        
        [longPressGestureRecongnizer setDelegate:self];
        [self addGestureRecognizer:longPressGestureRecongnizer];
        
        //------------------- Swipe ---------------------
        UISwipeGestureRecognizer *swipeGestureRecongnizer = [[UISwipeGestureRecognizer alloc]initWithTarget:self action:@selector(onSwipe:)];
        
        [swipeGestureRecongnizer setDelegate:self];
        [self addGestureRecognizer:swipeGestureRecongnizer];
    }
    
    return (self);
}

+(Class) layerClass
{
    // Code
    
    return([CAEAGLLayer class]);
}

//-(void) drawRect:(CGRect)rect
//{
//    // Code
//}


- (void)drawView:(id)displayLink
{
    [EAGLContext setCurrentContext:eaglContext];
    glBindFramebuffer(GL_FRAMEBUFFER, custumFrameBuffer);
    
    [self display];
    [self update];
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)[self layer]];
    
    GLint width;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    
    GLint height;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    // Bind with depth render buffer
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    
    // Give storage to depth renderbuffer using OpenGL method
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    
    glBindFramebuffer(GL_FRAMEBUFFER, custumFrameBuffer);
    
    GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer status is not complete\n");
        [self uninitialize];
        [self release];
        exit(0);
    }
    
    [self resize:width :height];
    [self drawView:displayLink]; // Warm-Up resize();
}

// start displayLink custom method which will be called by AppDelegate.m
-(void) startDisplayLink
{
    // Code
    if(isDisplayLink == NO)
    {
        // Create displayLink
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
        
        // Set frams per second for displayLink
        [displayLink setPreferredFramesPerSecond:framesPerSecond];
        
        // Add this displayLink to our run loop
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        // Set isDisplayLink to YES
        isDisplayLink = YES;
    }
}

-(void) stopDisplayLink
{
    // Code
    if(isDisplayLink == YES)
    {
        // Remove displayLink from runloop by invalidating it
        [displayLink invalidate];
        
        isDisplayLink = NO;
    }
}

- (int)initialize
{
    // Code
    const GLchar* vertexShaderSourceCode =
    "   #version 300 es                                 \n" \
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
                printf("Vertex shader complilation : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        [self uninitialize];
        [self release];
        exit(0);
    }

    /* Fragment Shader */
    const GLchar* fragmentShaderSourceCode =
    "   #version 300 es                                        \n" \
    "   precision mediump float;                               \n" \
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
                printf("fragment shader compilation : %s \n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        [self uninitialize];
        [self release];
        exit(0);
    }

    // Shader Program
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
                printf("shader program linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        [self uninitialize];
        [self release];
        exit(0);
    }

    /* Get Shader Uniform Locations */
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");

    const GLfloat cube_position[] =
    {
        // front
        1.0f,  1.0f,  1.0f, // top-right of front
        -1.0f,  1.0f,  1.0f, // top-left of front
        -1.0f, -1.0f,  1.0f, // bottom-left of front
        1.0f, -1.0f,  1.0f, // bottom-right of front

        // right
        1.0f,  1.0f, -1.0f, // top-right of right
        1.0f,  1.0f,  1.0f, // top-left of right
        1.0f, -1.0f,  1.0f, // bottom-left of right
        1.0f, -1.0f, -1.0f, // bottom-right of right

        // back
        1.0f,  1.0f, -1.0f, // top-right of back
        -1.0f,  1.0f, -1.0f, // top-left of back
        -1.0f, -1.0f, -1.0f, // bottom-left of back
        1.0f, -1.0f, -1.0f, // bottom-right of back

        // left
        -1.0f,  1.0f,  1.0f, // top-right of left
        -1.0f,  1.0f, -1.0f, // top-left of left
        -1.0f, -1.0f, -1.0f, // bottom-left of left
        -1.0f, -1.0f,  1.0f, // bottom-right of left

        // top
        1.0f,  1.0f, -1.0f, // top-right of top
        -1.0f,  1.0f, -1.0f, // top-left of top
        -1.0f,  1.0f,  1.0f, // bottom-left of top
        1.0f,  1.0f,  1.0f, // bottom-right of top

        // bottom
        1.0f, -1.0f,  1.0f, // top-right of bottom
        -1.0f, -1.0f,  1.0f, // top-left of bottom
        -1.0f, -1.0f, -1.0f, // bottom-left of bottom
        1.0f, -1.0f, -1.0f, // bottom-right of bottom
    };

    const GLfloat cube_color[] =
    {
        // front
        1.0f, 0.0f, 0.0f, // top-right of front
        1.0f, 0.0f, 0.0f, // top-left of front
        1.0f, 0.0f, 0.0f, // bottom-left of front
        1.0f, 0.0f, 0.0f, // bottom-right of front

        // right
        0.0f, 0.0f, 1.0f, // top-right of right
        0.0f, 0.0f, 1.0f, // top-left of right
        0.0f, 0.0f, 1.0f, // bottom-left of right
        0.0f, 0.0f, 1.0f, // bottom-right of right

        // back
        1.0f, 1.0f, 0.0f, // top-right of back
        1.0f, 1.0f, 0.0f, // top-left of back
        1.0f, 1.0f, 0.0f, // bottom-left of back
        1.0f, 1.0f, 0.0f, // bottom-right of back

        // left
        1.0f, 0.0f, 1.0f, // top-right of left
        1.0f, 0.0f, 1.0f, // top-left of left
        1.0f, 0.0f, 1.0f, // bottom-left of left
        1.0f, 0.0f, 1.0f, // bottom-right of left

        // top
        0.0f, 1.0f, 0.0f, // top-right of top
        0.0f, 1.0f, 0.0f, // top-left of top
        0.0f, 1.0f, 0.0f, // bottom-left of top
        0.0f, 1.0f, 0.0f, // bottom-right of top

        // bottom
        1.0f, 0.5f, 0.0f, // top-right of bottom
        1.0f, 0.5f, 0.0f, // top-left of bottom
        1.0f, 0.5f, 0.0f, // bottom-left of bottom
        1.0f, 0.5f, 0.0f, // bottom-right of bottom
    };
    
  
    /* Cube */
    /* VAO_square (Vertex Array Object ) */
    glGenVertexArrays(1,&vao_cube);
    glBindVertexArray(vao_cube);

    /* Cube */
    /* VBO for Position */
    glGenBuffers(1, &vbo_position_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_position), cube_position, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* VBO for Color */
    glGenBuffers(1, &vbo_color_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_color), cube_color, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Set Depth
    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set ClearColor
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Initialize Perspective Projection Matrix
    perspectiveProjectionMatrix = vmath::mat4::identity();

    [self resize:WIN_WIDTH :WIN_HEIGHT];

    return(0);
}

- (void)printGLInfo
{
    int numExtensions;
    int i;

    printf("OpenGL Vender : %s\n",glGetString(GL_VENDOR));
    printf("OpenGL Renderer : %s\n",glGetString(GL_RENDERER));
    printf("OpenGL Version : %s \n",glGetString(GL_VERSION));
    printf("OpenGL Shading language version(GLSL) : %s \n",glGetString(GL_SHADING_LANGUAGE_VERSION));

    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(i = 0; i < numExtensions; i++)
    {
        printf("%s\n",glGetStringi(GL_EXTENSIONS, i));
    }
}

- (void) resize:(int)width :(int)height
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

- (void) display
{
    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);

    // Cube
    mat4 modelViewProjectionMatrix = mat4::identity();
    mat4 modelViewMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    mat4 scaleMatrix = mat4::identity();
    mat4 rotationMatrix1 = mat4::identity();
    mat4 rotationMatrix2 = mat4::identity();
    mat4 rotationMatrix3 = mat4::identity();
    
    translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
    scaleMatrix = vmath::scale(0.8f,0.8f,0.8f);
    
    rotationMatrix1 = vmath::rotate(cube_angle, 1.0f,0.0f,0.0f);
    rotationMatrix2 = vmath::rotate(cube_angle, 0.0f,1.0f,0.0f);
    rotationMatrix3 = vmath::rotate(cube_angle, 0.0f,0.0f,1.0f);
    
    mat4 rotationMatrix = rotationMatrix1 * rotationMatrix2 * rotationMatrix3;
    modelViewMatrix = translationMatrix * scaleMatrix * rotationMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);
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

- (void) update
{
    // Code
    cube_angle = cube_angle - 1.5f;
    if(cube_angle <= 0.0f)
        cube_angle = cube_angle + 360.0f;
}

- (void) uninitialize
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

    if(vbo_color_cube)
    {
        glDeleteBuffers(1, &vbo_color_cube);
        vbo_color_cube = 0;
    }

    if(vbo_position_cube)
    {
        glDeleteBuffers(1, &vbo_position_cube);
        vbo_position_cube = 0;
    }

    if(vao_cube)
    {
        glDeleteVertexArrays(1,&vao_cube);
        vao_cube = 0;
    }
    
    // Delete depth renderbuffer
    if(depthRenderBuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderBuffer);
        depthRenderBuffer = 0;
    }
    
    // Delete color renderbuffer
    if(colorRenderBuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderBuffer);
        colorRenderBuffer = 0;
    }
    
    // Delete custom framebuffer
    if(custumFrameBuffer)
    {
        glDeleteFramebuffers(1, &custumFrameBuffer);
        custumFrameBuffer = 0;
    }
    
    // Release EAGL context
    if(eaglContext && [EAGLContext currentContext] == eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
        [eaglContext release];
        eaglContext = nil;
    }
}

-(BOOL) becomeFirstResponder
{
    // Code
    
    return(YES);
}

-(void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    // Code
}

-(void) onSingleTap:(UITapGestureRecognizer *)gestureRecongnizer
{
    // Code
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gestureReconnizer
{
    // Code
}

-(void) onSwipe:(UISwipeGestureRecognizer *)gestureRecognizer
{
    // Code
    [self uninitialize];
    [self release];
    exit(0);
}

-(void) onLongPress:(UILongPressGestureRecognizer *)gestureRecognizer
{
    // Code
}

-(void) dealloc
{
    // Code
    [super dealloc];
    [self uninitialize];
    
    // Release displayLink
    if(displayLink)
    {
        [displayLink invalidate];
        [displayLink stop];
        [displayLink release];
        displayLink = nil;
    }
}
@end


