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
    
    int single_tap;

    uint mvpMatrixUniform;
    mat4 perspectiveProjectionMatrix;
}

-(id) initWithFrame:(CGRect)frame
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
        //glPointSize(3);
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
- (void) update
{
    // Code
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
    single_tap++;
    
    switch(single_tap)
    {
        case 1:
            if(GRAPH == TRUE)
            {
                GRAPH = FALSE;
            }
            else
            {
                GRAPH = TRUE;
            }
            break;
        
        case 2:
            if(SQUARE == TRUE)
            {
                SQUARE = FALSE;
            }
            else
            {
                SQUARE = TRUE;
            }
        break;
        
        case 3:
            if(CIRCLE == TRUE)
            {
                CIRCLE = FALSE;
            }
            else
            {
                CIRCLE = TRUE;
            }
            break;
        
        case 4:
            if(TRIANGLE == TRUE)
            {
                TRIANGLE = FALSE;
            }
            else
            {
                TRIANGLE = TRUE;
            }
            break;
        
        case 5:
            GRAPH = FALSE;
            break;
        
        case 6:
            SQUARE = FALSE;
            break;
        
        case 7:
            CIRCLE = FALSE;
            break;
            
        case 8:
            TRIANGLE = FALSE;
            break;
            
        case 9:
            single_tap = 0;
        
    }
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


