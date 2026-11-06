///Windows HeaderFiles
#include<Windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<gl/glew.h>     /* this must be before <gl/gl.h> */
#include<gl/GL.h>

#include "Perspective.h"
#include "vmath.h"
using namespace vmath;

// MACROS
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//Link With OpenGL Library
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"glew32.lib")

//Global Function Declarations
LRESULT CALLBACK WndProc(HWND ,UINT , WPARAM, LPARAM);

// Global Variable Declaration
FILE *gpfile = NULL; // For File IO

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

BOOL  TRIANGLE = FALSE;
BOOL  CIRCLE = FALSE;
BOOL  SQUARE = FALSE;
BOOL  GRAPH = FALSE;

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
                
                case 'T':
                case 't':
                    if(TRIANGLE == TRUE)
                    {
                        TRIANGLE = FALSE;
                    }    
                    else if(TRIANGLE == FALSE)
                    {
                        TRIANGLE = TRUE;
                    }
                    break;

                case 'S':
                case 's':
                    if(SQUARE == TRUE)
                    {
                        SQUARE = FALSE;
                    }
                    else if(SQUARE == FALSE)
                    {
                        SQUARE = TRUE;
                    }
                    break;

                case 'C':
                case 'c':
                    if(CIRCLE == TRUE)
                    {
                        CIRCLE = FALSE;
                    }
                    else if(CIRCLE == FALSE)
                    {
                        CIRCLE = TRUE;
                    } 
                    break;

                case 'G' :
                case 'g' :
                    if(GRAPH == TRUE)
                    {
                        GRAPH = FALSE;
                    }                
                    else if(GRAPH == FALSE)
                    {
                        GRAPH = TRUE;
                    }
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
    void Circle(GLfloat radius, GLfloat centerX, GLfloat centerY, GLint numSides, GLfloat vertices[]);

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
                fprintf_s(gpfile,"shader program linking error log : %s \n",szInfoLog);
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
    //  Function declarations
    void Triangle(void);
    void Square(void);
    void Circle(void);


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
        Square();
    }

    // Call hollow triangle function
    if(TRIANGLE == TRUE)
    {
        Triangle();
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
        glPointSize(3);
        glDrawArrays(GL_LINE_LOOP, 0, circle_points);


        glBindVertexArray(0);
        glUseProgram(0);
    }

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

void Circle(void)
{

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