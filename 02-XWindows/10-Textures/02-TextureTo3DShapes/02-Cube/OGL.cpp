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

#include<SOIL/SOIL.h>   // texture header

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

/* OpenGL related variables */
GLuint shaderProgramObject = 0;
enum 
{
    SSG_ATTRIBUTE_POSITION = 0,
    SSG_ATTRIBUTE_COLOR,
    SSG_ATTRIBUTE_TEXCOORD
};

GLuint vao_cube = 0;
GLuint vbo_position_cube = 0;
GLuint vbo_color_cube = 0;
GLuint vbo_texcoord_cube = 0;
GLuint texture_kundali = 0;

GLfloat cube_angle = 0.0f;

GLuint mvpMatrixUniform = 0;
GLuint textureSamplerUniform = 0;
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
    GLuint loadglTexture(const char*);

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
        uninitialize();
    }

    /* Fragment Shader */
    const GLchar* fragmentShaderSourceCode ="#version 460 core\n" \
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
        uninitialize();
    }

    /* Shader Program */
    shaderProgramObject = glCreateProgram();
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_POSITION, "aPosition");
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
        uninitialize();
    }

    /* Get Shader Uniform Locations */
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");
    
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

    const GLfloat cubeTexcoords[] =
                                        {
	                                        // front
                                            1.0f, 0.0f, // bottom-right of front
                                            0.0f, 0.0f, // bottom-left of front
	                                        0.0f, 1.0f, // top-left of front
                                            1.0f, 1.0f, // top-right of front
	                                        
                                        
	                                        // right
	                                        1.0f, 0.0f, // bottom-right of front
                                            0.0f, 0.0f, // bottom-left of front
	                                        0.0f, 1.0f, // top-left of front
                                            1.0f, 1.0f, // top-right of front

	                                        // back
	                                        1.0f, 0.0f, // bottom-right of front
                                            0.0f, 0.0f, // bottom-left of front
	                                        0.0f, 1.0f, // top-left of front
                                            1.0f, 1.0f, // top-right of front

	                                        // left
	                                        1.0f, 0.0f, // bottom-right of front
                                            0.0f, 0.0f, // bottom-left of front
	                                        0.0f, 1.0f, // top-left of front
                                            1.0f, 1.0f, // top-right of front

	                                        // top
	                                        1.0f, 0.0f, // bottom-right of front
                                            0.0f, 0.0f, // bottom-left of front
	                                        0.0f, 1.0f, // top-left of front
                                            1.0f, 1.0f, // top-right of front

	                                        // bottom
	                                        1.0f, 0.0f, // bottom-right of front
                                            0.0f, 0.0f, // bottom-left of front
	                                        0.0f, 1.0f, // top-left of front
                                            1.0f, 1.0f, // top-right of front
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
    
    // Enabling Depth
    glClearDepth(1.0f);         // Compulsory
    glEnable(GL_DEPTH_TEST);    // Compulsory
    glDepthFunc(GL_LEQUAL);     // Compulsaory

    // Set the clear of window to blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // here OpenGL starts

    // Create stone texture
    texture_kundali = loadglTexture("Kundali.bmp");
    if(texture_kundali == 0)
    {
        printf("texture_kundali allocation failed\n");
        exit(-1);
    }

    /* Initialize Orthographicprojectionmatrix */
    perspectiveProjectionMatrix = vmath::mat4::identity();

    // Tell OpenGL to enable texture
    glEnable(GL_TEXTURE_2D);

    resize(WIN_WIDTH,WIN_HEIGHT);

    return(0);

}

GLuint loadglTexture(const char* path)
{
    // local variable declarations
    int width,height;
    unsigned char *imageData = NULL;
    GLuint texture;

    // code
    imageData = SOIL_load_image(path, &width, &height, NULL, SOIL_LOAD_RGB);
    if(imageData == NULL)
    {
        //printf("SOIL_load_image() failed for file %s: %s\n", path, SOIL_last_result());
        printf("SOIL_load_image() failed \n");
        exit(-1);
    }

    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0 ,GL_RGB, GL_UNSIGNED_BYTE, (void*)imageData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    SOIL_free_image_data(imageData);

    return(texture);

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
    
    glUseProgram(shaderProgramObject);

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
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_kundali);
    glUniform1i(textureSamplerUniform, 0);

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
    cube_angle = cube_angle - 1.0f;
    if(cube_angle <= 0.0f)
        cube_angle = cube_angle + 360.0f;
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

    // Delete kundali texture texcords
    if(vbo_texcoord_cube)
    {
        glDeleteBuffers(1, &vbo_texcoord_cube);
        vbo_texcoord_cube = 0;
    }

    /* Cube */
    /* DELETE VBO of Color */
    if(vbo_color_cube)
    {
        glDeleteBuffers(1, &vbo_color_cube);
        vbo_color_cube = 0;
    }

    /* Delete VBO of Position */
    if(vbo_position_cube)
    {
        glDeleteBuffers(1, &vbo_position_cube);
        vbo_position_cube = 0;
    }

    /* Delete VAO_square */
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

    // Delete kundali texture
    if(texture_kundali)
    {
        glDeleteTextures(1,&texture_kundali);
        texture_kundali = 0;
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



