// Standard headr files
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>  // For memset

// X 11 header files 
#include<X11/Xlib.h>    // For all X window API
#include<X11/Xutil.h>   // For XVisualInfo and related API
#include<X11/XKBlib.h>

// OpenGL Header files
#include<GL/glew.h>     // This file must be included before gl.h
#include<GL/gl.h>
#include<GL/glx.h>

#include "vmath.h"
#include "Sphere.h"
using namespace vmath;

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define FBO_WIDTH 512
#define FBO_HEIGHT 512

// Global variable declarations
FILE *gpFile = NULL;
Display *display = NULL;
Colormap colormap;
Window window;
XVisualInfo *visualInfo;

Bool bFullscreen = False;
Bool bActiveWindow = False;

typedef GLXContext (*glXCreateContextAttribsARBProc) (Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
GLXFBConfig glxFBConfig; 
GLXContext glxContext = NULL;

// OpenGL related variables for PP
// Cube related variables
GLuint shaderProgramObject_cube = 0;
enum 
{
    SSG_ATTRIBUTE_POSITION = 0,
    SSG_ATTRIBUTE_TEXCOORD
};

GLuint vao_cube = 0;
GLuint vbo_position_cube = 0;
GLuint vbo_color_cube = 0;
GLuint vbo_texcoord_cube = 0;
GLfloat cube_angle = 0.0f;

GLuint mvpMatrixUniform_cube = 0;
GLuint textureSamplerUniform_cube = 0;
mat4 perspectiveProjectionMatrix_cube;
//////////////////////////////

GLint winWidth;
GLint winHeight;

// Sphere related variables
GLuint shaderProgramObject_PV_sphere = 0;
GLuint shaderProgramObject_PF_sphere = 0;
enum 
{
    SSG_ATTRIBUTE_NORMAL = 0,
};

GLuint vao_sphere = 0;
GLuint vbo_sphere_position = 0;
GLuint vbo_texcoord_sphere = 0;
GLuint vbo_elements_sphere = 0;
GLuint vbo_normal_sphere = 0;

GLuint numSphereElements = 0;

GLuint modelMatrixUniform_sphere = 0;
GLuint projectionMatrixUniform_sphere = 0;
GLuint viewMatrixUniform_sphere = 0;

GLuint light_Diffused_Uniform_sphere[3];
GLuint light_Ambient_Uniform_sphere[3];
GLuint light_Position_Uniform_sphere[3];
GLuint light_Specular_Uniform_sphere[3];

GLuint material_Diffused_Uniform_sphere = 0;
GLuint material_Ambient_Uniform_sphere = 0;
GLuint material_Specular_Uniform_sphere = 0;
GLuint material_Shininess_Uniform_sphere = 0;

GLuint keyPressedUniform = 0;
Bool bLightingEnabled = False;

struct Light
{
    vec4 ambient;
    vec4 diffused;
    vec4 specular;
    vec4 position;
};

struct Light light[3];

GLfloat materialDiffused[] = { 1.0f, 1.0f, 1.0f, 1.0f };   
GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess = 128.0f;

GLfloat light_angle0 = 0.0f;
GLfloat light_angle1 = 0.0f;
GLfloat light_angle2 = 0.0f;
GLfloat SPEED = 0.5f;

char light_type = 'v';

mat4 perspectiveProjectionMatrix_sphere;

// FBO related variables
GLuint FBO;
GLuint RBO;
GLuint texture_FBO;
Bool FBO_Result = False;

int main(void)
{
    // Local function declarations
    void toggleFullScreen(void);
    int initialize_cube(void);
    void uninitialize_cube(void);
    void drawDisplay_cube(void);
    void resize_cube(int,int);
    void update_cube(void);

    // Local variable declaration
    int defaultScreen;
    int styleMask;
    int frameBufferAttributes[] = { 
                                    GLX_X_RENDERABLE, True,
                                    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                                    GLX_RENDER_TYPE, GLX_RGBA_BIT,
                                    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,   

                                    GLX_RED_SIZE, 8,
                                    GLX_GREEN_SIZE, 8,
                                    GLX_BLUE_SIZE , 8,
                                    GLX_ALPHA_SIZE, 8,
                                    GLX_DEPTH_SIZE, 24,
                                    GLX_STENCIL_SIZE, 8,
                                    GLX_DOUBLEBUFFER, True, 
                                    None 
                                  };
    char keys[26];
    Bool bDone = False;
    XSetWindowAttributes windowAttributes;
    Atom windowManagerDelete;
    XEvent event;
    KeySym keySym;

    int screenWidth;
    int screenHeight;
    
    // PP related variables
    GLXFBConfig *glxFBConfigs = NULL;
    GLXFBConfig bestglxFBConfig;
    XVisualInfo *tempeXVisualInfo = NULL;
    int numFBConfig;

    int bestFramebufferConfig = -1;
    int bestNumberOfSamples = -1;
    int worstNumberOfSamples = -1;
    int worstFramebufferConfig = 999;
    int sampleBuffers, samples;
    int i;

    // code
    // File I/O code
    gpFile = fopen("Log.txt","w");
    if(gpFile == NULL)
    {
        printf("Log file can not be created\n");
        uninitialize_cube();
        exit(-1);
    }
    else
    {
        fprintf(gpFile,"Log file created successfuly\n");
    }

    display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        printf("XOpenDisplay() failed \n");

        uninitialize_cube();
        exit(-1);
    }
    
    defaultScreen =  XDefaultScreen(display);
    
    // Get available FBConfig from X-Windows
    glxFBConfigs = glXChooseFBConfig(display, XDefaultScreen(display),frameBufferAttributes, &numFBConfig);
    if(glxFBConfigs == NULL)
    {
        fprintf(gpFile,"Matching glXFBConfig cannot be found\n");
        uninitialize_cube();
        exit(-1);
    }

 

    fprintf(gpFile,"%d matching FBConfig found\n",numFBConfig);

    // Find best matching FBConfig from above array
    for(i = 0; i < numFBConfig; i++)
    {
        tempeXVisualInfo = glXGetVisualFromFBConfig(display, glxFBConfigs[i]);
        if(tempeXVisualInfo != NULL)
        {
            // Get sample buffers
            glXGetFBConfigAttrib(display, glxFBConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);

            // Get samples
            glXGetFBConfigAttrib(display, glxFBConfigs[i], GLX_SAMPLES, &samples);

            if(bestFramebufferConfig < 0 || sampleBuffers && samples > bestNumberOfSamples)
            {
                bestFramebufferConfig = i;
                bestNumberOfSamples = samples;
            }

            if(worstFramebufferConfig < 0 || !sampleBuffers || samples < worstNumberOfSamples)
            {
                worstFramebufferConfig = i;
                worstNumberOfSamples = samples;
            }

            XFree(tempeXVisualInfo);
        }
    }

    // Accordingly get bestGLXFBConfig
    bestglxFBConfig = glxFBConfigs[bestFramebufferConfig];

    // Assign this found to best to golbal glXFBConfig
    glxFBConfig = bestglxFBConfig;

    // Free the memory given to array
    XFree(glxFBConfigs);

    // Now get best visula from bestFBConfig
    visualInfo = glXGetVisualFromFBConfig(display, bestglxFBConfig);
    fprintf(gpFile,"The choosen visuals id is = 0x%lu\n",visualInfo->visualid);

    memset((void*) &windowAttributes, 0, sizeof(windowAttributes));
    windowAttributes.border_pixel = 0;
    windowAttributes.background_pixel = XBlackPixel(display, visualInfo->screen);
    windowAttributes.background_pixmap = 0;
    windowAttributes.colormap = XCreateColormap(display, 
                                            XRootWindow(display,visualInfo->screen),
                                            visualInfo->visual, 
                                            AllocNone);
    colormap = windowAttributes.colormap;

    // set the style of Window
    styleMask = CWBorderPixel | CWBackPixel | CWColormap | CWEventMask ;

    // now finally create window 
    window = XCreateWindow(display, 
                            XRootWindow(display, visualInfo->screen),
                            0,
                            0,
                            WIN_WIDTH,
                            WIN_HEIGHT,
                            0,
                            visualInfo->depth,
                            InputOutput,
                            visualInfo->visual,
                            styleMask,
                            &windowAttributes);
    if(!window)
    {
        printf("XCreate() function failed \n");
        uninitialize_cube();
        exit(-1);
    }

    XSelectInput(display, window, 
                                ExposureMask |
                                VisibilityChangeMask |
                                StructureNotifyMask |
                                KeyPressMask |
                                ButtonPressMask |
                                PointerMotionMask |
                                FocusChangeMask );
    windowManagerDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(display, window, &windowManagerDelete, 1);
    XStoreName(display, window, "Sandip Sham Gudle");

    // Show map the window 
    XMapWindow(display, window);

    // Center the window
    screenWidth = XWidthOfScreen(XScreenOfDisplay(display, visualInfo->screen));
    screenHeight = XHeightOfScreen(XScreenOfDisplay(display, visualInfo->screen));

    XMoveWindow(display, window, (screenWidth - WIN_WIDTH) / 2, (screenHeight - WIN_HEIGHT) / 2 );
    
    // OpenGL Initialization
    initialize_cube();
    
    // Game Loop
    while(bDone == False)
    {
        while(XPending(display))
        {   
            memset((void*)&event, 0, sizeof(XEvent));
            XNextEvent(display, &event);
            switch(event.type)
            {     
                case MapNotify : 
                    break;
            
                case FocusIn :
                    bActiveWindow = True;  
                    break;
            
                case FocusOut :
                    bActiveWindow = False;  
                    break;
            
                case ConfigureNotify :
                    resize_cube(event.xconfigure.width, event.xconfigure.height);
                    break;

                case ButtonPress :
                    switch(event.xbutton.button)
                    {
                        case 1 :
                            break;
                    
                        case 2 :
                            break;
                    
                        case 3 :
                            break;

                    }
                
                    break;

                case KeyPress :
                    keySym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);

                    switch(keySym)
                    {
                        case XK_Escape :
                            if(bFullscreen == False)
                            {
                                toggleFullScreen();
                                bFullscreen = True;
                            }
                            else
                            {
                                toggleFullScreen();
                                bFullscreen = False;
                            }
                            
                        break;

                        default :
                            break;
                    }

                    XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
                    switch(keys[0])
                    {
                        case 'F':
                        case 'f':
                            light_type = 'f';
                            break;

                        case 'V':
                        case 'v':
                            light_type = 'v';
                            break;
                
                        case 'Q' :
                        case 'q' :
                            bDone = True;
                            break;

                        case 'L' :
                        case 'l' :
                            if(bLightingEnabled == False)
                            {
                                bLightingEnabled = True;
                            }
                            else
                            {
                                bLightingEnabled = False;
                            }
                        break;

                        default :
                        break;
                     
                    }
                break;

                case 33 :
                    bDone = True;
                break;

                default : 
                    break;

            }

        }

        // Rendering
        if(bActiveWindow == True)
        {
            drawDisplay_cube();

            update_cube();
        }

    }
    
    uninitialize_cube();

    return(0);

}

int initialize_cube(void)
{
    // Local function declarations
    void resize_cube(int,int);
    void uninitialize_cube(void);
    void printGLInfo(void);
    Bool create_FBO(GLint, GLint);
    Bool initialize_sphere(GLint, GLint);

    // Local variables
    int attribs_new[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                          GLX_CONTEXT_MINOR_VERSION_ARB, 6,
                          GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                          None
                        };
    
    int attribs_old[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
                          GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                          None
                        };
    

    // Code
    // Get address of function in function pointer
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
    if(glXCreateContextAttribsARB == NULL)
    {
        fprintf(gpFile,"Cannot get required function address\n");
        uninitialize_cube();
        exit(-1);
    }
    
    // Create PP compatible GLXContext
    glxContext = glXCreateContextAttribsARB(display, glxFBConfig, 0, True, attribs_new);
    if(!glxContext)
    {
        fprintf(gpFile,"Core profile based glXContext can not be obtained. \nFalling back to old context\n");

        // Getting old context
        glxContext = glXCreateContextAttribsARB(display, glxFBConfig, 0, True, attribs_old);
        if(!glxContext)
        {
            fprintf(gpFile,"Old glxContext can not be found \n");
            uninitialize_cube();
            exit(-1);
        }
        else
        {
            fprintf(gpFile,"Old glxContext found\n");
        }
    }
    else
    {
        fprintf(gpFile,"Core profile glxContext obtained successfuly\n");
    }

    // Is the context supports direct rendering 
    if(!glXIsDirect(display, glxContext))
    {
        fprintf(gpFile,"Not suppporting direct rendering or hardware rendering\n");
    }
    else
    {
        fprintf(gpFile,"Suppporting direct rendering or hardware rendering\n");
    }

    // Make this context as current context 
    glXMakeCurrent(display, window, glxContext);
    if(glXMakeCurrent(display, window, glxContext) == False)
    {
        printf(" glxMakeCurrent() failed\n");
        uninitialize_cube();
        exit(-1);
    }

    // Initialize GLEW 
    if(glewInit()!= GLEW_OK)
    {
        fprintf(gpFile,"Failed to initialize GLEW\n");
        uninitialize_cube();
        exit(-1);
    }

    // PrintGLInfo 
    printGLInfo();

    // Vertex Shaders
    const GLchar* vertexShaderSourceCode =  "#version 460 core\n" \
                                            "\n" \
                                            "in vec4 aPosition;\n" \
                                            "in vec2 aTexCoord;\n" \
                                            "out vec2 oTexCoord;\n" \
                                            "uniform mat4 uMVPMatrix;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "gl_Position = uMVPMatrix * aPosition;\n" \
                                            "oTexCoord = aTexCoord;\n" \
                                            "}\n";

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
        uninitialize_cube();
    }

    /* Fragment Shader */
    const GLchar* fragmentShaderSourceCode = "#version 460 core\n" \
                                            "\n" \
                                            "in vec2 oTexCoord;\n" \
                                            "uniform sampler2D uTextureSampler;\n" \
                                            "out vec4 FragColor;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "FragColor = texture(uTextureSampler, oTexCoord);\n" \
                                            "}\n";
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
        uninitialize_cube();
    }

    /* Shader Program */
    shaderProgramObject_cube = glCreateProgram();
    glAttachShader(shaderProgramObject_cube, vertexShaderObject);
    glAttachShader(shaderProgramObject_cube, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject_cube, SSG_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject_cube, SSG_ATTRIBUTE_TEXCOORD, "aTexCoord");
    glLinkProgram(shaderProgramObject_cube);

    status = 0;
    infoLoglength = 0;
    szInfoLog = 0;

    glGetProgramiv(shaderProgramObject_cube, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        glGetProgramiv( shaderProgramObject_cube, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {
            szInfoLog = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObject_cube, infoLoglength, NULL,szInfoLog);
                fprintf(gpFile,"shader program linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialize_cube();
    }

    /* Get Shader Uniform Locations */
    mvpMatrixUniform_cube = glGetUniformLocation(shaderProgramObject_cube, "uMVPMatrix");
    textureSamplerUniform_cube = glGetUniformLocation(shaderProgramObject_cube, "uTextureSampler");

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
	                                    1.0f, 1.0f, 1.0f, // top-right of front
	                                    1.0f, 1.0f, 1.0f, // top-left of front
	                                    1.0f, 1.0f, 1.0f, // bottom-left of front
	                                    1.0f, 1.0f, 1.0f, // bottom-right of front

                                    	// right
	                                    1.0f, 1.0f, 1.0f, // top-right of right
	                                    1.0f, 1.0f, 1.0f, // top-left of right
	                                    1.0f, 1.0f, 1.0f, // bottom-left of right
	                                    1.0f, 1.0f, 1.0f, // bottom-right of right

                                    	// back
	                                    1.0f, 1.0f, 1.0f, // top-right of back
	                                    1.0f, 1.0f, 1.0f, // top-left of back
	                                    1.0f, 1.0f, 1.0f, // bottom-left of back
	                                    1.0f, 1.0f, 1.0f, // bottom-right of back

	                                    // left
	                                    1.0f, 1.0f, 1.0f, // top-right of left
	                                    1.0f, 1.0f, 1.0f, // top-left of left
	                                    1.0f, 1.0f, 1.0f, // bottom-left of left
	                                    1.0f, 1.0f, 1.0f, // bottom-right of left

	                                    // top
	                                    1.0f, 1.0f, 1.0f, // top-right of top
	                                    1.0f, 1.0f, 1.0f, // top-left of top
	                                    1.0f, 1.0f, 1.0f, // bottom-left of top
	                                    1.0f, 1.0f, 1.0f, // bottom-right of top

	                                    // bottom
	                                    1.0f, 1.0f, 1.0f, // top-right of bottom
	                                    1.0f, 1.0f, 1.0f, // top-left of bottom
	                                    1.0f, 1.0f, 1.0f, // bottom-left of bottom
	                                    1.0f, 1.0f, 1.0f, // bottom-right of bottom

                                    };

    const GLfloat cubeTexcoords[] =
                                        {
	                                        // front
	                                        1.0f, 1.0f, // top-right of front
	                                        0.0f, 1.0f, // top-left of front
	                                        0.0f, 0.0f, // bottom-left of front
	                                        1.0f, 0.0f, // bottom-right of front

	                                        // right
	                                        1.0f, 1.0f, // top-right of right
	                                        0.0f, 1.0f, // top-left of right
	                                        0.0f, 0.0f, // bottom-left of right
	                                        1.0f, 0.0f, // bottom-right of right

	                                        // back
	                                        1.0f, 1.0f, // top-right of back
	                                        0.0f, 1.0f, // top-left of back
	                                        0.0f, 0.0f, // bottom-left of back
	                                        1.0f, 0.0f, // bottom-right of back

	                                        // left
	                                        1.0f, 1.0f, // top-right of left
	                                        0.0f, 1.0f, // top-left of left
	                                        0.0f, 0.0f, // bottom-left of left
	                                        1.0f, 0.0f, // bottom-right of left

	                                        // top
	                                        1.0f, 1.0f, // top-right of top
	                                        0.0f, 1.0f, // top-left of top
	                                        0.0f, 0.0f, // bottom-left of top
	                                        1.0f, 0.0f, // bottom-right of top

	                                        // bottom
	                                        1.0f, 1.0f, // top-right of bottom
	                                        0.0f, 1.0f, // top-left of bottom
	                                        0.0f, 0.0f, // bottom-left of bottom
	                                        1.0f, 0.0f, // bottom-right of bottom
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

     // VBO for cube texcoord
    glGenBuffers(1, &vbo_texcoord_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTexcoords), cubeTexcoords, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_TEXCOORD);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    
    // Enabling Depth
    glClearDepth(1.0f);         // Compulsory
    glEnable(GL_DEPTH_TEST);    // Compulsory
    glDepthFunc(GL_LEQUAL);     // Compulsaory

    // Set the clear of window to blue
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);       // here OpenGL starts

    /* Initialize Orthographicprojectionmatrix */
    perspectiveProjectionMatrix_cube = vmath::mat4::identity();

    // Tell OpenGL to enable texture
    glEnable(GL_TEXTURE_2D);
    resize_cube(WIN_WIDTH,WIN_HEIGHT);

    // FBO realated code 
    if(create_FBO(FBO_WIDTH, FBO_HEIGHT) == True )
    {
        FBO_Result = initialize_sphere(FBO_WIDTH,FBO_HEIGHT);
    }

    return(0);
}

Bool initialize_sphere(GLint textureWidth, GLint textureHeight)
{
    
    // Function Declarations 
    void resize_sphere(int,int);
    void uninitialize_sphere(void);

    // Per Vertex Light //
    // Vertex Shaders 
    const GLchar* vertexShaderSourceCode_PV =  "#version 460 core\n" \
                                            "\n" \
                                            "in vec4 aPosition;\n" \
                                            "in vec3 aNormal;\n" \
                                            "\n" \
                                            "uniform mat4 uViewMatrix;\n" \
                                            "uniform mat4 uModelMatrix;\n" \
                                            "uniform mat4 uProjectionMatrix;\n" \
                                            "\n" \
                                            "uniform vec3 uLight_Ambient[3];\n" \
                                            "uniform vec3 uLight_Specular[3];\n" \
                                            "uniform vec3 uLight_Diffused[3];\n" \
                                            "uniform vec4 uLight_Position[3];\n" \
                                            "\n" \
                                            "uniform vec3 uMaterial_Diffused;\n" \
                                            "uniform vec3 uMaterial_Ambient;\n" \
                                            "uniform vec3 uMaterial_Specular;\n" \
                                            "uniform float uMaterial_Shininess;\n" \
                                            "\n" \
                                            "uniform int uKeyPressed;\n" \
                                            "out vec3 oPhong_ADS_Light;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "if(uKeyPressed == 1)\n" \
                                            "{\n" \
                                            "vec4 eye_Coordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
                                            "vec3 transformed_Normals = normalize(mat3(uViewMatrix * uModelMatrix) * aNormal);\n" \
                                            "vec3 lightAmbient[3];\n" \
                                            "vec3 lightDiffused[3];\n" \
                                            "vec3 lightSpecular[3];\n" \
                                            "vec3 reflection_Vector[3];\n" \
                                            "vec3 light_Direction[3];\n" \
                                            "light_Direction[0] = normalize(vec3(uLight_Position[0] - eye_Coordinates));\n" \
                                            "light_Direction[1] = normalize(vec3(uLight_Position[1] - eye_Coordinates));\n" \
                                            "light_Direction[2] = normalize(vec3(uLight_Position[2] - eye_Coordinates));\n" \
                                            "reflection_Vector[0] = reflect(-light_Direction[0], transformed_Normals);\n" \
                                            "reflection_Vector[1] = reflect(-light_Direction[1], transformed_Normals);\n" \
                                            "reflection_Vector[2] = reflect(-light_Direction[2], transformed_Normals);\n" \
                                            "vec3 viewer_Vector = normalize(-eye_Coordinates.xyz);\n" \
                                            "lightAmbient[0] = uLight_Ambient[0] * uMaterial_Ambient;\n" \
                                            "lightAmbient[1] = uLight_Ambient[1] * uMaterial_Ambient;\n" \
                                            "lightAmbient[2] = uLight_Ambient[2] * uMaterial_Ambient;\n" \
                                            "lightDiffused[0] = uLight_Diffused[0] * uMaterial_Diffused * max(dot(light_Direction[0], transformed_Normals), 0.0);\n" \
                                            "lightDiffused[1] = uLight_Diffused[1] * uMaterial_Diffused * max(dot(light_Direction[1], transformed_Normals), 0.0);\n" \
                                            "lightDiffused[2] = uLight_Diffused[2] * uMaterial_Diffused * max(dot(light_Direction[2], transformed_Normals), 0.0);\n" \
                                            "lightSpecular[0] = uLight_Specular[0] * uMaterial_Specular * pow(max(dot(reflection_Vector[0], viewer_Vector), 0.0f), uMaterial_Shininess );\n" \
                                            "lightSpecular[1] = uLight_Specular[1] * uMaterial_Specular * pow(max(dot(reflection_Vector[1], viewer_Vector), 0.0f), uMaterial_Shininess );\n" \
                                            "lightSpecular[2] = uLight_Specular[2] * uMaterial_Specular * pow(max(dot(reflection_Vector[2], viewer_Vector), 0.0f), uMaterial_Shininess );\n" \
                                            "oPhong_ADS_Light = lightAmbient[0] + lightDiffused[0] + lightSpecular[0];\n" \
                                            "oPhong_ADS_Light = oPhong_ADS_Light + lightAmbient[1] + lightDiffused[1] + lightSpecular[1];\n" \
                                            "oPhong_ADS_Light = oPhong_ADS_Light + lightAmbient[2] + lightDiffused[2] + lightSpecular[2];\n" \
                                            "}\n" \
                                            "else\n" \
                                            "{\n" \
                                            "oPhong_ADS_Light = vec3(0.0f, 0.0f, 0.0f);\n" \
                                            "}\n" \
                                            "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
                                            "}\n";

    GLuint vertexShaderObject_PV = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject_PV, 1, (const GLchar**)&vertexShaderSourceCode_PV, NULL);
    glCompileShader(vertexShaderObject_PV);

    GLint status = 0;
    GLint infoLoglength = 0;
    char *szInfoLog = NULL;

    glGetShaderiv(vertexShaderObject_PV, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE )
    {
        glGetShaderiv(vertexShaderObject_PV, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {

            szInfoLog = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetShaderInfoLog(vertexShaderObject_PV, infoLoglength, NULL, szInfoLog);
                fprintf(gpFile,"Vertex shader complilation(per vertex) : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize_sphere();
    }


    /* Fragment Shader */
    const GLchar* fragmentShaderSourceCode_PV = "#version 460 core\n" \
                                            "\n" \
                                            "in vec3 oPhong_ADS_Light;\n" \
                                            "uniform int uKeyPressed;\n" \
                                            "out vec4 FragColor;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "if(uKeyPressed == 1)\n" \
                                            "{\n" \
                                            "FragColor = vec4(oPhong_ADS_Light, 1.0);\n" \
                                            "}\n" \
                                            "else\n" \
                                            "{\n" \
                                            "FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" \
                                            "}\n" \
                                            "}\n";

    GLuint fragmentShaderObject_PV = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObject_PV,1, (const GLchar**)&fragmentShaderSourceCode_PV, NULL);
    glCompileShader(fragmentShaderObject_PV);

    status = 0;
    infoLoglength = 0;
    szInfoLog = 0;

    glGetShaderiv(fragmentShaderObject_PV, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject_PV, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {
            szInfoLog  = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetShaderInfoLog(fragmentShaderObject_PV, infoLoglength, NULL, szInfoLog);
                fprintf(gpFile,"fragment shader compilation(per vertex) : %s \n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize_sphere();
    }

    // Per Fragment Light
    // Vertex Shaders 
    const GLchar* vertexShaderSourceCode_PF =  "#version 460 core\n" \
                                            "\n" \
                                            "in vec4 aPosition;\n" \
                                            "in vec3 aNormal;\n" \
                                            "\n" \
                                            "uniform mat4 uViewMatrix;\n" \
                                            "uniform mat4 uModelMatrix;\n" \
                                            "uniform mat4 uProjectionMatrix;\n" \
                                            "uniform vec4 uLight_Position[3];\n" \
                                            "\n" \
                                            "uniform int uKeyPressed;\n" \
                                            "out vec3 oTransformed_Normals;\n" \
                                            "out vec3 oLight_Direction[3];\n" \
                                            "out vec3 oViewer_Vector;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "if(uKeyPressed == 1)\n" \
                                            "{\n" \
                                            "vec4 eye_Coordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
                                            "oTransformed_Normals = mat3(uViewMatrix * uModelMatrix) * aNormal;\n" \
                                            "oLight_Direction[0] = vec3(uLight_Position[0] - eye_Coordinates);\n" \
                                            "oLight_Direction[1] = vec3(uLight_Position[1] - eye_Coordinates);\n" \
                                            "oLight_Direction[2] = vec3(uLight_Position[2] - eye_Coordinates);\n" \
                                            "oViewer_Vector = -eye_Coordinates.xyz;\n" \
                                            "}\n" \
                                            "else\n" \
                                            "{\n" \
                                            "oTransformed_Normals = vec3(0.0, 0.0, 0.0);\n" \
                                            "oLight_Direction[0] = vec3(0.0, 0.0, 0.0);\n" \
                                            "oLight_Direction[1] = vec3(0.0, 0.0, 0.0);\n" \
                                            "oLight_Direction[2] = vec3(0.0, 0.0, 0.0);\n" \
                                            "oViewer_Vector = vec3(0.0, 0.0, 0.0);\n" \
                                            "}\n" \
                                            "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
                                            "}\n";

    GLuint vertexShaderObject_PF = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject_PF, 1, (const GLchar**)&vertexShaderSourceCode_PF, NULL);
    glCompileShader(vertexShaderObject_PF);

    status = 0;
    infoLoglength = 0;
    szInfoLog = NULL;

    glGetShaderiv(vertexShaderObject_PF, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE )
    {
        glGetShaderiv(vertexShaderObject_PF, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {

            szInfoLog = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetShaderInfoLog(vertexShaderObject_PF, infoLoglength, NULL, szInfoLog);
                fprintf(gpFile,"Vertex shader complilation(per fragment) : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize_sphere();
    }

    /* Fragment Shader */
    const GLchar* fragmentShaderSourceCode_PF = "#version 460 core\n" \
                                            "\n" \
                                            "in vec3 oTransformed_Normals;\n" \
                                            "in vec3 oLight_Direction[3];\n" \
                                            "in vec3 oViewer_Vector;\n" \
                                            "\n" \
                                            "uniform vec3 uLight_Ambient[3];\n" \
                                            "uniform vec3 uLight_Specular[3];\n" \
                                            "uniform vec3 uLight_Diffused[3];\n" \
                                            "\n" \
                                            "uniform vec3 uMaterial_Diffused;\n" \
                                            "uniform vec3 uMaterial_Ambient;\n" \
                                            "uniform vec3 uMaterial_Specular;\n" \
                                            "uniform float uMaterial_Shininess;\n" \
                                            "\n" \
                                            "uniform int uKeyPressed;\n" \
                                            "out vec4 FragColor;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "vec3 phong_ADS_Light;\n" \
                                            "if(uKeyPressed == 1)\n" \
                                            "{\n" \
                                            "vec3 normalized_Light_Direction[3];\n" \
                                            "vec3 lightDiffused[3];\n" \
                                            "vec3 lightAmbient[3];\n" \
                                            "vec3 reflection_Vector[3];\n" \
                                            "vec3 lightSpecular[3];\n" \
                                            "vec3 normalized_Transformed_Normals = normalize(oTransformed_Normals);\n" \
                                            "normalized_Light_Direction[0] = normalize(oLight_Direction[0]);\n" \
                                            "normalized_Light_Direction[1] = normalize(oLight_Direction[1]);\n" \
                                            "normalized_Light_Direction[2] = normalize(oLight_Direction[2]);\n" \
                                            "vec3 normalized_Viewer_Vector = normalize(oViewer_Vector);\n" \
                                            "\n" \
                                            "lightAmbient[0] = uLight_Ambient[0] * uMaterial_Ambient;\n" \
                                            "lightAmbient[1] = uLight_Ambient[1] * uMaterial_Ambient;\n" \
                                            "lightAmbient[2] = uLight_Ambient[2] * uMaterial_Ambient;\n" \
                                            "lightDiffused[0] = uLight_Diffused[0] * uMaterial_Diffused * max(dot(normalized_Light_Direction[0], normalized_Transformed_Normals), 0.0);\n" \
                                            "lightDiffused[1] = uLight_Diffused[1] * uMaterial_Diffused * max(dot(normalized_Light_Direction[1], normalized_Transformed_Normals), 0.0);\n" \
                                            "lightDiffused[2] = uLight_Diffused[2] * uMaterial_Diffused * max(dot(normalized_Light_Direction[2], normalized_Transformed_Normals), 0.0);\n" \
                                            "reflection_Vector[0] = reflect(-normalized_Light_Direction[0], normalized_Transformed_Normals);\n" \
                                            "reflection_Vector[1] = reflect(-normalized_Light_Direction[1], normalized_Transformed_Normals);\n" \
                                            "reflection_Vector[2] = reflect(-normalized_Light_Direction[2], normalized_Transformed_Normals);\n" \
                                            "lightSpecular[0] = uLight_Specular[0] * uMaterial_Specular * pow(max(dot(reflection_Vector[0], normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n" \
                                            "lightSpecular[1] = uLight_Specular[1] * uMaterial_Specular * pow(max(dot(reflection_Vector[1], normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n" \
                                            "lightSpecular[2] = uLight_Specular[2] * uMaterial_Specular * pow(max(dot(reflection_Vector[2], normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n" \
                                            "phong_ADS_Light = lightAmbient[0] + lightDiffused[0] + lightSpecular[0];\n" \
                                            "phong_ADS_Light = phong_ADS_Light + lightAmbient[1] + lightDiffused[1] + lightSpecular[1];\n" \
                                            "phong_ADS_Light = phong_ADS_Light + lightAmbient[2] + lightDiffused[2] + lightSpecular[2];\n" \
                                            "}\n" \
                                            "else\n" \
                                            "{\n" \
                                            "phong_ADS_Light = vec3(1.0, 1.0, 1.0);\n" \
                                            "}\n" \
                                            "FragColor = vec4(phong_ADS_Light, 1.0);\n" \
                                            "}\n";
    
    GLuint fragmentShaderObject_PF = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObject_PF,1, (const GLchar**)&fragmentShaderSourceCode_PF, NULL);
    glCompileShader(fragmentShaderObject_PF);

    status = 0;
    infoLoglength = 0;
    szInfoLog = 0;

    glGetShaderiv(fragmentShaderObject_PF, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject_PF, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {
            szInfoLog  = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetShaderInfoLog(fragmentShaderObject_PF, infoLoglength, NULL, szInfoLog);
                fprintf(gpFile,"fragment shader compilation(per fragment) : %s \n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize_sphere();
    }

    //Shader Program for Per Vertex 
    shaderProgramObject_PV_sphere = glCreateProgram();
    glAttachShader(shaderProgramObject_PV_sphere, vertexShaderObject_PV);
    glAttachShader(shaderProgramObject_PV_sphere, fragmentShaderObject_PV);
    glBindAttribLocation(shaderProgramObject_PV_sphere, SSG_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject_PV_sphere, SSG_ATTRIBUTE_NORMAL, "aNormal");
    glLinkProgram(shaderProgramObject_PV_sphere);

    status = 0;
    infoLoglength = 0;
    szInfoLog = 0;

    glGetProgramiv(shaderProgramObject_PV_sphere, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        glGetProgramiv( shaderProgramObject_PV_sphere, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {
            szInfoLog = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObject_PV_sphere, infoLoglength, NULL,szInfoLog);
                fprintf(gpFile,"shader program(per vertex) linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialize_sphere();
    }

    /* Shader Program for Per Fragment */
    shaderProgramObject_PF_sphere = glCreateProgram();
    glAttachShader(shaderProgramObject_PF_sphere, vertexShaderObject_PF);
    glAttachShader(shaderProgramObject_PF_sphere, fragmentShaderObject_PF);
    glBindAttribLocation(shaderProgramObject_PF_sphere, SSG_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject_PF_sphere, SSG_ATTRIBUTE_NORMAL, "aNormal");
    glLinkProgram(shaderProgramObject_PF_sphere);

    status = 0;
    infoLoglength = 0;
    szInfoLog = 0;

    glGetProgramiv(shaderProgramObject_PF_sphere, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        glGetProgramiv( shaderProgramObject_PF_sphere, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {
            szInfoLog = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObject_PF_sphere, infoLoglength, NULL,szInfoLog);
                fprintf(gpFile,"shader program(fragment) linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialize_sphere();
    }

    /* Get Shader Uniform Locations  for Per Vertex */
    modelMatrixUniform_sphere = glGetUniformLocation(shaderProgramObject_PV_sphere, "uModelMatrix");
    viewMatrixUniform_sphere = glGetUniformLocation(shaderProgramObject_PV_sphere, "uViewMatrix");
    projectionMatrixUniform_sphere = glGetUniformLocation(shaderProgramObject_PV_sphere, "uProjectionMatrix");

    light_Diffused_Uniform_sphere[0] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Diffused[0]");
    light_Ambient_Uniform_sphere[0] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Ambient[0]");
    light_Specular_Uniform_sphere[0] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Specular[0]");
    light_Position_Uniform_sphere[0] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Position[0]");

    light_Diffused_Uniform_sphere[1] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Diffused[1]");
    light_Ambient_Uniform_sphere[1] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Ambient[1]");
    light_Specular_Uniform_sphere[1] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Specular[1]");
    light_Position_Uniform_sphere[1] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Position[1]");

    light_Diffused_Uniform_sphere[2] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Diffused[2]");
    light_Ambient_Uniform_sphere[2] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Ambient[2]");
    light_Specular_Uniform_sphere[2] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Specular[2]");
    light_Position_Uniform_sphere[2] = glGetUniformLocation(shaderProgramObject_PV_sphere, "uLight_Position[2]");

    material_Diffused_Uniform_sphere = glGetUniformLocation(shaderProgramObject_PV_sphere, "uMaterial_Diffused");
    material_Ambient_Uniform_sphere = glGetUniformLocation(shaderProgramObject_PV_sphere, "uMaterial_Ambient");
    material_Specular_Uniform_sphere = glGetUniformLocation(shaderProgramObject_PV_sphere, "uMaterial_Specular");
    material_Shininess_Uniform_sphere = glGetUniformLocation(shaderProgramObject_PV_sphere, "uMaterial_Shininess");
    keyPressedUniform = glGetUniformLocation(shaderProgramObject_PV_sphere, "uKeyPressed");

    /* Get Shader Uniform Locations for Per Fragment */
    modelMatrixUniform_sphere = glGetUniformLocation(shaderProgramObject_PF_sphere, "uModelMatrix");
    viewMatrixUniform_sphere = glGetUniformLocation(shaderProgramObject_PF_sphere, "uViewMatrix");
    projectionMatrixUniform_sphere = glGetUniformLocation(shaderProgramObject_PF_sphere, "uProjectionMatrix");

    light_Diffused_Uniform_sphere[0] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Diffused[0]");
    light_Ambient_Uniform_sphere[0] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Ambient[0]");
    light_Specular_Uniform_sphere[0] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Specular[0]");
    light_Position_Uniform_sphere[0] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Position[0]");

    light_Diffused_Uniform_sphere[1] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Diffused[1]");
    light_Ambient_Uniform_sphere[1] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Ambient[1]");
    light_Specular_Uniform_sphere[1] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Specular[1]");
    light_Position_Uniform_sphere[1] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Position[1]");

    light_Diffused_Uniform_sphere[2] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Diffused[2]");
    light_Ambient_Uniform_sphere[2] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Ambient[2]");
    light_Specular_Uniform_sphere[2] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Specular[2]");
    light_Position_Uniform_sphere[2] = glGetUniformLocation(shaderProgramObject_PF_sphere, "uLight_Position[2]");

    material_Diffused_Uniform_sphere = glGetUniformLocation(shaderProgramObject_PF_sphere, "uMaterial_Diffused");
    material_Ambient_Uniform_sphere = glGetUniformLocation(shaderProgramObject_PF_sphere, "uMaterial_Ambient");
    material_Specular_Uniform_sphere = glGetUniformLocation(shaderProgramObject_PF_sphere, "uMaterial_Specular");
    material_Shininess_Uniform_sphere = glGetUniformLocation(shaderProgramObject_PF_sphere, "uMaterial_Shininess");
    keyPressedUniform = glGetUniformLocation(shaderProgramObject_PF_sphere, "uKeyPressed");
    
    GLfloat sphere_position[1146];
    GLfloat sphere_normal[1146];
    GLfloat sphere_texcoords[764];
    GLushort sphere_elements[2280];

    getSphereVertexData(sphere_position, sphere_normal, sphere_texcoords, sphere_elements);
    numSphereElements = getNumberOfSphereElements();

    // Sphere vao
    glGenVertexArrays(1,&vao_sphere);
    glBindVertexArray(vao_sphere);

    // Sphere
    // VBO for Position 
    glGenBuffers(1, &vbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_position), sphere_position, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO for normals
    glGenBuffers(1, &vbo_normal_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_sphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normal), sphere_normal, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO for texture
    glGenBuffers(1, &vbo_texcoord_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord_sphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_texcoords), sphere_texcoords, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE,0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_TEXCOORD);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO for elements
    glGenBuffers(1, &vbo_elements_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements_sphere);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // VAO unbind
    glBindVertexArray(0);

    // Enabling Depth
    glClearDepth(1.0f);         // Compulsory
    glEnable(GL_DEPTH_TEST);    // Compulsory
    glDepthFunc(GL_LEQUAL);     // Compulsaory

    light[0].ambient = vec4( 0.0f, 0.0f, 0.0f, 1.0 );
    light[0].diffused = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
    light[0].specular = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
    light[0].position = vec4( 0.0f, 0.0f, 0.0f, 1.0f );

    light[1].ambient = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
    light[1].diffused = vec4( 0.0f, 1.0f, 0.0f, 1.0f );
    light[1].specular = vec4( 0.0f, 1.0f, 0.0f, 1.0f );
    light[1].position = vec4( 0.0f, 0.0f, 0.0f, 1.0f );

    light[2].ambient = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
    light[2].diffused = vec4( 0.0f, 0.0f, 1.0f, 1.0f );
    light[2].specular = vec4( 0.0f, 0.0f, 1.0f, 1.0f );
    light[2].position = vec4( 0.0f, 0.0f, 0.0f, 1.0f );

    // Set the clear of window to blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // here OpenGL starts

    // Initialize Orthographicprojectionmatrix 
    perspectiveProjectionMatrix_sphere = vmath::mat4::identity();

    resize_sphere(WIN_WIDTH,WIN_HEIGHT);

    return(True);
}

void printGLInfo(void)
{
    /* valiable declarations */
    GLint numExtensions;
    GLint i;

    /* code */
    fprintf(gpFile,"OpenGL Vender : %s\n",glGetString(GL_VENDOR));
    fprintf(gpFile,"OpenGL Renderer : %s\n",glGetString(GL_RENDERER));
    fprintf(gpFile,"OpenGL Version : %s \n",glGetString(GL_VERSION));
    fprintf(gpFile,"OpenGL Shading language version(GLSL) : %s \n",glGetString(GL_SHADING_LANGUAGE_VERSION));


    /* listing of supported extensions */
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(i = 0; i < numExtensions; i++)
    {
        fprintf(gpFile,"%s\n",glGetStringi(GL_EXTENSIONS, i));
    
    }

}

Bool create_FBO(GLint textureWidth, GLint textureHeight)
{
    // Variable declarations
    GLint max_render_buffer_size;

    // Check capacity of render buffer
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_render_buffer_size);
    if(max_render_buffer_size < textureWidth || max_render_buffer_size < textureHeight )
    {
        fprintf(gpFile,"texture size over flow\n");
        return(False);
    }

    // Create custom framebuffer
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Create texture for FBO in which we are going to render 2nd scene or sphere
    glGenTextures(1, &texture_FBO);
    glBindTexture(GL_TEXTURE_2D, texture_FBO);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    
    // Attach above texture to framebuffer at color attachment0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_FBO, 0);

    // Now create renderbuffer to hold depth of custom FBO
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);

    // Set storage of above renderbuffer of texture size for depth
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth, textureHeight);

    // Attach above depth related renderbuffer to FBO at depht attachment 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // Check the framebuffer wether successfull or not 
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(gpFile,"Framebuffer status is not complete\n");
        return(False);
    }

    // Unbind with framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return(True);

}

void resize_cube(int width, int height)
{
    // Code
    if(height <= 0)
        height = 1;
    
    winWidth = width;
    winHeight = height;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    perspectiveProjectionMatrix_cube = vmath::perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
}

void resize_sphere(int width, int height)
{
    // Code
    if(height <= 0)
        height = 1;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    /* Set Perspective Projection Matrix */
    perspectiveProjectionMatrix_sphere = vmath::perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
    
}

void drawDisplay_cube(void)
{
    // Function declaration
    void display_sphere(GLint, GLint);
    void update_sphere(void);
    void resize_cube(int, int);
    void resize_sphere(int, int);

    // Code
    // Render FBO scene
    if(FBO_Result == True)
    {
        display_sphere(FBO_WIDTH, FBO_HEIGHT);
        update_sphere();
    }

    // Call resize_cube() again to compensate the change by display_sphere
    resize_cube(winWidth, winHeight);
    
    // Reset the background color to white to compensate the change done by display_sphere
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject_cube);

    /* Cube */
    mat4 modelViewMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f,-6.0f);
    mat4 scaleMatrix = mat4::identity();
    scaleMatrix = vmath::scale(0.8f,0.8f,0.8f);
    mat4 rotationMatrix1 = mat4::identity();
    rotationMatrix1 = vmath::rotate(cube_angle, 1.0f,0.0f,0.0f);
    mat4 rotationMatrix2 = mat4::identity();
    rotationMatrix2 = vmath::rotate(cube_angle, 0.0f,1.0f,0.0f);
    mat4 rotationMatrix3 = mat4::identity();
    rotationMatrix3 = vmath::rotate(cube_angle, 0.0f,0.0f,1.0f);
    mat4 rotationMatrix = rotationMatrix1 * rotationMatrix2 * rotationMatrix3;
    modelViewMatrix = translationMatrix * scaleMatrix * rotationMatrix;
    mat4 modelViewProjectionMatrix = mat4::identity();
    modelViewProjectionMatrix = perspectiveProjectionMatrix_cube * modelViewMatrix; // order of multiplication is very importatnt

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_FBO);
    glUniform1i(textureSamplerUniform_cube, 0);

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(mvpMatrixUniform_cube, 1, GL_FALSE, modelViewProjectionMatrix);
    glBindVertexArray(vao_cube);

    glDrawArrays(GL_TRIANGLE_FAN, 0 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 4 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 8 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 12 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 16 , 4 );
    glDrawArrays(GL_TRIANGLE_FAN, 20 , 4 );
    
    glBindVertexArray(0);
    glUseProgram(0);


    glXSwapBuffers(display, window);
}

void display_sphere(GLint textureWidth, GLint textureHeight)
{
    // Function declaration
    void resize_sphere(int, int);

    // Code
    // Bind with FBO
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Call resize_sphere() 
    resize_sphere(textureWidth, textureHeight);
    
    // Set the background color to black to compensate the change done by display_sphere
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // For Per Vertex Light
    if(light_type == 'v')
    {
        glUseProgram(shaderProgramObject_PV_sphere);

        // Sphere
        mat4 translationMatrix = mat4::identity();
        translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
        mat4 modelMatrix = translationMatrix;
        mat4 viewMatrix = mat4::identity();

        /* Push above mvp into vertex shaders's mvp uniform */
        glUniformMatrix4fv(modelMatrixUniform_sphere, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform_sphere, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform_sphere,1 , GL_FALSE, perspectiveProjectionMatrix_sphere);

        if(bLightingEnabled == True)
        {
            glUniform1i(keyPressedUniform, 1);
            
            glUniform3fv(light_Ambient_Uniform_sphere[0], 1, light[0].ambient);
            glUniform3fv(light_Diffused_Uniform_sphere[0], 1, light[0].diffused);
            glUniform3fv(light_Specular_Uniform_sphere[0], 1, light[0].specular);
            glUniform4fv(light_Position_Uniform_sphere[0], 1, light[0].position);

            glUniform3fv(light_Ambient_Uniform_sphere[1], 1, light[1].ambient);
            glUniform3fv(light_Diffused_Uniform_sphere[1], 1, light[1].diffused);
            glUniform3fv(light_Specular_Uniform_sphere[1], 1, light[1].specular);
            glUniform4fv(light_Position_Uniform_sphere[1], 1, light[1].position);

            glUniform3fv(light_Ambient_Uniform_sphere[2], 1, light[2].ambient);
            glUniform3fv(light_Diffused_Uniform_sphere[2], 1, light[2].diffused);
            glUniform3fv(light_Specular_Uniform_sphere[2], 1, light[2].specular);
            glUniform4fv(light_Position_Uniform_sphere[2], 1, light[2].position);

            glUniform3fv(material_Diffused_Uniform_sphere, 1, materialDiffused);
            glUniform3fv(material_Ambient_Uniform_sphere, 1, materialAmbient);
            glUniform3fv(material_Specular_Uniform_sphere, 1, materialSpecular);
            glUniform1f(material_Shininess_Uniform_sphere, materialShininess);
        }
        else
        {
            glUniform1i(keyPressedUniform, 0);
        }

        glBindVertexArray(vao_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements_sphere);
        glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        glUseProgram(0);

    }


    // For Per Fragment Light
    if(light_type == 'f' )
    {

        glUseProgram(shaderProgramObject_PF_sphere);

        // Sphere
        mat4 translationMatrix = mat4::identity();
        translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
        mat4 modelMatrix = translationMatrix;
        mat4 viewMatrix = mat4::identity();
   
        /* Push above mvp into vertex shaders's mvp uniform */
        glUniformMatrix4fv(modelMatrixUniform_sphere, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform_sphere, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform_sphere,1 , GL_FALSE, perspectiveProjectionMatrix_sphere);

        if(bLightingEnabled == True)
        {
            glUniform1i(keyPressedUniform, 1);
            
            glUniform3fv(light_Ambient_Uniform_sphere[0], 1, light[0].ambient);
            glUniform3fv(light_Diffused_Uniform_sphere[0], 1, light[0].diffused);
            glUniform3fv(light_Specular_Uniform_sphere[0], 1, light[0].specular);
            glUniform4fv(light_Position_Uniform_sphere[0], 1, light[0].position);

            glUniform3fv(light_Ambient_Uniform_sphere[1], 1, light[1].ambient);
            glUniform3fv(light_Diffused_Uniform_sphere[1], 1, light[1].diffused);
            glUniform3fv(light_Specular_Uniform_sphere[1], 1, light[1].specular);
            glUniform4fv(light_Position_Uniform_sphere[1], 1, light[1].position);

            glUniform3fv(light_Ambient_Uniform_sphere[2], 1, light[2].ambient);
            glUniform3fv(light_Diffused_Uniform_sphere[2], 1, light[2].diffused);
            glUniform3fv(light_Specular_Uniform_sphere[2], 1, light[2].specular);
            glUniform4fv(light_Position_Uniform_sphere[2], 1, light[2].position);

            glUniform3fv(material_Diffused_Uniform_sphere, 1, materialDiffused);
            glUniform3fv(material_Ambient_Uniform_sphere, 1, materialAmbient);
            glUniform3fv(material_Specular_Uniform_sphere, 1, materialSpecular);
            glUniform1f(material_Shininess_Uniform_sphere, materialShininess);
        }
        else
        {
            glUniform1i(keyPressedUniform, 0);
        }

        glBindVertexArray(vao_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements_sphere);
        glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        glUseProgram(0);

    }

    // Unbind with framebuffer 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void update_cube(void)
{
    // Code 
    cube_angle = cube_angle - 0.5f;
    if(cube_angle <= 0.0f)
        cube_angle = cube_angle + 360.0f;
}

void update_sphere(void)
{
    // Code
    if(bLightingEnabled == True && light_type == 'v' || light_type == 'f')
    {   
        light[0].position[0] = 0.0f; 
        light[0].position[1] = 25.0f * (GLfloat)cos(light_angle0 * M_PI / 180.0f); 
        light[0].position[2] = 25.0f * (GLfloat)sin(light_angle0 * M_PI / 180.0f); 
        light[0].position[3] = 1.0f;
 
        light_angle0 = light_angle0 + SPEED;
        if(light_angle0 > 360.0f)
            light_angle0 = light_angle0 - 360.0f;
        
        light[1].position[0] = 10.0f * (GLfloat)cos(light_angle1 * M_PI / 180.0f);
        light[1].position[1] = 0.0f; 
        light[1].position[2] = 10.0f * (GLfloat)sin(light_angle1 * M_PI / 180.0f);
        light[1].position[3] = 1.0f;

        light_angle1 = light_angle1 + SPEED;
        if(light_angle1 > 360.0f)
            light_angle1 = light_angle1 - 360.0f;
       
        light[2].position[0] = 10.0f * (GLfloat)sin(light_angle2 * M_PI / 180.0f);
        light[2].position[1] = 10.0f * (GLfloat)cos(light_angle2 * M_PI / 180.0f);
        light[2].position[2] = 0.0f;
        light[2].position[3] = 1.0f;

        light_angle2 = light_angle2 + SPEED ;
        if(light_angle2 > 360.0f)
            light_angle2 = light_angle2 - 360.0f;
    }
}

void toggleFullScreen(void)
{
    // Local variable declarations
    Atom windowManagerStateNormal;
    Atom windowManagerStateFullScreen;  // 'wpPrev' in win32
    XEvent event;
    
    // Code
    windowManagerStateNormal = XInternAtom(display, "_NET_WM_STATE", False);
    windowManagerStateFullScreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN" , False);

    // Memset event structure
    memset((void*)&event, 0, sizeof(XEvent));  
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = windowManagerStateNormal;
    event.xclient.format = 32;     // 'msg struture'  in win32
    event.xclient.data.l[0] = bFullscreen ? 0 : 1;
    event.xclient.data.l[1] = windowManagerStateFullScreen;

    // X-Send event
    XSendEvent(display ,
                XRootWindow(display, visualInfo->screen),
                False ,
                SubstructureNotifyMask ,
                &event );

}

void uninitialize_cube(void)
{
    // Function Declarations
    void ToggleFullscreen(void);
    void uninitialize_sphere(void);

    // Local variable declarations
    GLXContext currentGlxContext = NULL;

    // Code
    if(shaderProgramObject_cube)
    {
        glUseProgram(shaderProgramObject_cube);
        
        GLint numShaders = 0;

        glGetProgramiv(shaderProgramObject_cube, GL_ATTACHED_SHADERS, &numShaders);
        if(numShaders > 0)
        {
            GLuint *pShaders = (GLuint*)malloc(numShaders * sizeof(GLuint));
            if(pShaders != NULL)
            {
                glGetAttachedShaders(shaderProgramObject_cube, numShaders, NULL, pShaders);
            
                for(GLint i = 0; i < numShaders; i++)
                {
                    glDetachShader(shaderProgramObject_cube, pShaders[i]);
                    glDeleteShader(pShaders[i] );
                    pShaders[i] = 0;
                }
        
                free(pShaders);
                pShaders = NULL;

            }
        }

        glUseProgram(0);
        glDeleteProgram(shaderProgramObject_cube);
        shaderProgramObject_cube = 0;

    }

    // Delete texture texcords
    if(vbo_texcoord_cube)
    {
        glDeleteBuffers(1, &vbo_texcoord_cube);
        vbo_texcoord_cube = 0;
    }

    // DELETE VBO of cube Color 
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

    // Delete VAO_square 
    if(vao_cube)
    {
        glDeleteVertexArrays(1,&vao_cube);
        vao_cube = 0;
    }

    /* DELETE VAO_cube */
    if(vao_cube)
    {
        glDeleteVertexArrays(1,&vao_cube);
        vao_cube = 0;
    }

    if(texture_FBO)
    {
        glDeleteTextures(1, &texture_FBO);
        texture_FBO = 0;
    }

    if(RBO)
    {
        glDeleteRenderbuffers(1, &RBO);
        RBO = 0;
    }

    if(FBO)
    {
        glDeleteFramebuffers(1, &FBO);
        FBO = 0;
    }

    // code 
    if(visualInfo)
    {
        free(visualInfo);
        visualInfo = NULL;
    }

    currentGlxContext = glXGetCurrentContext();
    if(currentGlxContext != NULL && currentGlxContext == glxContext )
    {
        glXMakeCurrent(display, 0,0);
    }

    if(glxContext)
    {
        glXDestroyContext(display, glxContext);
        glxContext = NULL;
    }  

    if(window)
    {
        XDestroyWindow(display, window);
    }

    if(colormap)
    {
        XFreeColormap(display, colormap);
    }

    if(display)
    {
        XCloseDisplay(display);
        display = NULL;
    }

    if(gpFile)
    {
        fprintf(gpFile,"Program Ended Successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

void uninitialize_sphere(void)
{
    // Code
    //For Per Vertex Light
    if(light_type == 'v')
    {
        if(shaderProgramObject_PV_sphere)
        {
            glUseProgram(shaderProgramObject_PV_sphere);
        
            GLint numShaders = 0;

            glGetProgramiv(shaderProgramObject_PV_sphere, GL_ATTACHED_SHADERS, &numShaders);
            if(numShaders > 0)
            {
                GLuint *pShaders = (GLuint*)malloc(numShaders * sizeof(GLuint));
                if(pShaders != NULL)
                {
                    glGetAttachedShaders(shaderProgramObject_PV_sphere, numShaders, NULL, pShaders);
            
                    for(GLint i = 0; i < numShaders; i++)
                    {
                        glDetachShader(shaderProgramObject_PV_sphere, pShaders[i]);
                        glDeleteShader(pShaders[i] );
                        pShaders[i] = 0;
                    }
        
                    free(pShaders);
                    pShaders = NULL;
                }
            }

            glUseProgram(0);
            glDeleteProgram(shaderProgramObject_PV_sphere);
            shaderProgramObject_PV_sphere = 0;
        }
    }

    if(light_type == 'f')
    {

        // For Per Fragment Light
        if(shaderProgramObject_PF_sphere)
        {
            glUseProgram(shaderProgramObject_PF_sphere);
        
            GLint numShaders = 0;

            glGetProgramiv(shaderProgramObject_PF_sphere, GL_ATTACHED_SHADERS, &numShaders);
            if(numShaders > 0)
            {
                GLuint *pShaders = (GLuint*)malloc(numShaders * sizeof(GLuint));
                if(pShaders != NULL)
                {
                    glGetAttachedShaders(shaderProgramObject_PF_sphere, numShaders, NULL, pShaders);
            
                    for(GLint i = 0; i < numShaders; i++)
                    {
                        glDetachShader(shaderProgramObject_PF_sphere, pShaders[i]);
                        glDeleteShader(pShaders[i] );
                        pShaders[i] = 0;
                    }
        
                    free(pShaders);
                    pShaders = NULL;
                }
            }

            glUseProgram(0);
            glDeleteProgram(shaderProgramObject_PF_sphere);
            shaderProgramObject_PF_sphere = 0;
        }
    }
    
    // Delete vbo of normals
    if(vbo_normal_sphere)
    {
        glDeleteBuffers(1, &vbo_normal_sphere);
        vbo_normal_sphere = 0;
    }

    // Delete vbo of texcoords
    if(vbo_texcoord_sphere)
    {
        glDeleteBuffers(1, &vbo_texcoord_sphere);
        vbo_texcoord_sphere = 0;
    }

    // Delete vbo of elements
    if(vbo_elements_sphere)
    {
        glDeleteBuffers(1, &vbo_elements_sphere);
        vbo_elements_sphere = 0;
    }

    /* DELETE VAO sphere */
    if(vao_sphere)
    {
        glDeleteVertexArrays(1,&vao_sphere);
        vao_sphere = 0;
    }

}


