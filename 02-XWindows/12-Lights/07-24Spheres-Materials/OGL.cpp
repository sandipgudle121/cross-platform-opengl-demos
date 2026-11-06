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
GLuint shaderProgramObject = 0;
enum 
{
    SSG_ATTRIBUTE_POSITION = 0,
    SSG_ATTRIBUTE_COLOR,
    SSG_ATTRIBUTE_NORMAL,
    SSG_ATTRIBUTE_TEXCOORD
};

GLuint vao_sphere = 0;
GLuint vbo_sphere_position = 0;
GLuint vbo_sphere_normal = 0;
GLuint vbo_sphere_element = 0;
GLuint vbo_texcoord = 0;

GLuint numSphereElements = 0;
GLuint NumVertices = 0;
GLuint NumElements = 0;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

GLint w;
GLint h;

GLuint light_Diffused_Uniform;
GLuint light_Ambient_Uniform;
GLuint light_Position_Uniform;
GLuint light_Specular_Uniform;

GLuint material_Diffused_Uniform = 0;
GLuint material_Ambient_Uniform = 0;
GLuint material_Specular_Uniform = 0;
GLuint material_Shininess_Uniform = 0;

GLuint keyPressedUniform = 0;
Bool bLightingEnabled = False;

struct Light
{
    vec4 ambient;
    vec4 diffused;
    vec4 specular;
    vec4 position;
};

struct Light light;

GLfloat materialDiffused_1[] = { 0.7568f, 0.61424f, 0.07568f, 1.0f };   
GLfloat materialAmbient_1[] = { 0.0215f, 0.1745f, 0.0215f, 1.0f };
GLfloat materialSpecular_1[] = { 0.633f, 0.727811f, 0.633f, 1.0f };
GLfloat materialShininess_1 = 0.06f * 128.0f;

GLfloat materialDiffused_2[] = { 0.54f, 0.89f, 0.63f, 1.0f };   
GLfloat materialAmbient_2[] = { 0.135f, 0.2225f, 0.1573f, 1.0f };
GLfloat materialSpecular_2[] = { 0.316228, 0.316228, 0.316228, 1.0f };
GLfloat materialShininess_2 = 0.1f * 128.0f;

GLfloat materialDiffused_3[] = { 0.18275, 0.17, 0.22525, 1.0f };   
GLfloat materialAmbient_3[] = { 0.05375, 0.05, 0.06625, 1.0f };
GLfloat materialSpecular_3[] = { 0.332741, 0.328634, 0.346435, 1.0f };
GLfloat materialShininess_3 = 0.3 * 128.0f;

GLfloat materialDiffused_4[] = { 1.0, 0.829 , 0.829 , 1.0f };   
GLfloat materialAmbient_4[] = { 0.25, 0.20725 ,0.20725 , 1.0f };
GLfloat materialSpecular_4[] = { 0.296648, 0.296648 , 0.296648, 1.0f };
GLfloat materialShininess_4 = 0.088 * 128.0f;

GLfloat materialDiffused_5[] = { 0.61424, 0.04136, 0.04136, 1.0f };   
GLfloat materialAmbient_5[] = { 0.1745, 0.01175, 0.01175, 1.0f };
GLfloat materialSpecular_5[] = { 0.727811, 0.626959 ,0.626959 , 1.0f };
GLfloat materialShininess_5 = 0.6 * 128.0f;

GLfloat materialDiffused_6[] = { 0.396, 0.74151, 0.69102, 1.0f };   
GLfloat materialAmbient_6[] = { 0.1, 0.18725, 0.1745, 1.0f };
GLfloat materialSpecular_6[] = { 0.297554, 0.30829, 0.306668, 1.0f };
GLfloat materialShininess_6 = 0.1 * 128.0;

GLfloat materialDiffused_7[] = { 0.780392, 0.568627, 0.113725, 1.0f };   
GLfloat materialAmbient_7[] = { 0.329412, 0.223529, 0.027451, 1.0f };
GLfloat materialSpecular_7[] = { 0.992157, 0.941176, 0.807843, 1.0f };
GLfloat materialShininess_7 = 0.21794872 * 128.0;

GLfloat materialDiffused_8[] = { 0.714, 0.4284, 0.18144, 1.0f };   
GLfloat materialAmbient_8[] = { 0.2125, 0.1275, 0.054, 1.0f };
GLfloat materialSpecular_8[] = { 0.393548, 0.271906, 0.166721, 1.0f };
GLfloat materialShininess_8 = 0.2 * 128.0;

GLfloat materialDiffused_9[] = { 0.4, 0.4, 0.4, 1.0f };   
GLfloat materialAmbient_9[] = { 0.25, 0.25, 0.25, 1.0f };
GLfloat materialSpecular_9[] = { 0.774597, 0.774597 , 0.774597, 1.0f };
GLfloat materialShininess_9 = 0.6 * 128.0;

GLfloat materialDiffused_10[] = { 0.7038, 0.27048, 0.0828, 1.0f };   
GLfloat materialAmbient_10[] = { 0.19125, 0.0735, 0.0225, 1.0f };
GLfloat materialSpecular_10[] = { 0.256777, 0.137622, 0.086014, 1.0f };
GLfloat materialShininess_10 = 0.1 * 128.0;

GLfloat materialDiffused_11[] = { 0.75164, 0.60648, 0.22648, 1.0f };   
GLfloat materialAmbient_11[] = { 0.24725, 0.1995, 0.0745, 1.0f };
GLfloat materialSpecular_11[] = { 0.608281, 0.555802, 0.366065, 1.0f };
GLfloat materialShininess_11 = 0.4 * 128.0;

GLfloat materialDiffused_12[] = { 0.50754, 0.50754, 0.50754, 1.0f };   
GLfloat materialAmbient_12[] = { 0.19225, 0.19225, 0.19225, 1.0f };
GLfloat materialSpecular_12[] = { 0.508273, 0.508273, 0.508273, 1.0f };
GLfloat materialShininess_12 =  0.4 * 128.0;

GLfloat materialDiffused_13[] = { 0.01, 0.01, 0.01, 1.0f };   
GLfloat materialAmbient_13[] = { 0.0, 0.0, 0.0, 1.0f };
GLfloat materialSpecular_13[] = { 0.50, 0.50, 0.50, 1.0f };
GLfloat materialShininess_13 = 0.25 * 128.0;

GLfloat materialDiffused_14[] = { 0.0, 0.50980392, 0.50980392, 1.0f };   
GLfloat materialAmbient_14[] = { 0.0, 0.1, 0.06, 1.0f };
GLfloat materialSpecular_14[] = { 0.50196078, 0.50196078, 0.50196078, 1.0f };
GLfloat materialShininess_14 = 0.25 * 128.0;

GLfloat materialDiffused_15[] = { 0.1, 0.35, 0.1, 1.0f };   
GLfloat materialAmbient_15[] = { 0.0, 0.0, 0.0, 1.0f };
GLfloat materialSpecular_15[] = { 0.45, 0.45, 0.45, 1.0f };
GLfloat materialShininess_15 = 0.25 * 128.0;

GLfloat materialDiffused_16[] = { 0.5, 0.0, 0.0, 1.0f };   
GLfloat materialAmbient_16[] = { 0.0, 0.0, 0.0, 1.0f };
GLfloat materialSpecular_16[] = { 0.7, 0.6, 0.6, 1.0f };
GLfloat materialShininess_16 = 0.25 * 128.0;

GLfloat materialDiffused_17[] = { 0.55, 0.55, 0.55, 1.0f };   
GLfloat materialAmbient_17[] = { 0.0, 0.0, 0.0, 1.0f };
GLfloat materialSpecular_17[] = { 0.70, 0.70, 0.70, 1.0f };
GLfloat materialShininess_17 = 0.25 * 128.0;

GLfloat materialDiffused_18[] = { 0.5, 0.5, 0.0, 1.0f };   
GLfloat materialAmbient_18[] = { 0.0, 0.0, 0.0, 1.0f };
GLfloat materialSpecular_18[] = { 0.60, 0.60, 0.50, 1.0f };
GLfloat materialShininess_18 = 0.25 * 128.0;

GLfloat materialDiffused_19[] = { 0.01, 0.01, 0.01, 1.0f };   
GLfloat materialAmbient_19[] = { 0.02, 0.02, 0.02, 1.0f };
GLfloat materialSpecular_19[] = { 0.4, 0.4, 0.4, 1.0f };
GLfloat materialShininess_19 = 0.078125 * 128.0;

GLfloat materialDiffused_20[] = { 0.4, 0.5, 0.5, 1.0f };   
GLfloat materialAmbient_20[] = { 0.0, 0.05, 0.05, 1.0f };
GLfloat materialSpecular_20[] = { 0.04, 0.7, 0.7, 1.0f };
GLfloat materialShininess_20 = 0.078125 * 128;

GLfloat materialDiffused_21[] = { 0.4, 0.5, 0.4, 1.0f };   
GLfloat materialAmbient_21[] = { 0.0, 0.05, 0.0, 1.0f };
GLfloat materialSpecular_21[] = { 0.04, 0.7, 0.04, 1.0f };
GLfloat materialShininess_21 = 0.078125 * 128.0;

GLfloat materialDiffused_22[] = { 0.5, 0.4, 0.4, 1.0f };   
GLfloat materialAmbient_22[] = { 0.02, 0.0, 0.0, 1.0f };
GLfloat materialSpecular_22[] = { 0.7, 0.04, 0.04, 1.0f };
GLfloat materialShininess_22 = 0.078125 * 128.0;

GLfloat materialDiffused_23[] = { 0.5, 0.5, 0.5, 1.0f };   
GLfloat materialAmbient_23[] = { 0.05, 0.05, 0.05, 1.0f };
GLfloat materialSpecular_23[] = { 0.7, 0.7, 0.7, 1.0f };
GLfloat materialShininess_23 = 0.078125 * 128.0;

GLfloat materialDiffused_24[] = { 0.5, 0.5, 0.4, 1.0f };   
GLfloat materialAmbient_24[] = { 0.05, 0.05, 0.0, 1.0f };
GLfloat materialSpecular_24[] = { 0.7, 0.7, 0.04, 1.0f };
GLfloat materialShininess_24 = 0.078125 * 128.0;

Bool X_ROTATION = False;
Bool Y_ROTATION = False;
Bool Z_ROTATION = False;

GLfloat light_angle0 = 0.0f;
GLfloat light_angle1 = 0.0f;
GLfloat light_angle2 = 0.0f;

GLfloat SPEED = 2.0f;

GLuint modelMatrixUniform = 0;
GLuint projectionMatrixUniform = 0;
GLuint viewMatrixUniform = 0;
mat4 perspectiveProjectionMatrix;

int main(void)
{
    // Local function declarations
    void uninitialize(void);
    void toggleFullScreen(void);
    void drawDisplay(void);
    int initialize(void);
    void resize(int,int);
    void update(void);

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
        uninitialize();
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

        uninitialize();
        exit(1);
    }
    
    defaultScreen =  XDefaultScreen(display);
    
    // Get available FBConfig from X-Windows
    glxFBConfigs = glXChooseFBConfig(display, XDefaultScreen(display),frameBufferAttributes, &numFBConfig);
    if(glxFBConfigs == NULL)
    {
        fprintf(gpFile,"Matching glXFBConfig cannot be found\n");
        uninitialize();
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
        uninitialize();
        exit(1);
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
    initialize();
    
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
                    w = event.xconfigure.width;
                    h = event.xconfigure.height;
                    resize(event.xconfigure.width, event.xconfigure.height);
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
                            bDone = True;
                        break;

                        default :
                            break;
                    }

                    XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
                    switch(keys[0])
                    {
                        case 'F':
                        case 'f':
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
                
                        case 'X':
                        case 'x':
                            if(X_ROTATION == False)
                            {
                                X_ROTATION = True;
                            }
                            else
                            {
                                Y_ROTATION = False;
                                Z_ROTATION = False;
                            }
                            break;

                        case 'y':
                        case 'Y':
                            if(Y_ROTATION == False)
                            {
                                Y_ROTATION = True;
                            }
                            else
                            {
                                X_ROTATION = False;
                                Z_ROTATION = False;
                            }
                            break;

                        case 'z' :
                        case 'Z':
                            if(Z_ROTATION == False)
                            {
                                Z_ROTATION = True;
                            }
                            else
                            {
                                //Z_ROTATION = False;
                                X_ROTATION = False;
                                Y_ROTATION = False;
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
            drawDisplay();

            update();
        }

    }
    
    uninitialize();

    return(0);

}

int initialize(void)
{
    // Local function declarations
    void resize(int,int);
    void uninitialize(void);
    void printGLInfo(void);

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
        uninitialize();
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
            uninitialize();
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
        uninitialize();
        exit(1);
    }

    // Initialize GLEW 
    if(glewInit()!= GLEW_OK)
    {
        fprintf(gpFile,"Failed to initialize GLEW\n");
        uninitialize();
        exit(-1);
    }

    // PrintGLInfo 
    printGLInfo();

    /* Vertex Shaders */
    const GLchar* vertexShaderSourceCode =  "#version 460 core\n" \
                                            "\n" \
                                            "in vec4 aPosition;\n" \
                                            "in vec3 aNormal;\n" \
                                            "\n" \
                                            "uniform mat4 uViewMatrix;\n" \
                                            "uniform mat4 uModelMatrix;\n" \
                                            "uniform mat4 uProjectionMatrix;\n" \
                                            "uniform vec4 uLight_Position;\n" \
                                            "\n" \
                                            "uniform int uKeyPressed;\n" \
                                            "out vec3 oTransformed_Normals;\n" \
                                            "out vec3 oLight_Direction;\n" \
                                            "out vec3 oViewer_Vector;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "if(uKeyPressed == 1)\n" \
                                            "{\n" \
                                            "vec4 eye_Coordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
                                            "oTransformed_Normals = mat3(uViewMatrix * uModelMatrix) * aNormal;\n" \
                                            "oLight_Direction = vec3(uLight_Position - eye_Coordinates);\n" \
                                            "oViewer_Vector = -eye_Coordinates.xyz;\n" \
                                            "}\n" \
                                            "else\n" \
                                            "{\n" \
                                            "oTransformed_Normals = vec3(0.0, 0.0, 0.0);\n" \
                                            "oLight_Direction = vec3(0.0, 0.0, 0.0);\n" \
                                            "oViewer_Vector = vec3(0.0, 0.0, 0.0);\n" \
                                            "}\n" \
                                            "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
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
                fprintf(gpFile,"Vertex shader complilation(per fragment) : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize();
    }

    /* Fragment Shader */
    const GLchar* fragmentShaderSourceCode = "#version 460 core\n" \
                                            "\n" \
                                            "in vec3 oTransformed_Normals;\n" \
                                            "in vec3 oLight_Direction;\n" \
                                            "in vec3 oViewer_Vector;\n" \
                                            "\n" \
                                            "uniform vec3 uLight_Ambient;\n" \
                                            "uniform vec3 uLight_Specular;\n" \
                                            "uniform vec3 uLight_Diffused;\n" \
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
                                            "vec3 normalized_Light_Direction;\n" \
                                            "vec3 lightDiffused;\n" \
                                            "vec3 lightAmbient;\n" \
                                            "vec3 reflection_Vector;\n" \
                                            "vec3 lightSpecular;\n" \
                                            "vec3 normalized_Transformed_Normals = normalize(oTransformed_Normals);\n" \
                                            "normalized_Light_Direction = normalize(oLight_Direction);\n" \
                                            "vec3 normalized_Viewer_Vector = normalize(oViewer_Vector);\n" \
                                            "\n" \
                                            "lightAmbient = uLight_Ambient * uMaterial_Ambient;\n" \
                                            "lightDiffused = uLight_Diffused * uMaterial_Diffused * max(dot(normalized_Light_Direction, normalized_Transformed_Normals), 0.0);\n" \
                                            "reflection_Vector = reflect(-normalized_Light_Direction, normalized_Transformed_Normals);\n" \
                                            "lightSpecular = uLight_Specular * uMaterial_Specular * pow(max(dot(reflection_Vector, normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n" \
                                            "phong_ADS_Light = lightAmbient + lightDiffused + lightSpecular;\n" \
                                            "}\n" \
                                            "else\n" \
                                            "{\n" \
                                            "phong_ADS_Light = vec3(1.0, 1.0, 1.0);\n" \
                                            "}\n" \
                                            "FragColor = vec4(phong_ADS_Light, 1.0);\n" \
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
                fprintf(gpFile,"fragment shader compilation(per fragment) : %s \n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize();
    }

    /* Shader Program for Per Fragment */
    shaderProgramObject = glCreateProgram();
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_COLOR, "aColor");
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
                fprintf(gpFile,"shader program(fragment) linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialize();
    }

    /* Get Shader Uniform Locations for Per Fragment */
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
    glGenBuffers(1, &vbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normal), sphere_normal, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO for texture
    glGenBuffers(1, &vbo_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_texcoords), sphere_texcoords, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE,0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_TEXCOORD);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // VBO for elements
    glGenBuffers(1, &vbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // VAO unbind
    glBindVertexArray(0);

    // Enabling Depth
    glClearDepth(1.0f);         // Compulsory
    glEnable(GL_DEPTH_TEST);    // Compulsory
    glDepthFunc(GL_LEQUAL);     // Compulsaory

    light.ambient = vec4( 0.0f, 0.0f, 0.0f, 1.0 );
    light.diffused = vec4( 1.0f, 1.0f, 1.0f, 1.0f );
    light.specular = vec4( 1.0f, 1.0f, 1.0f, 1.0f );
    light.position = vec4( 0.0f, 0.0f, 0.0f, 1.0f );

    // Set the clear of window to blue
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);       // here OpenGL starts

    /* Initialize Orthographicprojectionmatrix */
    perspectiveProjectionMatrix = vmath::mat4::identity();

    resize(WIN_WIDTH,WIN_HEIGHT);

    return(0);

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

void drawDisplay(void)
{
    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Sphere 1
    glUseProgram(shaderProgramObject);
    glViewport(0, 3.2 * h/4, w/6, h/6);

    // Sphere
    mat4 translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    mat4 modelMatrix = translationMatrix;
    mat4 viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_1);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_1);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_1);
        glUniform1f(material_Shininess_Uniform, materialShininess_1);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 2
    glUseProgram(shaderProgramObject);
    glViewport(w/6, 3.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_2);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_2);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_2);
        glUniform1f(material_Shininess_Uniform, materialShininess_2);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 3
    glUseProgram(shaderProgramObject);
    glViewport(2 * w/6, 3.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_3);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_3);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_3);
        glUniform1f(material_Shininess_Uniform, materialShininess_3);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 4
    glUseProgram(shaderProgramObject);
    glViewport(3 * w/6, 3.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_4);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_4);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_4);
        glUniform1f(material_Shininess_Uniform, materialShininess_4);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
//////////////////////////////////////////////////////////////////////////////////////////////

    // Sphere 5
    glUseProgram(shaderProgramObject);
    glViewport(4 * w/6, 3.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_5);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_5);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_5);
        glUniform1f(material_Shininess_Uniform, materialShininess_5);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 6
    glUseProgram(shaderProgramObject);
    glViewport(5 * w/6, 3.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_6);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_6);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_6);
        glUniform1f(material_Shininess_Uniform, materialShininess_6);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 7
    glUseProgram(shaderProgramObject);
    glViewport(0, 2.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_7);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_7);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_7);
        glUniform1f(material_Shininess_Uniform, materialShininess_7);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 8
    glUseProgram(shaderProgramObject);
    glViewport(w/6, 2.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_8);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_8);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_8);
        glUniform1f(material_Shininess_Uniform, materialShininess_8);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
////////////////////////////////////////////////////////////////////////////////////////

    // Sphere 9
    glUseProgram(shaderProgramObject);
    glViewport(2 * w/6, 2.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_9);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_9);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_9);
        glUniform1f(material_Shininess_Uniform, materialShininess_9);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 10
    glUseProgram(shaderProgramObject);
    glViewport(3 * w/6, 2.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_10);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_10);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_10);
        glUniform1f(material_Shininess_Uniform, materialShininess_10);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 11
    glUseProgram(shaderProgramObject);
    glViewport(4 * w/6, 2.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_11);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_11);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_11);
        glUniform1f(material_Shininess_Uniform, materialShininess_11);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 12
    glUseProgram(shaderProgramObject);
    glViewport(5 * w/6, 2.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_12);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_12);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_12);
        glUniform1f(material_Shininess_Uniform, materialShininess_12);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
//////////////////////////////////////////////////////////////////////////////////

    // Sphere 13
    glUseProgram(shaderProgramObject);
    glViewport(0, 1.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_13);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_13);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_13);
        glUniform1f(material_Shininess_Uniform, materialShininess_13);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 14
    glUseProgram(shaderProgramObject);
    glViewport(w/6, 1.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_14);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_14);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_14);
        glUniform1f(material_Shininess_Uniform, materialShininess_14);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 15
    glUseProgram(shaderProgramObject);
    glViewport(2 * w/6, 1.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_15);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_15);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_15);
        glUniform1f(material_Shininess_Uniform, materialShininess_15);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 16
    glUseProgram(shaderProgramObject);
    glViewport(3 * w/6, 1.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_16);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_16);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_16);
        glUniform1f(material_Shininess_Uniform, materialShininess_16);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
//////////////////////////////////////////////////////////////////////////////////

    // Sphere 17
    glUseProgram(shaderProgramObject);
    glViewport(4 * w/6, 1.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_17);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_17);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_17);
        glUniform1f(material_Shininess_Uniform, materialShininess_17);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 18
    glUseProgram(shaderProgramObject);
    glViewport(5 * w/6, 1.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_18);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_18);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_18);
        glUniform1f(material_Shininess_Uniform, materialShininess_18);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 19
    glUseProgram(shaderProgramObject);
    glViewport(0, 0.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_19);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_19);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_19);
        glUniform1f(material_Shininess_Uniform, materialShininess_19);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 20
    glUseProgram(shaderProgramObject);
    glViewport(w/6, 0.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_20);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_20);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_20);
        glUniform1f(material_Shininess_Uniform, materialShininess_20);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
///////////////////////////////////////////////////////////////////////////////////////

    // Sphere 21
    glUseProgram(shaderProgramObject);
    glViewport(2 * w/6, 0.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_21);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_21);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_21);
        glUniform1f(material_Shininess_Uniform, materialShininess_21);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 22
    glUseProgram(shaderProgramObject);
    glViewport(3 * w/6, 0.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_22);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_22);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_22);
        glUniform1f(material_Shininess_Uniform, materialShininess_22);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 23
    glUseProgram(shaderProgramObject);
    glViewport(4 * w/6, 0.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_23);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_23);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_23);
        glUniform1f(material_Shininess_Uniform, materialShininess_23);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Sphere 24
    glUseProgram(shaderProgramObject);
    glViewport(5 * w/6, 0.2 * h/4, w/6, h/6);

    // Sphere
    translationMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    modelMatrix = translationMatrix;
    viewMatrix = mat4::identity();

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == True)
    {
        glUniform1i(keyPressedUniform, 1);
            
        glUniform3fv(light_Ambient_Uniform, 1, light.ambient);
        glUniform3fv(light_Diffused_Uniform, 1, light.diffused);
        glUniform3fv(light_Specular_Uniform, 1, light.specular);
        glUniform4fv(light_Position_Uniform, 1, light.position);

        glUniform3fv(material_Diffused_Uniform, 1, materialDiffused_24);
        glUniform3fv(material_Ambient_Uniform, 1, materialAmbient_24);
        glUniform3fv(material_Specular_Uniform, 1, materialSpecular_24);
        glUniform1f(material_Shininess_Uniform, materialShininess_24);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vao_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glXSwapBuffers(display, window);
}

void resize(int width, int height)
{
    // Code
    if(height <= 0)
        height = 1;
    

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    perspectiveProjectionMatrix = vmath::perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
}

void update(void)
{
    // Code
    if(bLightingEnabled == True )
    {   
        if(X_ROTATION == True)
        {
            light.position[0] = 0.0f; 
            light.position[1] = 45.0f * (GLfloat)cos(light_angle0 * M_PI / 180.0f); 
            light.position[2] = 45.0f * (GLfloat)sin(light_angle0 * M_PI / 180.0f); 
            light.position[3] = 1.0f;
 
            light_angle0 = light_angle0 + SPEED;
            if(light_angle0 > 360.0f)
                light_angle0 = light_angle0 - 360.0f;
        }
        
        if(Y_ROTATION == True)
        {
            light.position[0] = 45.0f * (GLfloat)cos(light_angle1 * M_PI / 180.0f);
            light.position[1] = 0.0f; 
            light.position[2] = 45.0f * (GLfloat)sin(light_angle1 * M_PI / 180.0f);
            light.position[3] = 1.0f;

            light_angle1 = light_angle1 + SPEED;
            if(light_angle1 > 360.0f)
                light_angle1 = light_angle1 - 360.0f;
        }
        
       if(Z_ROTATION == True)
       {
            light.position[0] = 45.0f * (GLfloat)sin(light_angle2 * M_PI / 180.0f);
            light.position[1] = 45.0f * (GLfloat)cos(light_angle2 * M_PI / 180.0f);
            light.position[2] = 0.0f;
            light.position[3] = 1.0f;

            light_angle2 = light_angle2 + SPEED ;
            if(light_angle2 > 360.0f)
                light_angle2 = light_angle2 - 360.0f;
       }
        
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

void uninitialize(void)
{
    // Local variable declarations
    GLXContext currentGlxContext = NULL;

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

    // Delete vbo element 
    if(vbo_sphere_element)
    {
        glDeleteBuffers(1, &vbo_sphere_element);
        vbo_sphere_element = 0;
    }

    // Delete vbo normal
    if(vbo_sphere_normal)
    {
        glDeleteBuffers(1, &vbo_sphere_normal);
        vbo_sphere_normal = 0;
    }

    /* Delete VBO of Position */
    if(vbo_sphere_position)
    {
        glDeleteBuffers(1, &vbo_sphere_position);
        vbo_sphere_position = 0;
    }

    /* Delete VAO */
    if(vao_sphere)
    {
        glDeleteVertexArrays(1,&vao_sphere);
        vao_sphere = 0;

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



