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
    SSG_ATTRIBUTE_COLOR
};

GLuint vao_H_Lines = 0;
GLuint vao_V_Lines = 0;
GLuint vao_green_line = 0;
GLuint vao_red_line = 0;
GLuint vao_triangle = 0;
GLuint vao_square = 0;
GLuint vao_circle = 0;

GLuint vbo_position_H_Lines = 0;
GLuint vbo_position_V_Lines = 0;
GLuint vbo_position_green_line = 0;
GLuint vbo_position_red_line = 0;
GLuint vbo_position_triangle = 0;
GLuint vbo_position_square = 0;
GLuint vbo_position_circle = 0;

GLuint vbo_color = 0;
GLuint vbo_color_green_line = 0;
GLuint vbo_color_red_line = 0;
GLuint vbo_color_hollow_triangle = 0;
GLuint vbo_color_hollow_square = 0;
GLuint vbo_color_hollow_circle = 0;

const int circle_points = 360;

Bool  TRIANGLE = False;
Bool  CIRCLE = False;
Bool  SQUARE = False;
Bool  GRAPH = False;

GLuint mvpMatrixUniform = 0;
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

                        case 'T':
                        case 't':
                            if(TRIANGLE == True)
                            {
                                TRIANGLE = False;
                            }    
                            else if(TRIANGLE == False)
                            {
                                TRIANGLE = True;
                            }
                            break;

                        case 'S':
                    case 's':
                        if(SQUARE == True)
                        {
                            SQUARE = False;
                        }
                        else if(SQUARE == False)
                        {
                            SQUARE = True;
                        }
                        break;

                    case 'C':
                    case 'c':
                        if(CIRCLE == True)
                        {
                            CIRCLE = False;
                        }
                        else if(CIRCLE == False)
                        {
                            CIRCLE = True;
                        } 
                        break;

                    case 'G' :
                    case 'g' :
                        if(GRAPH == True)
                        {
                            GRAPH = False;
                        }                
                        else if(GRAPH == False)
                        {
                            GRAPH = True;
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

    // Vertex Shaders
    const GLchar* vertexShaderSourceCode =  "#version 460 core\n" \
                                            "\n" \
                                            "in vec4 aPosition;\n" \
                                            "in vec4 aColor;\n" \
                                            "out vec4 oColor;\n" \
                                            "uniform mat4 uMVPMatrix;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "gl_Position=uMVPMatrix * aPosition;\n" \
                                            "oColor=aColor;\n" \
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
        uninitialize();
    }

    /* Fragment Shader */
    const GLchar* fragmentShaderSourceCode = "#version 460 core\n" \
                                            "\n" \
                                            "in vec4 oColor;\n" \
                                            "out vec4 FragColor;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "FragColor=oColor;\n" \
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
        uninitialize();
    }

    /* Shader Program */
    shaderProgramObject = glCreateProgram();
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_POSITION, "aPosition");
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
        uninitialize();
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
    
    const GLfloat trianle[] = {
                                    0.0f, 1.0f, 0.0f,
                                    -1.0f, -1.0f, 0.0f,
                                    1.0f, -1.0f, 0.0f
                              };
    
    const GLfloat square[] = {
                                1.0f,1.0f,0.0f,
                                -1.0f,1.0f,0.0f,
                                -1.0f,-1.0f,0.0f,
                                1.0f,-1.0f,0.0f
                             };

    const GLfloat hollow_circle[] = {
                                        1.0f,1.0f,0.0f,
                                        -1.0f,1.0f,0.0f,
                                        -1.0f,-1.0f,0.0f,
                                        1.0f,-1.0f,0.0f
                             };

    const GLfloat hollow_triangle_color[] = {
                                                1.0f,1.0f,0.0f,
                                                1.0f,1.0f,0.0f,
                                                1.0f,1.0f,0.0f
                                            }; 
    
    const GLfloat hollow_square_color[] = {
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
    
    // Enabling Depth
    glClearDepth(1.0f);         // Compulsory
    glEnable(GL_DEPTH_TEST);    // Compulsory
    glDepthFunc(GL_LEQUAL);     // Compulsaory

    // Set the clear of window to blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // here OpenGL starts

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
    //  Function declarations
    void Triangle(void);
    void Square(void);

    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(GRAPH == True)
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
    if(SQUARE == True)
    {
        Square();
    }

    // Call hollow triangle function
    if(TRIANGLE == True)
    {
        Triangle();
    }

    // Hollow circle
    if(CIRCLE == True)
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

    // Delete VBO of Position for triangle
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

void Square(void)
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

void Triangle(void)
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

