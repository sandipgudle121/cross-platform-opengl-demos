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
    SSG_ATTRIBUTE_POSITION = 1,
    SSG_ATTRIBUTE_NORMAL
};

float materialDiffused[4];
float lightDiffused[4];
float lightPosition[4];

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

    if(bAnimationEnabled == TRUE)
    {
        [self update];
    }
    
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
    "   #version 300 es                                                                                                                                             \n" \
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
    "   #version 300 es                                                             \n" \
    "   precision mediump float;                                                    \n" \
    "   precision highp int;                                                        \n" \
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
                printf("shader program linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        [self uninitialize];
        [self release];
        exit(0);
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
    glClearDepthf(1.0f);
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

    // PYRAMID
    mat4 modelMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    mat4 rotationMatrix = mat4::identity();

    translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
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

- (void) update
{
    // Code
    pyramid_angle = pyramid_angle + 1.5f;
    if(pyramid_angle >= 360.0f)
    {
        pyramid_angle = pyramid_angle - 360.0f;
    }
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
    if(bAnimationEnabled == FALSE)
    {
        bAnimationEnabled = TRUE;
    }
    else
    {
        bAnimationEnabled = FALSE;
    }
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gestureReconnizer
{
    // Code
    if(bLightingEnabled == FALSE)
    {
        bLightingEnabled = TRUE;
    }
    else
    {
        bLightingEnabled = FALSE;
    }
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


