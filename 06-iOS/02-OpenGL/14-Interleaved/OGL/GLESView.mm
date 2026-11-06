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
    SSG_ATTRIBUTE_COLOR,
    SSG_ATTRIBUTE_NORMAL,
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

-(id) initWithFrame:(CGRect)frame
{
    bLightingEnabled = NO;
    bAnimationEnabled = NO;
    
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
    "   #version 300 es                                                                 \n" \
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
    "   #version 300 es                                                                                                                                                 \n" \
    "   precision mediump float;                                                                                                                                        \n" \
    "   precision highp int;                                                                                                                                            \n" \
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
        // position                // color             // normals                // texcoords
         1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.0f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.0f,     0.0f,  0.0f,  1.0f,    0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,    1.0f, 0.0f, 0.0f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
        1.0f, -1.0f,  1.0f,    1.0f, 0.0f, 0.0f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
                         
        // right
        // position                // color             // normals                // texcoords
         1.0f,  1.0f, -1.0f,    0.0f, 0.0f, 1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
         1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f,     1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    0.0f, 0.0f, 1.0f,     1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
                         
        // back
        // position                // color             // normals                // texcoords
         1.0f,  1.0f, -1.0f,    1.0f, 1.0f, 0.0f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,    1.0f, 1.0f, 0.0f,     0.0f,  0.0f, -1.0f,    0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,    1.0f, 1.0f, 0.0f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    1.0f, 1.0f, 0.0f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f,
                         
        // left
        // position                // color             // normals                // texcoords
        -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 1.0f,    -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,    1.0f, 0.0f, 1.0f,    -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
                         
        // top
        // position                // color             // normals                // texcoords
         1.0f,  1.0f, -1.0f,    0.0f, 1.0f, 0.0f,     0.0f,  1.0f,  0.0f,    1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,    0.0f, 1.0f, 0.0f,     0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f,     0.0f,  1.0f,  0.0f,    0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f,     0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
                         
        // bottom
        // position                // color             // normals                // texcoords
         1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f,     0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f,     0.0f, -1.0f,  0.0f,    0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f,     0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f,     0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
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
    glClearDepthf(1.0f);
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
    texture_marble = [self loadGLTextureFromImageFile: @"marble" :@"bmp"];
    if(texture_marble == 0)
    {
        printf("Failed to Marble Texture for Cube\n");
        return(-1);
    }

    // Initialize Perspective Projection Matrix
    perspectiveProjectionMatrix = vmath::mat4::identity();

    [self resize:WIN_WIDTH :WIN_HEIGHT];

    return(0);
}


// Texture Function
-(GLuint) loadGLTextureFromImageFile: (NSString *)imageFileName :(NSString *)extension
{
    // Code
    // Step 1: Create NSString based image file name.
    NSString *textureFileNameWithPath = [[NSBundle mainBundle]pathForResource:imageFileName ofType:extension];
    
    // Step 2: Create UIImage from the image file.
    UIImage *image = [UIImage imageWithContentsOfFile:textureFileNameWithPath];
    if (image == nil)
    {
        NSLog(@"Failed to create UIImage from file: %@", textureFileNameWithPath);
        return 0;
    }

    // Step 3: Create CGImageObject from above NSImageObject.
    CGImageRef cgImage = [image CGImage];

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
    mat4 viewMatrix = mat4::identity();
    mat4 modelMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    mat4 scaleMatrix = mat4::identity();
    mat4 rotationMatrix1 = mat4::identity();
    mat4 rotationMatrix2 = mat4::identity();
    mat4 rotationMatrix3 = mat4::identity();
    mat4 rotationMatrix = mat4::identity();

    translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
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


