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
    SSG_ATTRIBUTE_COLOR,
    SSG_ATTRIBUTE_NORMAL
};

GLuint vao_pyramid = 0;
GLuint vbo_position_pyramid = 0;
GLuint vbo_color_pyramid = 0;
GLfloat pyramid_angle = 0.0f;
GLuint vbo_normal = 0;

GLuint modelMatrixUniform = 0;
GLuint projectionMatrixUniform = 0;
GLuint viewMatrixUniform = 0;
GLuint keyPressedUniform = 0;

GLuint light_Diffused_Uniform[2];
GLuint light_Ambient_Uniform[2];
GLuint light_Position_Uniform[2];
GLuint light_Specular_Uniform[2];

GLuint material_Diffused_Uniform = 0;
GLuint material_Ambient_Uniform = 0;
GLuint material_Specular_Uniform = 0;
GLuint material_Shininess_Uniform = 0;

struct Light
{
    vec4 ambient;
    vec4 diffused;
    vec4 specular;
    vec4 position;
};

struct Light light[2];

// GLfloat lightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f }; 
// GLfloat lightAmbient[] = { 0.1f,0.1f, 0.1f, 1.0f };      
// GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f };        
// GLfloat lightDiffused[] = { 1.0f, 1.0f, 1.0f ,1.0f }; 

GLfloat materialDiffused[] = { 1.0f, 1.0f, 1.0f, 1.0f };   
GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess = 128.0f;

BOOL bLightingEnabled = FALSE;
BOOL bAnimationEnabled = FALSE;

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
                // Render
                display();

                // Update
                if(bAnimationEnabled == TRUE)
                {
                    update();
                }
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

                case 'A' :
                case 'a' :
                    if(bAnimationEnabled == FALSE)
                    {
                        bAnimationEnabled = TRUE;
                    }
                    else
                    {
                        bAnimationEnabled = FALSE;
                    }
                break;

                case 'L' :
                case 'l' :
                    if(bLightingEnabled == FALSE)
                    {
                        bLightingEnabled = TRUE;
                    }
                    else
                    {
                        bLightingEnabled = FALSE;
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

    // Vertex Shaders 
    const GLchar* vertexShaderSourceCode =  "#version 460 core\n" \
                                            "\n" \
                                            "in vec4 aPosition;\n" \
                                            "in vec3 aNormal;\n" \
                                            "\n" \
                                            "uniform mat4 uViewMatrix;\n" \
                                            "uniform mat4 uModelMatrix;\n" \
                                            "uniform mat4 uProjectionMatrix;\n" \
                                            "uniform vec4 uLight_Position[2];\n" \
                                            "\n" \
                                            "uniform int uKeyPressed;\n" \
                                            "out vec3 oTransformed_Normals;\n" \
                                            "out vec3 oLight_Direction[2];\n" \
                                            "out vec3 oViewer_Vector;\n" \
                                            "void main(void)\n" \
                                            "{\n" \
                                            "if(uKeyPressed == 1)\n" \
                                            "{\n" \
                                            "vec4 eye_Coordinates = uViewMatrix * uModelMatrix * aPosition;\n" \
                                            "oTransformed_Normals = mat3(uViewMatrix * uModelMatrix) * aNormal;\n" \
                                            "oLight_Direction[0] = vec3(uLight_Position[0] - eye_Coordinates);\n" \
                                            "oLight_Direction[1] = vec3(uLight_Position[1] - eye_Coordinates);\n" \
                                            "oViewer_Vector = -eye_Coordinates.xyz;\n" \
                                            "}\n" \
                                            "else\n" \
                                            "{\n" \
                                            "oTransformed_Normals = vec3(0.0, 0.0, 0.0);\n" \
                                            "oLight_Direction[0] = vec3(0.0, 0.0, 0.0);\n" \
                                            "oLight_Direction[1] = vec3(0.0, 0.0, 0.0);\n" \
                                            "oViewer_Vector = vec3(0.0, 0.0, 0.0);\n" \
                                            "}\n" \
                                            "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n" \
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

    // Fragment Shader 
    const GLchar* fragmentShaderSourceCode = "#version 460 core\n" \
                                            "\n" \
                                            "in vec3 oTransformed_Normals;\n" \
                                            "in vec3 oLight_Direction[2];\n" \
                                            "in vec3 oViewer_Vector;\n" \
                                            "\n" \
                                            "uniform vec3 uLight_Ambient[2];\n" \
                                            "uniform vec3 uLight_Specular[2];\n" \
                                            "uniform vec3 uLight_Diffused[2];\n" \
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
                                            "vec3 normalized_Light_Direction[2];\n" \
                                            "vec3 lightDiffused[2];\n" \
                                            "vec3 lightAmbient[2];\n" \
                                            "vec3 reflection_Vector[2];\n" \
                                            "vec3 lightSpecular[2];\n" \
                                            "vec3 normalized_Transformed_Normals = normalize(oTransformed_Normals);\n" \
                                            "normalized_Light_Direction[0] = normalize(oLight_Direction[0]);\n" \
                                            "normalized_Light_Direction[1] = normalize(oLight_Direction[1]);\n" \
                                            "vec3 normalized_Viewer_Vector = normalize(oViewer_Vector);\n" \
                                            "\n" \
                                            "lightAmbient[0] = uLight_Ambient[0] * uMaterial_Ambient;\n" \
                                            "lightAmbient[1] = uLight_Ambient[1] * uMaterial_Ambient;\n" \
                                            "lightDiffused[0] = uLight_Diffused[0] * uMaterial_Diffused * max(dot(normalized_Light_Direction[0], normalized_Transformed_Normals), 0.0);\n" \
                                            "lightDiffused[1] = uLight_Diffused[1] * uMaterial_Diffused * max(dot(normalized_Light_Direction[1], normalized_Transformed_Normals), 0.0);\n" \
                                            "reflection_Vector[0] = reflect(-normalized_Light_Direction[0], normalized_Transformed_Normals);\n" \
                                            "reflection_Vector[1] = reflect(-normalized_Light_Direction[1], normalized_Transformed_Normals);\n" \
                                            "lightSpecular[0] = uLight_Specular[0] * uMaterial_Specular * pow(max(dot(reflection_Vector[0], normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n" \
                                            "lightSpecular[1] = uLight_Specular[1] * uMaterial_Specular * pow(max(dot(reflection_Vector[1], normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n" \
                                            "phong_ADS_Light = lightAmbient[0] + lightDiffused[0] + lightSpecular[0];\n" 
                                            "phong_ADS_Light = phong_ADS_Light + lightAmbient[1] + lightDiffused[1] + lightSpecular[1];\n" \
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
                fprintf_s(gpfile,"shader program linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialize();
    }

    /* Get Shader Uniform Locations */
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
    
  
    // Pyramid VAO
    // VAO_square (Vertex Array Object)
    glGenVertexArrays(1,&vao_pyramid);
    glBindVertexArray(vao_pyramid);

    // VBO for normals
    glGenBuffers(1, &vbo_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Cube */
    /* VBO for Position */
    glGenBuffers(1, &vbo_position_pyramid);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position_pyramid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_position), pyramid_position, GL_STATIC_DRAW);
    glVertexAttribPointer(SSG_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(SSG_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Enabling Depth
    glClearDepth(1.0f);         // Compulsory
    glEnable(GL_DEPTH_TEST);    // Compulsory
    glDepthFunc(GL_LEQUAL);     // Compulsaory

    light[0].ambient = vec4( 0.0f, 0.0f, 0.0f, 1.0 );
    light[0].diffused = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
    light[0].specular = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
    light[0].position = vec4( -2.0f, 0.0f, 0.0f, 1.0f );

    light[1].ambient = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
    light[1].diffused = vec4( 0.0f, 0.0f, 1.0f, 1.0f );
    light[1].specular = vec4( 0.0f, 0.0f, 1.0f, 1.0f );
    light[1].position = vec4( 2.0f, 0.0f, 0.0f, 1.0f );

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // here OpenGL starts

    // Initialize Orthographicprojectionmatrix 
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
    // Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);

    /* Pyramid */
    mat4 modelMatrix = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    translationMatrix = vmath::translate(0.0f,0.0f,-5.0f);
    mat4 rotationMatrix = mat4::identity();
    rotationMatrix = vmath::rotate(pyramid_angle, 0.0f,1.0f,0.0f);
    modelMatrix = translationMatrix * rotationMatrix;
   
    /* Push above mvp into vertex shaders's mvp uniform */
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1 , GL_FALSE, perspectiveProjectionMatrix);

    if(bLightingEnabled == TRUE)
    {
        glUniform1i(keyPressedUniform, 1);

        glUniform3fv(light_Ambient_Uniform[0], 1, light[0].ambient);
        glUniform3fv(light_Diffused_Uniform[0], 1, light[0].diffused);
        glUniform3fv(light_Specular_Uniform[0], 1, light[0].specular);
        glUniform4fv(light_Position_Uniform[0], 1, light[0].position);

        glUniform3fv(light_Ambient_Uniform[1], 1, light[1].ambient);
        glUniform3fv(light_Diffused_Uniform[1], 1, light[1].diffused);
        glUniform3fv(light_Specular_Uniform[1], 1, light[1].specular);
        glUniform4fv(light_Position_Uniform[1], 1, light[1].position);

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

    SwapBuffers(ghdc);
}

void update(void)
{
    // Code
    pyramid_angle = pyramid_angle + 1.5f;
    if(pyramid_angle >= 360.0f)
        pyramid_angle = pyramid_angle - 360.0f;
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
    
    // Delete vbo of normals
    if(vbo_normal)
    {
        glDeleteBuffers(1, &vbo_normal);
        vbo_normal;
    }
    
    /* Pyramid */
    /* Delete VBO of Color */
    if(vbo_color_pyramid)
    {
        glDeleteBuffers(1, &vbo_color_pyramid);
        vbo_color_pyramid = 0;
    }

    /* Delete VBO of Position */
    if(vbo_position_pyramid)
    {
        glDeleteBuffers(1, &vbo_position_pyramid);
        vbo_position_pyramid = 0;
    }

    /* Delete VAO_pyramid */
    if(vao_pyramid)
    {
        glDeleteVertexArrays(1,&vao_pyramid);
        vao_pyramid = 0;
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

