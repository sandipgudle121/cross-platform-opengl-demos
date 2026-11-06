///Windows HeaderFiles
#include<Windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<gl/glew.h>     /* this must be before <gl/gl.h> */
#include<gl/GL.h>

#include "Perspective.h"
#include "vmath.h"
#include "Sphere.h"
using namespace vmath;

// MACROS
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
// #define PI 3.14159265359

//Link With OpenGL Library
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"Sphere.lib")

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
GLuint shaderProgramObject_PV = 0;
GLuint shaderProgramObject_PF = 0;
enum 
{
    SSG_ATTRIBUTE_POSITION = 0,
    SSG_ATTRIBUTE_COLOR,
    SSG_ATTRIBUTE_NORMAL,
    SSG_ATTRIBUTE_TEXCOORD
};

GLuint vao_sphere = 0;
GLuint vbo_sphere_position = 0;
GLuint vbo_texcoord = 0;
GLuint vbo_elements = 0;
GLuint vbo_normal = 0;

GLuint numSphereElements = 0;

GLuint modelMatrixUniform = 0;
GLuint projectionMatrixUniform = 0;
GLuint viewMatrixUniform = 0;

GLuint light_Diffused_Uniform[3];
GLuint light_Ambient_Uniform[3];
GLuint light_Position_Uniform[3];
GLuint light_Specular_Uniform[3];

GLuint material_Diffused_Uniform = 0;
GLuint material_Ambient_Uniform = 0;
GLuint material_Specular_Uniform = 0;
GLuint material_Shininess_Uniform = 0;

GLuint keyPressedUniform = 0;
BOOL bLightingEnabled = FALSE;

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
GLfloat SPEED = 0.1f;

char light_type = 'v';

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

        case WM_CHAR :
            switch(LOWORD(wParam))
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
                    DestroyWindow(hwnd);
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
    
    // Per Vertex Light
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

    GLint status = NULL;
    GLint infoLoglength = NULL;
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
                fprintf_s(gpfile,"Vertex shader complilation(per vertex) : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize();
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
                fprintf_s(gpfile,"fragment shader compilation(per vertex) : %s \n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize();
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

    status = NULL;
    infoLoglength = NULL;
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
                fprintf_s(gpfile,"Vertex shader complilation(per fragment) : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize();
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
                fprintf_s(gpfile,"fragment shader compilation(per fragment) : %s \n", szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
                
            }
        }
        uninitialize();
    }

    //Shader Program for Per Vertex 
    shaderProgramObject_PV = glCreateProgram();
    glAttachShader(shaderProgramObject_PV, vertexShaderObject_PV);
    glAttachShader(shaderProgramObject_PV, fragmentShaderObject_PV);
    glBindAttribLocation(shaderProgramObject_PV, SSG_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject_PV, SSG_ATTRIBUTE_COLOR, "aColor");
    glBindAttribLocation(shaderProgramObject_PV, SSG_ATTRIBUTE_NORMAL, "aNormal");
    glLinkProgram(shaderProgramObject_PV);

    status = 0;
    infoLoglength = 0;
    szInfoLog = 0;

    glGetProgramiv(shaderProgramObject_PV, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        glGetProgramiv( shaderProgramObject_PV, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {
            szInfoLog = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObject_PV, infoLoglength, NULL,szInfoLog);
                fprintf_s(gpfile,"shader program(per vertex) linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialize();
    }

    /* Shader Program for Per Fragment */
    shaderProgramObject_PF = glCreateProgram();
    glAttachShader(shaderProgramObject_PF, vertexShaderObject_PF);
    glAttachShader(shaderProgramObject_PF, fragmentShaderObject_PF);
    glBindAttribLocation(shaderProgramObject_PF, SSG_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject_PF, SSG_ATTRIBUTE_COLOR, "aColor");
    glBindAttribLocation(shaderProgramObject_PF, SSG_ATTRIBUTE_NORMAL, "aNormal");
    glLinkProgram(shaderProgramObject_PF);

    status = 0;
    infoLoglength = 0;
    szInfoLog = 0;

    glGetProgramiv(shaderProgramObject_PF, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        glGetProgramiv( shaderProgramObject_PF, GL_INFO_LOG_LENGTH, &infoLoglength);
        if(infoLoglength > 0)
        {
            szInfoLog = (GLchar*)malloc(infoLoglength);
            if(szInfoLog != NULL)
            {
                glGetProgramInfoLog(shaderProgramObject_PF, infoLoglength, NULL,szInfoLog);
                fprintf_s(gpfile,"shader program(fragment) linking error log : %s \n",szInfoLog);
                free(szInfoLog);
                szInfoLog = NULL;
            }
        }
        uninitialize();
    }

    /* Get Shader Uniform Locations  for Per Vertex */
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject_PV, "uModelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject_PV, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject_PV, "uProjectionMatrix");

    light_Diffused_Uniform[0] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Diffused[0]");
    light_Ambient_Uniform[0] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Ambient[0]");
    light_Specular_Uniform[0] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Specular[0]");
    light_Position_Uniform[0] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Position[0]");

    light_Diffused_Uniform[1] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Diffused[1]");
    light_Ambient_Uniform[1] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Ambient[1]");
    light_Specular_Uniform[1] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Specular[1]");
    light_Position_Uniform[1] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Position[1]");

    light_Diffused_Uniform[2] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Diffused[2]");
    light_Ambient_Uniform[2] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Ambient[2]");
    light_Specular_Uniform[2] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Specular[2]");
    light_Position_Uniform[2] = glGetUniformLocation(shaderProgramObject_PV, "uLight_Position[2]");

    material_Diffused_Uniform = glGetUniformLocation(shaderProgramObject_PV, "uMaterial_Diffused");
    material_Ambient_Uniform = glGetUniformLocation(shaderProgramObject_PV, "uMaterial_Ambient");
    material_Specular_Uniform = glGetUniformLocation(shaderProgramObject_PV, "uMaterial_Specular");
    material_Shininess_Uniform = glGetUniformLocation(shaderProgramObject_PV, "uMaterial_Shininess");
    keyPressedUniform = glGetUniformLocation(shaderProgramObject_PV, "uKeyPressed");

    /* Get Shader Uniform Locations for Per Fragment */
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject_PF, "uModelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject_PF, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject_PF, "uProjectionMatrix");

    light_Diffused_Uniform[0] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Diffused[0]");
    light_Ambient_Uniform[0] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Ambient[0]");
    light_Specular_Uniform[0] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Specular[0]");
    light_Position_Uniform[0] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Position[0]");

    light_Diffused_Uniform[1] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Diffused[1]");
    light_Ambient_Uniform[1] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Ambient[1]");
    light_Specular_Uniform[1] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Specular[1]");
    light_Position_Uniform[1] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Position[1]");

    light_Diffused_Uniform[2] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Diffused[2]");
    light_Ambient_Uniform[2] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Ambient[2]");
    light_Specular_Uniform[2] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Specular[2]");
    light_Position_Uniform[2] = glGetUniformLocation(shaderProgramObject_PF, "uLight_Position[2]");

    material_Diffused_Uniform = glGetUniformLocation(shaderProgramObject_PF, "uMaterial_Diffused");
    material_Ambient_Uniform = glGetUniformLocation(shaderProgramObject_PF, "uMaterial_Ambient");
    material_Specular_Uniform = glGetUniformLocation(shaderProgramObject_PF, "uMaterial_Specular");
    material_Shininess_Uniform = glGetUniformLocation(shaderProgramObject_PF, "uMaterial_Shininess");
    keyPressedUniform = glGetUniformLocation(shaderProgramObject_PF, "uKeyPressed");
    
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
    glGenBuffers(1, &vbo_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
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
    glGenBuffers(1, &vbo_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
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

    // For Per Vertex Light
    if(light_type == 'v')
    {
        glUseProgram(shaderProgramObject_PV);

        // Sphere
        mat4 translationMatrix = mat4::identity();
        translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
        mat4 modelMatrix = translationMatrix;
        mat4 viewMatrix = mat4::identity();

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

            glUniform3fv(light_Ambient_Uniform[2], 1, light[2].ambient);
            glUniform3fv(light_Diffused_Uniform[2], 1, light[2].diffused);
            glUniform3fv(light_Specular_Uniform[2], 1, light[2].specular);
            glUniform4fv(light_Position_Uniform[2], 1, light[2].position);

            glUniform3fv(material_Diffused_Uniform, 1, materialDiffused);
            glUniform3fv(material_Ambient_Uniform, 1, materialAmbient);
            glUniform3fv(material_Specular_Uniform, 1, materialSpecular);
            glUniform1f(material_Shininess_Uniform, materialShininess);
        }
        else
        {
            glUniform1i(keyPressedUniform, 0);
        }

        glBindVertexArray(vao_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
        glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        glUseProgram(0);

    }


    // For Per Fragment Light
    if(light_type == 'f' )
    {

        glUseProgram(shaderProgramObject_PF);

        // Sphere
        mat4 translationMatrix = mat4::identity();
        translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
        mat4 modelMatrix = translationMatrix;
        mat4 viewMatrix = mat4::identity();
   
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

            glUniform3fv(light_Ambient_Uniform[2], 1, light[2].ambient);
            glUniform3fv(light_Diffused_Uniform[2], 1, light[2].diffused);
            glUniform3fv(light_Specular_Uniform[2], 1, light[2].specular);
            glUniform4fv(light_Position_Uniform[2], 1, light[2].position);

            glUniform3fv(material_Diffused_Uniform, 1, materialDiffused);
            glUniform3fv(material_Ambient_Uniform, 1, materialAmbient);
            glUniform3fv(material_Specular_Uniform, 1, materialSpecular);
            glUniform1f(material_Shininess_Uniform, materialShininess);
        }
        else
        {
            glUniform1i(keyPressedUniform, 0);
        }

        glBindVertexArray(vao_sphere);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
        glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        glUseProgram(0);

    }

    SwapBuffers(ghdc);
}

void update(void)
{
    // Code

    if(bLightingEnabled == TRUE && light_type == 'v' || light_type == 'f')
    {   
        light[0].position[0] = 0.0f; 
        light[0].position[1] = 2.0f * (GLfloat)cos(light_angle0 * M_PI / 180.0f); 
        light[0].position[2] = 2.0f * (GLfloat)sin(light_angle0 * M_PI / 180.0f); 
        light[0].position[3] = 1.0f;
 
        light_angle0 = light_angle0 + SPEED;
        if(light_angle0 > 360.0f)
            light_angle0 = light_angle0 - 360.0f;
        
        light[1].position[0] = 2.0f * (GLfloat)cos(light_angle1 * M_PI / 180.0f);
        light[1].position[1] = 0.0f; 
        light[1].position[2] = 2.0f * (GLfloat)sin(light_angle1 * M_PI / 180.0f);
        light[1].position[3] = 1.0f;

        light_angle1 = light_angle1 + SPEED;
        if(light_angle1 > 360.0f)
            light_angle1 = light_angle1 - 360.0f;
       
        light[2].position[0] = 20.0f * (GLfloat)sin(light_angle2 * M_PI / 180.0f);
        light[2].position[1] = 20.0f * (GLfloat)cos(light_angle2 * M_PI / 180.0f);
        light[2].position[2] = 0.0f;
        light[2].position[3] = 1.0f;

        light_angle2 = light_angle2 + SPEED ;
        if(light_angle2 > 360.0f)
            light_angle2 = light_angle2 - 360.0f;
    }
}

void uninitialize(void)
{
    // Function Declarations
    void ToggleFullscreen(void);

    // Code
    //For Per Vertex Light
    if(light_type == 'v')
    {
        if(shaderProgramObject_PV)
        {
            glUseProgram(shaderProgramObject_PV);
        
            GLint numShaders = 0;

            glGetProgramiv(shaderProgramObject_PV, GL_ATTACHED_SHADERS, &numShaders);
            if(numShaders > 0)
            {
                GLuint *pShaders = (GLuint*)malloc(numShaders * sizeof(GLuint));
                if(pShaders != NULL)
                {
                    glGetAttachedShaders(shaderProgramObject_PV, numShaders, NULL, pShaders);
            
                    for(GLint i = 0; i < numShaders; i++)
                    {
                        glDetachShader(shaderProgramObject_PV, pShaders[i]);
                        glDeleteShader(pShaders[i] );
                        pShaders[i] = 0;
                    }
        
                    free(pShaders);
                    pShaders = NULL;
                }
            }

            glUseProgram(0);
            glDeleteProgram(shaderProgramObject_PV);
            shaderProgramObject_PV = 0;
        }
    }

    if(light_type == 'f')
    {

        // For Per Fragment Light
        if(shaderProgramObject_PF)
        {
            glUseProgram(shaderProgramObject_PF);
        
            GLint numShaders = 0;

            glGetProgramiv(shaderProgramObject_PF, GL_ATTACHED_SHADERS, &numShaders);
            if(numShaders > 0)
            {
                GLuint *pShaders = (GLuint*)malloc(numShaders * sizeof(GLuint));
                if(pShaders != NULL)
                {
                    glGetAttachedShaders(shaderProgramObject_PF, numShaders, NULL, pShaders);
            
                    for(GLint i = 0; i < numShaders; i++)
                    {
                        glDetachShader(shaderProgramObject_PF, pShaders[i]);
                        glDeleteShader(pShaders[i] );
                        pShaders[i] = 0;
                    }
        
                    free(pShaders);
                    pShaders = NULL;
                }
            }

            glUseProgram(0);
            glDeleteProgram(shaderProgramObject_PF);
            shaderProgramObject_PF = 0;
        }
    }
    
    // Delete vbo of normals
    if(vbo_normal)
    {
        glDeleteBuffers(1, &vbo_normal);
        vbo_normal = 0;
    }

    // Delete vbo of texcoords
    if(vbo_texcoord)
    {
        glDeleteBuffers(1, &vbo_texcoord);
        vbo_texcoord = 0;
    }

    // Delete vbo of elements
    if(vbo_elements)
    {
        glDeleteBuffers(1, &vbo_elements);
        vbo_elements = 0;
    }

    /* DELETE VAO sphere */
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

