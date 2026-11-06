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

#define CHECK_IMAGE_WIDTH 64
#define CHECK_IMAGE_HEIGHT 64

enum
{
    SSG_ATTRIBUTE_POSITION = 0,
    SSG_ATTRIBUTE_COLOR = 1,
    SSG_ATTRIBUTE_TEXCOORD
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
    
    uint vao;
    uint vbo_position;
    uint vbo_texcord;
    uint texture_cheackerboard;
    GLubyte checkImage[CHECK_IMAGE_HEIGHT][CHECK_IMAGE_WIDTH][4];

    uint mvpMatrixUniform;
    uint textureSamplerUniform;
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
    "   #version 300 es                                                 \n" \
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
    "   #version 300 es                                                 \n" \
    "   precision mediump float;                                        \n" \
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
                printf("fragment shader compilation : %s \n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        [self uninitialize];
        [self release];
        exit(0);
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
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");

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
    glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
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
    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set ClearColor
    glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
    
    // Create Texture
    [self loadGlTexture];

    // Initialize Perspective Projection Matrix
    perspectiveProjectionMatrix = vmath::mat4::identity();

    [self resize:WIN_WIDTH :WIN_HEIGHT];

    return(0);
}


- (void)loadGlTexture
{
    [self makeCheckImage]; // Fill checkImage array

    glGenTextures(1, &texture_cheackerboard);
    glBindTexture(GL_TEXTURE_2D, texture_cheackerboard);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Create multiple mipmap images
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECK_IMAGE_WIDTH, CHECK_IMAGE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (void)makeCheckImage
{
    for (int i = 0; i < CHECK_IMAGE_HEIGHT; i++)
    {
        for (int j = 0; j < CHECK_IMAGE_WIDTH; j++)
        {
            int c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;

            checkImage[i][j][0] = (GLubyte)c;
            checkImage[i][j][1] = (GLubyte)c;
            checkImage[i][j][2] = (GLubyte)c;
            checkImage[i][j][3] = (GLubyte)255;
        }
    }
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
    // Variables
    float square_position[12];
    
    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);

    mat4 modelViewMatrix = mat4::identity();
    modelViewMatrix = vmath::translate(0.0f, 0.0f, -4.0f);
    mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_cheackerboard);
    glUniform1i(textureSamplerUniform, 0);

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
    glBindVertexArray(vao);
    square_position[0] = 0.0f;
    square_position[1] = 1.0f;
    square_position[2] = 0.0f;
    square_position[3] = -2.0f;
    square_position[4] = 1.0f;
    square_position[5] = 0.0f;
    square_position[6] = -2.0f;
    square_position[7] = -1.0f;
    square_position[8] = 0.0f;
    square_position[9] = 0.0f;
    square_position[10] = -1.0f;
    square_position[11] = 0.0f;

    glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_position), square_position, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_cheackerboard);
    glUniform1i(textureSamplerUniform, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0 , 4 );
    glBindVertexArray(0);

    // Angular square
    // Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
    glBindVertexArray(vao);
    
    square_position[0] = 3.41412f;
    square_position[1] = 1.0f;
    square_position[2] = -4.41412f;
    square_position[3] = 1.0f;
    square_position[4] = 1.0f;
    square_position[5] = 0.0f;
    square_position[6] = 1.0f;
    square_position[7] = -1.0f;
    square_position[8] = 0.0f;
    square_position[9] = 3.41412f;
    square_position[10] = -1.0f;
    square_position[11] = -4.41412f;
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_position), square_position, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_cheackerboard);
    glUniform1i(textureSamplerUniform, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0 , 4 );
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


