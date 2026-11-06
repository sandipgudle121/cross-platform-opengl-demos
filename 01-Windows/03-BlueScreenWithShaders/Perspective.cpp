///Windows HeaderFiles
#include<Windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<gl/glew.h>     /* this must be before <gl/gl.h> */
#include<gl/GL.h>

#include "Perspective.h"

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
    //void resize(int,int);
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
    const GLchar* vertexShaderSourceCode =  "#version 460 core\n" \
                                            "\n" \
                                            "void main(void)\n" \
                                            "{\n" \
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
                                            "void main(void)\n" \
                                            "{\n" \
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
    
    
    // Enabling Depth
    glClearDepth(1.0f);         // Compulsory
    glEnable(GL_DEPTH_TEST);    // Compulsory
    glDepthFunc(GL_LEQUAL);     // Compulsaory

    // Set the clear of window to blue
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);       // here OpenGL starts

    //resize(WIN_WIDTH,WIN_HEIGHT);

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
    
}

void display(void)
{
    // Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);
    /* Here Drawing will tak place */
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

