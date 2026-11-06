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

// ASCII values of 0 t 9 number key on main keyboard
#define VK_KEY_0 0x30
#define VK_KEY_1 0x31
#define VK_KEY_2 0x32
#define VK_KEY_3 0x33
#define VK_KEY_4 0x34
#define VK_KEY_5 0x35
#define VK_KEY_6 0x36
#define VK_KEY_7 0x37
#define VK_KEY_8 0x38
#define VK_KEY_9 0x39

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
    SSG_ATTRIBUTE_COLOR,
    SSG_ATTRIBUTE_TEXCOORD
};

GLuint vao_square = 0;
GLuint vbo_position_square = 0;
GLuint vbo_texcord = 0;
GLuint texture_smiley = 0;

GLuint mvpMatrixUniform = 0;
GLuint textureSamplerUniform = 0;
GLuint keyPressUniform = 0;

int keyPressed = 0;

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

                case VK_NUMPAD1 :
                case VK_KEY_1 :
                    keyPressed = 1;
                    break;

                case VK_NUMPAD2 :
                case VK_KEY_2 :
                    keyPressed = 2;
                    break;

                case VK_NUMPAD3 :
                case VK_KEY_3 :
                    keyPressed = 3;
                    break;

                case VK_NUMPAD4 :
                case VK_KEY_4 :
                    keyPressed = 4;
                    break;

                default :
                    keyPressed = 0;
                    glDisable(GL_TEXTURE_2D);
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
    void resize(int,int);
    void printGLInfo(void);
    void uninitialize(void);
    BOOL loadGlTexture(GLuint*,TCHAR[]);

    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;
    ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
    BOOL bResult;

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
    const GLchar* fragmentShaderSourceCode ="#version 460 core\n" \
                                            "\n" \
                                            "in vec2 oTexCoord;\n" \
                                            "uniform sampler2D uTextureSampler;\n" \
                                            "uniform int uKeyPressed;\n" \
                                            "out vec4 FragColor;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "if(uKeyPressed == 0)\n" \
                                            "{\n" \
                                            "FragColor = vec4(1.0,1.0,1.0,1.0);\n" \
                                            "}\n" \
                                            "else\n" \
                                            "{\n" \
                                            "FragColor = texture(uTextureSampler, oTexCoord);\n" \
                                            "}\n" \
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
                fprintf_s(gpfile,"shader program linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialize();
    }

    /* Get Shader Uniform Locations */
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "uTextureSampler");
    keyPressUniform = glGetUniformLocation(shaderProgramObject, "uKeyPressed");

    const GLfloat square_position[] = 
                                        {
                                           1.0f,1.0f,0.0f,
                                           -1.0f,1.0f,0.0f,
                                           -1.0f,-1.0f,0.0f,
                                           1.0f,-1.0f,0.0f
                                        };
    
    // Square 
    // VAO_square (Vertex Array Object )
    glGenVertexArrays(1,&vao_square);
    glBindVertexArray(vao_square);

    /* VBO for Position */
    glGenBuffers(1, &vbo_position_square);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_square);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_position), square_position, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // vbo for texcord
    glGenBuffers(1, &vbo_texcord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcord);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_TEXCOORD);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Enabling Depth
    glClearDepth(1.0f);         // Compulsory
    glEnable(GL_DEPTH_TEST);    // Compulsory
    glDepthFunc(GL_LEQUAL);     // Compulsaory

    // Set the clear of window to blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // here OpenGL starts

    // Create Texture
    bResult = loadGlTexture(&texture_smiley, MAKEINTRESOURCE(MY_SMILEY_BITMAP));
    if(bResult ==  FALSE)
    {   
        fprintf(gpfile,"Loading of kundali texture failed\n");
        return(-7);
    }

    /* Initialize Orthographicprojectionmatrix */
    perspectiveProjectionMatrix = vmath::mat4::identity();

    // Tell OpenGL to enable texture
    glEnable(GL_TEXTURE_2D);
    resize(WIN_WIDTH,WIN_HEIGHT);

    return(0);
}

BOOL loadGlTexture(GLuint* texture, TCHAR imageResourceId[])
{
    // Local variable declarations
    HBITMAP hBitmap = NULL;
    BITMAP bmp;

    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL),imageResourceId,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
    if(hBitmap == NULL)
    {
        fprintf(gpfile,"load image failed\n");
        return(-8);
    } 

    // Get image data
    GetObject(hBitmap,sizeof(BITMAP),&bmp);

    glGenTextures(1,texture);
    glBindTexture(GL_TEXTURE_2D,*texture);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); 

    // Create multiple mipmap images
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0 ,GL_BGR, GL_UNSIGNED_BYTE, (void*)bmp.bmBits);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    DeleteObject(hBitmap);
    hBitmap = NULL;

    return(TRUE);
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
    // Variables 
    GLfloat square_texcoord[8];

    // Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);

    /* Squre */
    mat4 modelViewMatrix = mat4::identity();
    modelViewMatrix = vmath::translate(0.0f,0.0f,-6.0f);
    mat4 modelViewProjectionmatri = perspectiveProjectionMatrix * modelViewMatrix; // order of multiplication is very importatnt

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_smiley);
    glUniform1i(textureSamplerUniform, 0);

    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionmatri);
    glBindVertexArray(vao_square);

    // Texture Coordinates
    if(keyPressed == 1)
    {
        square_texcoord[0] = 1.0f;
        square_texcoord[1] = 1.0f;
        square_texcoord[2] = 0.0f;
        square_texcoord[3] = 1.0f;
        square_texcoord[4] = 0.0f;
        square_texcoord[5] = 0.0f;
        square_texcoord[6] = 1.0f;
        square_texcoord[7] = 0.0f;

        glUniform1i(keyPressUniform,1);
    } 
    else if(keyPressed == 2)
    {
        square_texcoord[0] = 0.5f;
        square_texcoord[1] = 0.5f;
        square_texcoord[2] = 0.0f;
        square_texcoord[3] = 0.5f;
        square_texcoord[4] = 0.0f;
        square_texcoord[5] = 0.0f;
        square_texcoord[6] = 0.5f;
        square_texcoord[7] = 0.0f;

        glUniform1i(keyPressUniform,1);
    }
    else if(keyPressed == 3)
    {
        square_texcoord[0] = 2.0f;
        square_texcoord[1] = 2.0f;
        square_texcoord[2] = 0.0f;
        square_texcoord[3] = 2.0f;
        square_texcoord[4] = 0.0f;
        square_texcoord[5] = 0.0f;
        square_texcoord[6] = 2.0f;
        square_texcoord[7] = 0.0f;

        glUniform1i(keyPressUniform,1);
    }
    else if(keyPressed == 4)
    {
        square_texcoord[0] = 0.5f;
        square_texcoord[1] = 0.5f;
        square_texcoord[2] = 0.5f;
        square_texcoord[3] = 0.5f;
        square_texcoord[4] = 0.5f;
        square_texcoord[5] = 0.5f;
        square_texcoord[6] = 0.5f;
        square_texcoord[7] = 0.5f;

        glUniform1i(keyPressUniform,1);
    }
    else
    {
        glUniform1i(keyPressUniform, 0);         ;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_texcoord), square_texcoord, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0 , 4 );
    glBindVertexArray(0);
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

    // Delete texture texcords
    if(vbo_texcord)
    {
        glDeleteBuffers(1, &vbo_texcord);
        vbo_texcord = 0;
    }

    /* Delete VBO of Position */
    if(vbo_position_square)
    {
        glDeleteBuffers(1, &vbo_position_square);
        vbo_position_square = 0;
    }

    /* Delete VAO_square */
    if(vao_square)
    {
        glDeleteVertexArrays(1,&vao_square);
        vao_square = 0;
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

    // Delete texture
    if(texture_smiley)
    {
        glDeleteTextures(1,&texture_smiley);
        texture_smiley = 0;
    }

    // Close the Log File
    if(gpfile)
    { 
        fprintf(gpfile,"Program Ended Successfully\n");
        fclose(gpfile);
        gpfile = NULL;
    }
}

