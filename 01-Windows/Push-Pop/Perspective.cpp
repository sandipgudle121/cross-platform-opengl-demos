///Windows HeaderFiles
#include<Windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<gl/glew.h>     /* this must be before <gl/gl.h> */
#include<gl/GL.h>

#include "Perspective.h"
#include "vmath.h"
#include "Sphere.h"

using namespace vmath;

// MACROS
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//Link With OpenGL Library
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"Sphere.lib")

//Global Function Declarations
LRESULT CALLBACK WndProc(HWND ,UINT , WPARAM, LPARAM);

// Global Variable Declaration
FILE *gpfile = NULL;    // For File IO

// Global Variable Declaration for OpenGL 
HDC ghdc = NULL;
HGLRC ghrc = NULL;      // Create OpenGL context from device context 

// Global Variable For Fullscreen of Window
HWND ghwnd = NULL;      
DWORD dwStyle = 0;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT)};
BOOL gbFullScreen = FALSE;
int iResult = 0;
BOOL gbActive = FALSE;

/* OpenGL related variables */
GLuint shaderProgramObject = 0;
enum 
{
    SSG_ATTRIBUTE_VERTEX,
    SSG_ATTRIBUTE_ELEMENT,
    SSG_ATTRIBUTE_NORMAL,
    SSG_ATTRIBUTE_COLOR
};

GLuint vao_sphere = 0;
GLuint vbo_sphere_position = 0;
GLuint vbo_sphere_normal = 0;
GLuint vbo_sphere_element = 0;

GLuint NumVertices = 0;
GLuint NumElements = 0;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

mat4 modelMatrix;
mat4 viewMatrix;

// Matrix stack structure
typedef struct ModelView
{
    mat4 ViewMatrix;
    mat4 ModelMatrix;
} MODELVIEWMATRIX;

#define MODEL_VIEW_MATRIX_STACK_DEPTH 32

// Initilizing model matrix stack
MODELVIEWMATRIX modelViewMatrix[MODEL_VIEW_MATRIX_STACK_DEPTH];

// Initialing the top of the matrix stack
int stackTop = -1;
int day = 0;
int year = 0;
int hour = 0;


GLuint mvpMatrixUniform = 0;
mat4 perspectiveProjectionMatrix;

// Entry-Point Function
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int iCmdShow)
{
    // Function Declarations
    int initialize(void);
    void uninitialize(void);
    void display(void);
    void update(void);

 //Local Variable Declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[]= TEXT("SG_Window"); 

    // For Avtive Window
    BOOL bDone = FALSE;

    // Code 

    
    if(fopen_s(&gpfile,"Log.txt","w") != 0)
    {
        MessageBox(NULL,TEXT("log file cannot be opened"),
                    TEXT("Error"),MB_OK | MB_ICONERROR);
        exit(0);          
    }
    fprintf(gpfile,"Program Started Succesfully\n");

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(LEAF_ICON));
    wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(LEAF_ICON));

    RegisterClassEx(&wndclass);

    // Create Window
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);

    int xw =  w/2 - 400;
    int yh =  h/2 - 300;
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,szAppName,
                        TEXT("Sandip Sham Gudle"),
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,   
                        xw,
                        yh,
                        WIN_WIDTH,
                        WIN_HEIGHT,                  
                        NULL,
                        NULL,
                        hInstance,
                        NULL );

    ghwnd  = hwnd;

    // Initialization
    iResult = initialize();

    if(iResult != 0)
    {
        MessageBox(hwnd,TEXT("initialization failed"),
                    TEXT("Error"),MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);

    }

    // Show Window
    ShowWindow(hwnd, iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    
    // Message Loop
    // Game Loop
    while(bDone == FALSE)
    {
        if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                bDone = TRUE;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

        }
        else
        {
            if(gbActive== TRUE)
            {
                //Render
                display();

                //Update
                update();
            }
        }
    }

    // uninitialization
    uninitialize();

    return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT iMsg,WPARAM wParam, LPARAM lParam )
{

    // Function Declaration
    void ToggleFullscreen(void);
    void resize(int,int);
    // Code 
    switch(iMsg)
    {
        case WM_SETFOCUS :
            gbActive = TRUE;
        break;

        case WM_KILLFOCUS :
            gbActive = FALSE;
        break;

        case WM_SIZE :
            resize(LOWORD(lParam),HIWORD(lParam));
        break;

        case WM_ERASEBKGND :
            return(0);

        case WM_KEYDOWN :
            switch(LOWORD(wParam))
            {
                case VK_ESCAPE :
                    DestroyWindow(hwnd);
                    break;
            }
        break;

        case WM_CHAR :
            switch(LOWORD(wParam))
            {
                case 'F':
                case 'f':
                    if(gbFullScreen ==FALSE)
                    {
                        ToggleFullscreen();
                        gbFullScreen= TRUE;
                    }
                    else
                    {
                        ToggleFullscreen();
                        gbFullScreen = FALSE;
                    }
                break;

                case 'D' :
                    day = (day + 6) % 360;
                    hour = (hour + 3) % 360;
                    break;

                case 'd' :
                    day = (day - 6) % 360;
                    hour = (hour - 3) % 360;
                    break;

                case 'Y' :
                    year = (year + 3 ) % 360;
                    //hour = (hour - 3) % 360;
                    day = (day + 6) % 360;
                    break;

                case 'y' :
                    year = (year - 3) % 360;
                    //hour = (hour + 3) % 360;
                    day = (day - 6) % 360;
                    break;

                case 'm' :
                    hour = (hour + 3) % 360;
                    break;
                
                case 'M' :
                    hour = (hour - 3) % 360;
                    break;
            }
        break;

        case WM_DESTROY:

            if(gpfile)
            {
                fprintf(gpfile,"Program Ended Successfully\n");
                fclose(gpfile);
                gpfile = NULL;
            }
            PostQuitMessage(0);
            break;

        default :
            break;
    }
    
    return(DefWindowProc(hwnd,iMsg,wParam,lParam));

}

void ToggleFullscreen(void)
{
    // Local Variable Declarations
    MONITORINFO mi = { sizeof(MONITORINFO)};

    // Code
    if(gbFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd,GWL_STYLE);
        if(dwStyle & WS_OVERLAPPEDWINDOW)
        {
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd,MONITORINFOF_PRIMARY),&mi))
            {
                SetWindowLong(ghwnd,GWL_STYLE,dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right - mi.rcMonitor.left,mi.rcMonitor.bottom - mi.rcMonitor.top,SWP_NOZORDER | SWP_FRAMECHANGED);

            }
        }
        ShowCursor(FALSE);
    }

    else
    {
        SetWindowPlacement(ghwnd,&wpPrev);
        SetWindowLong(ghwnd,GWL_STYLE ,
        dwStyle | WS_OVERLAPPEDWINDOW );
        SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE  | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }
}

int initialize(void)
{
    // Variables
    // Function Declarations 
    void resize(int,int);
    void printGLInfo(void);
    void uninitialize(void);

    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;
    ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));

    // get dc
    ghdc = GetDC(ghwnd);
    if(ghdc == NULL)
    {
        fprintf(gpfile,"GetDC failed\n");
        return(-1);
    }

    // Code
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER);
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    iPixelFormatIndex = ChoosePixelFormat(ghdc,&pfd);

    if(iPixelFormatIndex == 0)
    {
        fprintf(gpfile,"ChoosePixelFormat fuction failed\n");
        return(-2);
    }

    if(SetPixelFormat(ghdc,iPixelFormatIndex,&pfd) == FALSE)
    {
        fprintf(gpfile,"SetPixelFormat function failed\n");
        return(-3);
    }

    // tell wgl  library to give opengl compatible context from this device context

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
        fprintf(gpfile,"wglContext failed\n");
        return(-4);
    }

    // make rendering context current 
    if(wglMakeCurrent(ghdc,ghrc) == FALSE)
    {
        fprintf(gpfile,"wgl MakeCurrent failed\n");
        return(-5);
    }

    /* initialize GLEW */
    if(glewInit()!= GLEW_OK)
    {
        fprintf(gpfile,"failed to initialize GLEW \n");
        return(-6);
    }

    /* PrintGLInfo */
    printGLInfo();

    /* Vertex Shaders */
    const GLchar* vertexShaderSourceCode =  

    "#version 460 core\n" \
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

    GLint status = NULL;
    GLint infoLoglength = NULL;
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
                fprintf_s(gpfile,"Vertex shader complilation : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize();
    }

    /* Fragment Shader */
    const GLchar* fragmentShaderSourceCode = 

    "#version 460 core\n" \
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
                fprintf_s(gpfile,"fragment shader compilation : %s \n", szInfoLog);
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
    glBindAttribLocation(shaderProgramObject, SSG_ATTRIBUTE_VERTEX, "aPosition");
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
                fprintf_s(gpfile,"shader program linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialize();
    }

    /* Get Shader Uniform Locations */
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");

    getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
    NumVertices = getNumberOfSphereVertices();
    NumElements = getNumberOfSphereElements();

    // vao 
    glGenVertexArrays(1,&vao_sphere);
    glBindVertexArray(vao_sphere);

    // vbo position
    glGenBuffers(1, &vbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_VERTEX);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // vbo normal
    glGenBuffers(1, &vbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // vbo element
    glGenBuffers(1, &vbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // vao unbind
    glBindVertexArray(0);

    // Enabling Depth
    glClearDepth(1.0f);         // Compulsory
    glEnable(GL_DEPTH_TEST);    // Compulsory
    glDepthFunc(GL_LEQUAL);     // Compulsaory
    glShadeModel(GL_SMOOTH);    // Optional
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);   // Optional or for beautification

    // Set the clear of window to blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // here OpenGL starts

    // Initialize Orthographicprojectionmatrix */
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
    fprintf(gpfile,"OpenGL Vender : %s\n",glGetString(GL_VENDOR));
    fprintf(gpfile,"OpenGL Renderer : %s\n",glGetString(GL_RENDERER));
    fprintf(gpfile,"OpenGL Version : %s \n",glGetString(GL_VERSION));
    fprintf(gpfile,"OpenGL Shading language version(GLSL) : %s \n",glGetString(GL_SHADING_LANGUAGE_VERSION));


    /* listing of supported extensions */
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(i = 0; i < numExtensions; i++)
    {
        fprintf(gpfile,"%s\n",glGetStringi(GL_EXTENSIONS, i));
    
    }

}

void resize(int width, int height)
{
    // Code
    if(height <= 0)
        height = 1;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    /* Set Perspective Projection Matrix */
    perspectiveProjectionMatrix = vmath::perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
}

void display(void)
{
    // Function Declarations
    void pushMatrix(void);
    void popMatrix(void);

    // Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);

    vmath::mat4 translationMatrix;
	vmath::mat4 scaleMatrix;
	vmath::mat4 rotationMatrix;

    //--------------------- SUN ----------------------------
    modelMatrix = vmath::mat4::identity();
    viewMatrix = vmath::mat4::identity();

    // Sun's Transformations
    translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
    scaleMatrix = vmath::scale(0.60f, 0.60f, 0.60f);

    modelMatrix = modelMatrix * translationMatrix * scaleMatrix;
    pushMatrix(); // Save Sun's transformation to stack

    // Render Sun
    vmath::mat4 sunMVPMatrix = perspectiveProjectionMatrix * modelMatrix;
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, sunMVPMatrix);
    glBindVertexArray(vao_sphere);
    glVertexAttrib3f(SSG_ATTRIBUTE_COLOR, 1.0f, 0.5f, 0.0f); // Sun's color
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, NumElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    //--------------------- EARTH ----------------------------
    popMatrix(); // Restore Sun's transformation as base
    pushMatrix();

    // Earth's Transformations
    rotationMatrix = vmath::rotate((float)year, 0.0f, 1.0f, 0.0f); // Earth's orbit around Sun
    translationMatrix = vmath::translate(1.2f, 0.0f, 0.0f);        // Position relative to Sun
    scaleMatrix = vmath::scale(0.50f, 0.50f, 0.50f);

    modelMatrix = modelMatrix * rotationMatrix * translationMatrix * scaleMatrix;
    pushMatrix(); // Save Earth's transformation for self-rotation

    // Earth's Self-Rotation
    rotationMatrix = vmath::rotate((float)day, 0.0f, 1.0f, 0.0f);
    modelMatrix = modelMatrix * rotationMatrix;

    // Render Earth
    vmath::mat4 earthMVPMatrix = perspectiveProjectionMatrix * modelMatrix;
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, earthMVPMatrix);
    glBindVertexArray(vao_sphere);
    glVertexAttrib3f(SSG_ATTRIBUTE_COLOR, 0.4f, 0.9f, 1.0f); // Earth's color
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, NumElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    //--------------------- MOON ----------------------------
    popMatrix(); // Restore to Earth's orbit state
    pushMatrix();

    // Moon's Transformations
    rotationMatrix = vmath::rotate((float)hour, 0.0f, 1.0f, 0.0f); // Moon's orbit around Earth
    translationMatrix = vmath::translate(1.0f, 0.0f, 0.0f); 
    scaleMatrix = vmath::scale(0.25f, 0.25f, 0.25f); 

    modelMatrix = modelMatrix * rotationMatrix * translationMatrix * scaleMatrix;

    // Compute MVP and Render Moon
    vmath::mat4 moonMVPMatrix = perspectiveProjectionMatrix * modelMatrix;
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, moonMVPMatrix);
    glBindVertexArray(vao_sphere);
    glVertexAttrib3f(SSG_ATTRIBUTE_COLOR, 0.8f, 0.8f, 0.8f); // Moon's color
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, NumElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    // End Moon's context
    popMatrix(); // Restore to Earth's orbit state
    popMatrix(); // Restore to Sun's base state
    
    glUseProgram(0);
    SwapBuffers(ghdc);
}

void update(void)
{
    // Code
}

void uninitialize(void)
{
    // Function Declarations
    void ToggleFullscreen(void);

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

    if(gbFullScreen == TRUE)
    {
        ToggleFullscreen();
        gbFullScreen = FALSE;
    }

    // Make the hdc as current context
    if(wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL,NULL);
    }

    // destroy rendering context 
    if(ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    // release the HDC
    if(ghdc)
    {
        ReleaseDC(ghwnd,ghdc);
        ghdc = NULL;
    }

    // Get Rid of Window Handle
    if(ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // Close the Log File
    if(gpfile)
    { 
        fprintf(gpfile,"Program Ended Successfully\n");
        fclose(gpfile);
        gpfile = NULL;
    }
}

// Following functions are analogous to glPushMatrix() and glPopMatrix()
void pushMatrix(void)
{
    // Code
    if(stackTop == MODEL_VIEW_MATRIX_STACK_DEPTH - 1)
    {
        // This is stack overflow condition
        fprintf(gpfile,"Matrix Stack is Full.\n");
    }
    else
    {
        fprintf(gpfile,"Pushing Model and View Matrix on Stack.\n");
        MODELVIEWMATRIX tempModelView;
        tempModelView.ModelMatrix = modelMatrix;
        tempModelView.ViewMatrix = viewMatrix;
        stackTop = stackTop + 1;
        modelViewMatrix[stackTop] = tempModelView;
    }
}

void popMatrix(void)
{
    // Code
    if(stackTop == -1)
    {
        fprintf(gpfile,"Matrix Stack is Empty.\n");
    }
    else
    {
        fprintf(gpfile,"Poping Model and View Matrix from Stack.\n");
        modelMatrix = modelViewMatrix[stackTop].ModelMatrix;
        viewMatrix = modelViewMatrix[stackTop].ViewMatrix;
        stackTop = stackTop - 1;
    }
}