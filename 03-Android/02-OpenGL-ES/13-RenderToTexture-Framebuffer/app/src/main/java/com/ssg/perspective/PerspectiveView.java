package com.ssg.perspective;

import android.content.Context;

// OpenGL packages
import android.opengl.GLSurfaceView;
import android.opengl.GLES32;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;

// Java I/O
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

import android.opengl.Matrix;


// Event related packages
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnDoubleTapListener;
import android.view.GestureDetector.OnGestureListener;

public class PerspectiveView extends GLSurfaceView implements GLSurfaceView.Renderer, OnDoubleTapListener, OnGestureListener
{
    // Class variables
    private GestureDetector gesture_detector;
    
    private int shaderProgramObject_cube;
    
    private int[] vao_cube = new int[1];
    private int[] vbo_cube_position = new int[1];
    private int[] vbo_cube_texcoord = new int[1];

    private float angle_cube = 0.0f;

    private int mvpMatrixUniform_cube;
    private int textureSamplerUniform_cube;
    private float[] perspectiveProjectionMatrix_cube = new float[16]; 


    // Sphere related Code
    private int shaderProgramObject_PV;
    private int shaderProgramObject_PF;

    private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];

    private float[] light_Diffused_1 = {1.0f, 0.0f, 0.0f, 1.0f};
    private float[] light_Ambient_1 = { 0.0f, 0.0f, 0.0f, 1.0f};
    private float[] light_Specular_1 = { 1.0f,0.0f, 0.0f, 1.0f};
    private float[] light_Position_1 = {0.0f, 0.0f, 0.0f, 1.0f};

    private float[] light_Diffused_2 = {0.0f, 1.0f, 0.0f, 1.0f};
    private float[] light_Ambient_2 = { 0.0f, 0.0f, 0.0f, 1.0f};
    private float[] light_Specular_2 = { 0.0f, 1.0f, 0.0f, 1.0f};
    private float[] light_Position_2 = {0.0f, 0.0f, 0.0f, 1.0f};

    private float[] light_Diffused_3 = {0.0f, 0.0f, 1.0f, 1.0f};
    private float[] light_Ambient_3 = { 0.0f, 0.0f, 0.0f, 1.0f};
    private float[] light_Specular_3 = { 0.0f, 0.0f, 1.0f, 1.0f};
    private float[] light_Position_3 = {0.0f, 0.0f, 0.0f, 1.0f};

    private float[] materialDiffused = {1.0f, 1.0f, 1.0f, 1.0f};
    private float[] materialAmbient = {0.0f, 0.0f, 0.0f, 1.0f};
    private float[] materialSpecular = {1.0f, 1.0f, 1.0f, 1.0f};
    private float materialShininess = 128.0f;

    private int numVertices;
    private int numElements;

    private float angle_light = 0.0f;
    private float SPEED = 0.01f;
    private char light_type = 'v';
    private boolean lightEnabled = false;
    private int tapUniform;

    private int[] lightDiffusedUniform = new int[3];
    private int[] lightAmbientUniform = new int[3];
    private int[] lightSpecularUniform = new int[3];
    private int[] lightPositionUniform = new int[3];

    private int materialDiffusedUniform = 0;
    private int materialAmbientUniform = 0;
    private int materialSpecularUniform = 0;
    private int materialShininessUniform = 0;

    private int viewMatrixUniform_sphere;
    private int modelMatrixUniform_sphere;
    private int projectionMatrixUniform_sphere;
    
    private float[] perspectiveProjectionMatrix_sphere = new float[16];


    // FBO related variables
    private int FBO;
    private int RBO;
    private int[] texture_FBO = new int[1];
    boolean FBO_result = false;

    private int winWidth;
    private int winHeight;
    private int FBO_WIDTH = 512;
    private int FBO_HEIGHT = 512;

    public PerspectiveView(Context context)
    {
        super(context);

        // OpenGL-ES related 
        setEGLContextClientVersion(3);
        setRenderer(this);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        // Event related
        // Create and set gesture detector object
        gesture_detector = new GestureDetector(context, this, null, false);
        gesture_detector.setOnDoubleTapListener(this);
    }

    // Implementation of three methods of GLSurfaceView
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        initialize_cube(gl);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
        // Code
        resize_cube(width, height);
        winWidth = width;
        winHeight = height;
    }

    @Override
    public void onDrawFrame(GL10 gl)
    {
        // Code 
        display_cube();

        update_cube();
    }

    // Implementation on-touch event of view class
    @Override
    public boolean onTouchEvent(MotionEvent e)
    {
        if(!gesture_detector.onTouchEvent(e))
        {
            super.onTouchEvent(e);
        }

        return(true);
    }

    // Implementation of on three methods of on double tap listner interface
    @Override
    public boolean onDoubleTap(MotionEvent e)
    {
        if(lightEnabled == false)
        {
            lightEnabled = true;
        }
        else
        {
            lightEnabled = false;
        }
        return(true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
    {
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        if(light_type == 'v') 
        {
            light_type = 'f';
        } 
        else 
        {
            light_type = 'v';
        }
        return(true);
    }

    // Six methods of gesture listner interface
    @Override
    public boolean onDown(MotionEvent e)
    {
        return(true);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {
        return(true);
    }

    @Override
    public void onLongPress(MotionEvent e)
    {
  
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
    {
        // Code
        uninitialize_cube();

        System.exit(0);
        
        return(true);
    }

    @Override
    public void  onShowPress(MotionEvent e)
    {

    }

    @Override
    public boolean onSingleTapUp(MotionEvent e)
    {
        return(true);
    }

    // Implementation of private method
    private int initialize_cube(GL10 gl)
    {
        // Code
        // Print OpenGL ES Info
        printGLInfo(gl);

        // Vertex shader
        final String vertexShaderSourceCode = String.format
        (   
            "   #version 320 es                                         \n"+
            "   in vec4 aPosition;                                      \n"+
            "   in vec2 aTexCoord;                                      \n"+
            "   out vec2 oTexCoord;                                     \n"+
            "   uniform mat4 uMVPMatrix;                                \n"+
            "   void main(void)                                         \n"+
            "   {                                                       \n"+
            "       gl_Position = uMVPMatrix * aPosition;               \n"+
            "       oTexCoord = aTexCoord;                              \n"+
            "   }                                                       \n"
        );
        
        int vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);
        GLES32.glCompileShader(vertexShaderObject);

        int[] shaderCompileStatus = new int[1];
        int[] infoLogLength = new int[1];
        String infoLog = null;

        GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS, shaderCompileStatus, 0);
        if(shaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);
                System.out.println("SSG: Vertex shader complilation :" + infoLog);
                uninitialize_cube();
                System.exit(0);
            }
        }

        // Fragment shader
        final String fragmentShaderSourceCode = String.format
        (   
            "   #version 320 es                                         \n"+
            "   precision highp float;                                  \n"+
            "   in vec2 oTexCoord;                                      \n"+
            "   uniform sampler2D uTextureSampler;                      \n"+
            "   out vec4 FragColor;                                     \n"+
            "   void main(void)                                         \n"+
            "   {                                                       \n"+
            "       FragColor = texture(uTextureSampler, oTexCoord);    \n"+
            "   }                                                       \n"
        );
        
        int fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        GLES32.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);
        GLES32.glCompileShader(fragmentShaderObject);

        shaderCompileStatus[0] = 0;
        infoLogLength[0] = 0;
        infoLog = null;

        GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS, shaderCompileStatus, 0);
        if(shaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);
                System.out.println("SSG: Fragment shader complilation :" + infoLog);
                uninitialize_cube();
                System.exit(0);
            }
        }

        // Shader program
        shaderProgramObject_cube = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramObject_cube, vertexShaderObject);
        GLES32.glAttachShader(shaderProgramObject_cube, fragmentShaderObject);
        GLES32.glBindAttribLocation(shaderProgramObject_cube, VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, "aPosition");
        GLES32.glBindAttribLocation(shaderProgramObject_cube, VertexAttributesEnum.SSG_ATTRIBUTE_TEXCOORD, "aTexCoord");
        GLES32.glLinkProgram(shaderProgramObject_cube);

        int[] programLinkStatus = new int[1];
        infoLogLength[0] = 0;
        infoLog = null;

        GLES32.glGetProgramiv(shaderProgramObject_cube, GLES32.GL_LINK_STATUS, programLinkStatus, 0);
        if(programLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject_cube, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetProgramInfoLog(shaderProgramObject_cube);
                System.out.println("SSG: Shader program link log :" + infoLog);
                uninitialize_cube();
                System.exit(0);
            }
        }

        // Get Shader Uniform Locations 
        mvpMatrixUniform_cube = GLES32.glGetUniformLocation(shaderProgramObject_cube, "uMVPMatrix");
        textureSamplerUniform_cube = GLES32.glGetUniformLocation(shaderProgramObject_cube, "uTextureSampler");

        // Geometry
        final float[] cube_position = new float[]
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

        final float[] cube_texcoord = new float[]
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


        // Vao_cube
        GLES32.glGenVertexArrays(1, vao_cube, 0);
        GLES32.glBindVertexArray(vao_cube[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, vbo_cube_position, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cube_position[0]);

        // Prepare triangle_cube vertics array for glBufferData
        ByteBuffer bytebuffer = ByteBuffer.allocateDirect(cube_position.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        FloatBuffer positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(cube_position);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cube_position.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);


        // VBO Texcoord
        GLES32.glGenBuffers(1, vbo_cube_texcoord, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cube_texcoord[0]);

        bytebuffer = ByteBuffer.allocateDirect(cube_texcoord.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        FloatBuffer textureBuffer = bytebuffer.asFloatBuffer();
        textureBuffer.put(cube_texcoord);
        textureBuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cube_texcoord.length * 4, textureBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_TEXCOORD, 2, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_TEXCOORD);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        // Depth enable settings
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        // Clear color
        GLES32.glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        Matrix.setIdentityM(perspectiveProjectionMatrix_cube, 0);

        // FBO-related code
        if (create_FBO(FBO_WIDTH, FBO_HEIGHT) == true) 
        {
            FBO_result = initialize_sphere(FBO_WIDTH, FBO_HEIGHT);
        }

        return(0);
    }

    public boolean initialize_sphere(int textureWidth, int textureHeight)
    {
        // Code
        // Vertex shader of per vertex light
        final String vertexShaderSourceCode_PV = String.format
        (   
            "#version 320 es\n"+
            "\n"+
            "in vec4 aPosition;\n"+
            "in vec3 aNormal;\n"+
            "\n"+
            "uniform mat4 uViewMatrix;\n"+
            "uniform mat4 uModelMatrix;\n"+
            "uniform mat4 uProjectionMatrix;\n"+
            "\n"+
            "uniform vec3 uLight_Ambient[3];\n"+
            "uniform vec3 uLight_Specular[3];\n"+
            "uniform vec3 uLight_Diffused[3];\n"+
            "uniform vec4 uLight_Position[3];\n"+
            "\n"+
            "uniform vec3 uMaterial_Diffused;\n"+
            "uniform vec3 uMaterial_Ambient;\n"+
            "uniform vec3 uMaterial_Specular;\n"+
            "uniform float uMaterial_Shininess;\n"+
            "\n"+
            "uniform int uKeyPressed;\n"+
            "out vec3 oPhong_ADS_Light;\n"+
            "void main(void)\n"+
            "{\n"+
            "if(uKeyPressed == 1)\n"+
            "{\n"+
            "vec4 eye_Coordinates = uViewMatrix * uModelMatrix * aPosition;\n"+
            "vec3 transformed_Normals = normalize(mat3(uViewMatrix * uModelMatrix) * aNormal);\n"+
            "vec3 lightAmbient[3];\n"+
            "vec3 lightDiffused[3];\n"+
            "vec3 lightSpecular[3];\n"+
            "vec3 reflection_Vector[3];\n"+
            "vec3 light_Direction[3];\n"+
            "light_Direction[0] = normalize(vec3(uLight_Position[0] - eye_Coordinates));\n"+
            "light_Direction[1] = normalize(vec3(uLight_Position[1] - eye_Coordinates));\n"+
            "light_Direction[2] = normalize(vec3(uLight_Position[2] - eye_Coordinates));\n"+
            "reflection_Vector[0] = reflect(-light_Direction[0], transformed_Normals);\n"+
            "reflection_Vector[1] = reflect(-light_Direction[1], transformed_Normals);\n"+
            "reflection_Vector[2] = reflect(-light_Direction[2], transformed_Normals);\n"+
            "vec3 viewer_Vector = normalize(-eye_Coordinates.xyz);\n"+
            "lightAmbient[0] = uLight_Ambient[0] * uMaterial_Ambient;\n"+
            "lightAmbient[1] = uLight_Ambient[1] * uMaterial_Ambient;\n"+
            "lightAmbient[2] = uLight_Ambient[2] * uMaterial_Ambient;\n"+
            "lightDiffused[0] = uLight_Diffused[0] * uMaterial_Diffused * max(dot(light_Direction[0], transformed_Normals), 0.0);\n"+
            "lightDiffused[1] = uLight_Diffused[1] * uMaterial_Diffused * max(dot(light_Direction[1], transformed_Normals), 0.0);\n"+
            "lightDiffused[2] = uLight_Diffused[2] * uMaterial_Diffused * max(dot(light_Direction[2], transformed_Normals), 0.0);\n"+
            "lightSpecular[0] = uLight_Specular[0] * uMaterial_Specular * pow(max(dot(reflection_Vector[0], viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
            "lightSpecular[1] = uLight_Specular[1] * uMaterial_Specular * pow(max(dot(reflection_Vector[1], viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
            "lightSpecular[2] = uLight_Specular[2] * uMaterial_Specular * pow(max(dot(reflection_Vector[2], viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
            "oPhong_ADS_Light = lightAmbient[0] + lightDiffused[0] + lightSpecular[0];\n"+
            "oPhong_ADS_Light = oPhong_ADS_Light + lightAmbient[1] + lightDiffused[1] + lightSpecular[1];\n"+
            "oPhong_ADS_Light = oPhong_ADS_Light + lightAmbient[2] + lightDiffused[2] + lightSpecular[2];\n"+
            "}\n"+
            "else\n"+
            "{\n"+
            "oPhong_ADS_Light = vec3(0.0f, 0.0f, 0.0f);\n"+
            "}\n"+
            "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"+
            "}\n"                                                   
        );
        
        int vertexShaderObject_PV = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        GLES32.glShaderSource(vertexShaderObject_PV, vertexShaderSourceCode_PV);
        GLES32.glCompileShader(vertexShaderObject_PV);

        int[] shaderCompileStatus = new int[1];
        int[] infoLogLength = new int[1];
        String infoLog = null;

        GLES32.glGetShaderiv(vertexShaderObject_PV, GLES32.GL_COMPILE_STATUS, shaderCompileStatus, 0);
        if(shaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject_PV, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetShaderInfoLog(vertexShaderObject_PV);
                System.out.println("SSG: Vertex shader complilation of per vertex light :" + infoLog);
                uninitialize_sphere();
                System.exit(0);
            }
        }

        // Fragment shader of per vertex light
        final String fragmentShaderSourceCode_PV = String.format
        (   
            "#version 320 es\n"+
            "\n"+
            "precision highp  float;"+
            "precision highp  int;"+
            "in vec3 oPhong_ADS_Light;\n"+
            "uniform int uKeyPressed;\n"+
            "out vec4 FragColor;\n"+
            "void main(void)\n"+
            "{\n"+
            "if(uKeyPressed == 1)\n"+
            "{\n"+
            "FragColor = vec4(oPhong_ADS_Light, 1.0);\n"+
            "}\n"+
            "else\n"+
            "{\n"+
            "FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"+
            "}\n"+
            "}\n"               
        );
        
        int fragmentShaderObject_PV = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        GLES32.glShaderSource(fragmentShaderObject_PV, fragmentShaderSourceCode_PV);
        GLES32.glCompileShader(fragmentShaderObject_PV);

        shaderCompileStatus[0] = 0;
        infoLogLength[0] = 0;
        infoLog = null;

        GLES32.glGetShaderiv(fragmentShaderObject_PV, GLES32.GL_COMPILE_STATUS, shaderCompileStatus, 0);
        if(shaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject_PV, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject_PV);
                System.out.println("SSG: Fragment shader complilation of per vertex light :" + infoLog);
                uninitialize_sphere();
                System.exit(0);
            }
        }

        // Shader program of per vertex light
        shaderProgramObject_PV = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramObject_PV, vertexShaderObject_PV);
        GLES32.glAttachShader(shaderProgramObject_PV, fragmentShaderObject_PV);
        GLES32.glBindAttribLocation(shaderProgramObject_PV, VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, "aPosition");
        GLES32.glBindAttribLocation(shaderProgramObject_PV, VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL, "aNormal");
        GLES32.glLinkProgram(shaderProgramObject_PV);

        int[] programLinkStatus = new int[1];
        infoLogLength[0] = 0;
        infoLog = null;

        GLES32.glGetProgramiv(shaderProgramObject_PV, GLES32.GL_LINK_STATUS, programLinkStatus, 0);
        if(programLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject_PV, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetProgramInfoLog(shaderProgramObject_PV);
                System.out.println("SSG: Shader program link log of per vertex light :" + infoLog);
                uninitialize_sphere();
                System.exit(0);
            }
        }

        // Vertex shader
        final String vertexShaderSourceCode_PF = String.format
        (   
            "#version 320 es\n"+
            "\n"+
            "in vec4 aPosition;\n"+
            "in vec3 aNormal;\n"+
            "\n"+
            "uniform mat4 uViewMatrix;\n"+
            "uniform mat4 uModelMatrix;\n"+
            "uniform mat4 uProjectionMatrix;\n"+
            "uniform vec4 uLight_Position[3];\n"+
            "\n"+
            "uniform int uKeyPressed;\n"+
            "out vec3 oTransformed_Normals;\n"+
            "out vec3 oLight_Direction[3];\n"+
            "out vec3 oViewer_Vector;\n"+
            "void main(void)\n"+
            "{\n"+
            "if(uKeyPressed == 1)\n"+
            "{\n"+
            "vec4 eye_Coordinates = uViewMatrix * uModelMatrix * aPosition;\n"+
            "oTransformed_Normals = mat3(uViewMatrix * uModelMatrix) * aNormal;\n"+
            "oLight_Direction[0] = vec3(uLight_Position[0] - eye_Coordinates);\n"+
            "oLight_Direction[1] = vec3(uLight_Position[1] - eye_Coordinates);\n"+
            "oLight_Direction[2] = vec3(uLight_Position[2] - eye_Coordinates);\n"+
            "oViewer_Vector = -eye_Coordinates.xyz;\n"+
            "}\n"+
            "else\n"+
            "{\n"+
            "oTransformed_Normals = vec3(0.0, 0.0, 0.0);\n"+
            "oLight_Direction[0] = vec3(0.0, 0.0, 0.0);\n"+
            "oLight_Direction[1] = vec3(0.0, 0.0, 0.0);\n"+
            "oLight_Direction[2] = vec3(0.0, 0.0, 0.0);\n"+
            "oViewer_Vector = vec3(0.0, 0.0, 0.0);\n"+
            "}\n"+
            "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"+
            "}\n"                                                   
        );
        
        int vertexShaderObject_PF = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        GLES32.glShaderSource(vertexShaderObject_PF, vertexShaderSourceCode_PF);
        GLES32.glCompileShader(vertexShaderObject_PF);

        shaderCompileStatus = new int[1];
        infoLogLength = new int[1];
        infoLog = null;

        GLES32.glGetShaderiv(vertexShaderObject_PF, GLES32.GL_COMPILE_STATUS, shaderCompileStatus, 0);
        if(shaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject_PF, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetShaderInfoLog(vertexShaderObject_PF);
                System.out.println("SSG: Vertex shader complilation :" + infoLog);
                uninitialize_sphere();
                System.exit(0);
            }
        }

        // Fragment shader for per fragment light
        final String fragmentShaderSourceCode_PF = String.format
        (   
            "#version 320 es\n"+
            "\n"+
            "precision highp  float;"+
            "precision highp  int;"+
            "in vec3 oTransformed_Normals;\n"+
            "in vec3 oLight_Direction[3];\n"+
            "in vec3 oViewer_Vector;\n"+
            "\n"+
            "uniform vec3 uLight_Ambient[3];\n"+
            "uniform vec3 uLight_Specular[3];\n"+
            "uniform vec3 uLight_Diffused[3];\n"+
            "\n"+
            "uniform vec3 uMaterial_Diffused;\n"+
            "uniform vec3 uMaterial_Ambient;\n"+
            "uniform vec3 uMaterial_Specular;\n"+
            "uniform float uMaterial_Shininess;\n"+
            "\n"+
            "uniform int uKeyPressed;\n"+
            "out vec4 FragColor;\n"+
            "void main(void)\n"+
            "{\n"+
            "vec3 phong_ADS_Light;\n"+
            "if(uKeyPressed == 1)\n"+
            "{\n"+
            "vec3 normalized_Light_Direction[3];\n"+
            "vec3 lightDiffused[3];\n"+
            "vec3 lightAmbient[3];\n"+
            "vec3 reflection_Vector[3];\n"+
            "vec3 lightSpecular[3];\n"+
            "vec3 normalized_Transformed_Normals = normalize(oTransformed_Normals);\n"+
            "normalized_Light_Direction[0] = normalize(oLight_Direction[0]);\n"+
            "normalized_Light_Direction[1] = normalize(oLight_Direction[1]);\n"+
            "normalized_Light_Direction[2] = normalize(oLight_Direction[2]);\n"+
            "vec3 normalized_Viewer_Vector = normalize(oViewer_Vector);\n"+
            "\n"+
            "lightAmbient[0] = uLight_Ambient[0] * uMaterial_Ambient;\n"+
            "lightAmbient[1] = uLight_Ambient[1] * uMaterial_Ambient;\n"+
            "lightAmbient[2] = uLight_Ambient[2] * uMaterial_Ambient;\n"+
            "lightDiffused[0] = uLight_Diffused[0] * uMaterial_Diffused * max(dot(normalized_Light_Direction[0], normalized_Transformed_Normals), 0.0);\n"+
            "lightDiffused[1] = uLight_Diffused[1] * uMaterial_Diffused * max(dot(normalized_Light_Direction[1], normalized_Transformed_Normals), 0.0);\n"+
            "lightDiffused[2] = uLight_Diffused[2] * uMaterial_Diffused * max(dot(normalized_Light_Direction[2], normalized_Transformed_Normals), 0.0);\n"+
            "reflection_Vector[0] = reflect(-normalized_Light_Direction[0], normalized_Transformed_Normals);\n"+
            "reflection_Vector[1] = reflect(-normalized_Light_Direction[1], normalized_Transformed_Normals);\n"+
            "reflection_Vector[2] = reflect(-normalized_Light_Direction[2], normalized_Transformed_Normals);\n"+
            "lightSpecular[0] = uLight_Specular[0] * uMaterial_Specular * pow(max(dot(reflection_Vector[0], normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
            "lightSpecular[1] = uLight_Specular[1] * uMaterial_Specular * pow(max(dot(reflection_Vector[1], normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
            "lightSpecular[2] = uLight_Specular[2] * uMaterial_Specular * pow(max(dot(reflection_Vector[2], normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
            "phong_ADS_Light = lightAmbient[0] + lightDiffused[0] + lightSpecular[0];\n"+
            "phong_ADS_Light = phong_ADS_Light + lightAmbient[1] + lightDiffused[1] + lightSpecular[1];\n"+
            "phong_ADS_Light = phong_ADS_Light + lightAmbient[2] + lightDiffused[2] + lightSpecular[2];\n"+
            "}\n"+
            "else\n"+
            "{\n"+
            "phong_ADS_Light = vec3(1.0, 1.0, 1.0);\n"+
            "}\n"+
            "FragColor = vec4(phong_ADS_Light, 1.0);\n"+
            "}\n"                             
        );
        
        int fragmentShaderObject_PF = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        GLES32.glShaderSource(fragmentShaderObject_PF, fragmentShaderSourceCode_PF);
        GLES32.glCompileShader(fragmentShaderObject_PF);

        shaderCompileStatus[0] = 0;
        infoLogLength[0] = 0;
        infoLog = null;

        GLES32.glGetShaderiv(fragmentShaderObject_PF, GLES32.GL_COMPILE_STATUS, shaderCompileStatus, 0);
        if(shaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject_PF, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject_PF);
                System.out.println("SSG: Fragment shader complilation :" + infoLog);
                uninitialize_sphere();
                System.exit(0);
            }
        }

        // Shader program
        shaderProgramObject_PF = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramObject_PF, vertexShaderObject_PF);
        GLES32.glAttachShader(shaderProgramObject_PF, fragmentShaderObject_PF);
        GLES32.glBindAttribLocation(shaderProgramObject_PF, VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, "aPosition");
        GLES32.glBindAttribLocation(shaderProgramObject_PF, VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL, "aNormal");
        GLES32.glLinkProgram(shaderProgramObject_PF);

        programLinkStatus = new int[1];
        infoLogLength[0] = 0;
        infoLog = null;

        GLES32.glGetProgramiv(shaderProgramObject_PF, GLES32.GL_LINK_STATUS, programLinkStatus, 0);
        if(programLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject_PF, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetProgramInfoLog(shaderProgramObject_PF);
                System.out.println("SSG: Shader program link log :" + infoLog);
                uninitialize_sphere();
                System.exit(0);
            }
        }
        
        // Geometry
        Sphere sphere=new Sphere();
        float sphere_vertices[]=new float[1146];
        float sphere_normals[]=new float[1146];
        float sphere_textures[]=new float[764];
        short sphere_elements[]=new short[2280];
        sphere.getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        
        numVertices = sphere.getNumberOfSphereVertices();
        numElements = sphere.getNumberOfSphereElements();

        // vao
        GLES32.glGenVertexArrays(1,vao_sphere,0);
        GLES32.glBindVertexArray(vao_sphere[0]);
        
        // position vbo
        GLES32.glGenBuffers(1,vbo_sphere_position,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,vbo_sphere_position[0]);
        
        ByteBuffer byteBuffer=ByteBuffer.allocateDirect(sphere_vertices.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_vertices);
        verticesBuffer.position(0);
        
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                            sphere_vertices.length * 4,
                            verticesBuffer,
                            GLES32.GL_STATIC_DRAW);
        
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION,
                                     3,
                                     GLES32.GL_FLOAT,
                                     false,0,0);
        
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);
        
        // normal vbo
        GLES32.glGenBuffers(1,vbo_sphere_normal,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,vbo_sphere_normal[0]);
        
        byteBuffer=ByteBuffer.allocateDirect(sphere_normals.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_normals);
        verticesBuffer.position(0);
        
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                            sphere_normals.length * 4,
                            verticesBuffer,
                            GLES32.GL_STATIC_DRAW);
        
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL,
                                     3,
                                     GLES32.GL_FLOAT,
                                     false,0,0);
        
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL);
        
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);
        
        // element vbo
        GLES32.glGenBuffers(1,vbo_sphere_element,0);
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,vbo_sphere_element[0]);
        
        byteBuffer=ByteBuffer.allocateDirect(sphere_elements.length * 2);
        byteBuffer.order(ByteOrder.nativeOrder());
        ShortBuffer elementsBuffer=byteBuffer.asShortBuffer();
        elementsBuffer.put(sphere_elements);
        elementsBuffer.position(0);
        
        GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER,
                            sphere_elements.length * 2,
                            elementsBuffer,
                            GLES32.GL_STATIC_DRAW);
        
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,0);

        GLES32.glBindVertexArray(0);

        // Depth enable settings
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        // Clear color
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        Matrix.setIdentityM(perspectiveProjectionMatrix_sphere, 0);

        resize_sphere(winWidth, winHeight);

        return(true);
    }

    public boolean create_FBO(int textureWidth, int textureHeight) 
    {
        // Check the maximum render buffer size
        int[] maxRenderBufferSize = new int[1];
        GLES32.glGetIntegerv(GLES32.GL_MAX_RENDERBUFFER_SIZE, maxRenderBufferSize, 0);
        if (maxRenderBufferSize[0] < textureWidth || maxRenderBufferSize[0] < textureHeight) 
        {
            System.out.println("SSG: Texture size overflow\n" );
            return false;
        }
    
        // Create framebuffer
        int[] framebuffers = new int[1];
        GLES32.glGenFramebuffers(1, framebuffers, 0);
        FBO = framebuffers[0];
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, FBO);
    
        // Create texture for FBO
        GLES32.glGenTextures(1, texture_FBO, 0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture_FBO[0]);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_S, GLES32.GL_CLAMP_TO_EDGE);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_T, GLES32.GL_CLAMP_TO_EDGE);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexImage2D(GLES32.GL_TEXTURE_2D, 0, GLES32.GL_RGB, textureWidth, textureHeight, 0, GLES32.GL_RGB, GLES32.GL_UNSIGNED_SHORT_5_6_5, null);
    
        // Attach texture to the framebuffer at color attachment 0
        GLES32.glFramebufferTexture2D(GLES32.GL_FRAMEBUFFER, GLES32.GL_COLOR_ATTACHMENT0, GLES32.GL_TEXTURE_2D, texture_FBO[0], 0);
    
        // Create a renderbuffer for depth
        int[] renderbuffers = new int[1];
        GLES32.glGenRenderbuffers(1, renderbuffers, 0);
        RBO = renderbuffers[0];
        GLES32.glBindRenderbuffer(GLES32.GL_RENDERBUFFER, RBO);
        GLES32.glRenderbufferStorage(GLES32.GL_RENDERBUFFER, GLES32.GL_DEPTH_COMPONENT16, textureWidth, textureHeight);
    
        // Attach renderbuffer to the framebuffer for depth attachment
        GLES32.glFramebufferRenderbuffer(GLES32.GL_FRAMEBUFFER, GLES32.GL_DEPTH_ATTACHMENT, GLES32.GL_RENDERBUFFER, RBO);
    
        // Check framebuffer status
        int status = GLES32.glCheckFramebufferStatus(GLES32.GL_FRAMEBUFFER);
        if (status != GLES32.GL_FRAMEBUFFER_COMPLETE) 
        {
            System.out.println("SSG: Framebuffer status is not complete:\n" + status );
            return false;
        }
    
        // Unbind framebuffer
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);
    
        return true;
    }

    private void printGLInfo(GL10 gl)
    {
        // Code
        System.out.println("SSG: OpenGL-ES renderer :" + gl.glGetString(GL10.GL_RENDERER));
        System.out.println("SSG: OpenGL-ES Version :" + gl.glGetString(GL10.GL_VERSION));
        System.out.println("SSG: OpneGL-ES Shading Language version :" + gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION));
    }

    private void resize_cube(int width, int height)
    {
        // Code
        if(height <= 0)
            height = 1;
        
        GLES32.glViewport(0, 0, width, height);

        Matrix.perspectiveM(perspectiveProjectionMatrix_cube, 0, 45.0f, (float)width/ (float)height, 0.1f, 100.0f);
    }

    private void resize_sphere(int width, int height)
    {
        // Code
        if(height <= 0)
            height = 1;
        
        GLES32.glViewport(0, 0, width, height);

        Matrix.perspectiveM(perspectiveProjectionMatrix_sphere, 0, 45.0f, (float)width/ (float)height, 0.1f, 100.0f);
    }

    private void display_cube()
    {

        // Code
        // Render FBO Scene
        if(FBO_result == true)
        {
            display_sphere(FBO_WIDTH, FBO_HEIGHT);
            update_sphere();
        }

        // Call resize_cube
        resize_cube(winWidth, winHeight);

        // Reset bg Color to white
        GLES32.glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramObject_cube);

        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];
        float[] rotationMatrix = new float[16];
        float[] rotationMatrixX = new float[16];
        float[] rotationMatrixY = new float[16];
        float[] rotationMatrixZ = new float[16];
        
        // Cube
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.setIdentityM(rotationMatrix,0);
        Matrix.setIdentityM(rotationMatrixX,0);
        Matrix.setIdentityM(rotationMatrixY,0);
        Matrix.setIdentityM(rotationMatrixZ,0);

        Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -5.0f);    
        Matrix.setRotateM(rotationMatrixX,0, angle_cube, 1.0f, 0.0f, 0.0f);
        Matrix.setRotateM(rotationMatrixY,0, angle_cube, 0.0f, 1.0f, 0.0f);
        Matrix.setRotateM(rotationMatrixZ,0, angle_cube, 0.0f, 0.0f, 1.0f);

        Matrix.multiplyMM(rotationMatrix, 0, rotationMatrixZ, 0, rotationMatrixY, 0);
        Matrix.multiplyMM(rotationMatrix, 0, rotationMatrix, 0, rotationMatrixX, 0);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix, 0);
        
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix_cube, 0, modelViewMatrix, 0);
        GLES32.glUniformMatrix4fv(mvpMatrixUniform_cube, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture_FBO[0]);
        GLES32.glUniform1i(textureSamplerUniform_cube, 0);

        GLES32.glBindVertexArray(vao_cube[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);
        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        // Render
        requestRender();    // Swapbuffer equalent
    }

    private void display_sphere(int textureWidth, int textureHeight)
    {
        // Code
        // Bind with FBO
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, FBO);

        // Call resize_sphere()
        resize_sphere(textureWidth, textureHeight);

        // Set bg Color to Black
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        if(light_type == 'v')
        {
            GLES32.glUseProgram(shaderProgramObject_PV);

            float[] modelMatrix = new float[16];
            float[] viewMatrix = new float[16];
            float[] projectionMatrix = new float[16];

            // Get shader uniform location for Per Vertex Light
            viewMatrixUniform_sphere = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uModelMatrix");
            modelMatrixUniform_sphere = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uViewMatrix");
            projectionMatrixUniform_sphere = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uProjectionMatrix");
        
            lightDiffusedUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Diffused[0]");
            lightAmbientUniform[0] =  GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Ambient[0]");
            lightSpecularUniform[0] =  GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Specular[0]");
            lightPositionUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Position[0]");

            lightDiffusedUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Diffused[1]");
            lightAmbientUniform[1] =  GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Ambient[1]");
            lightSpecularUniform[1] =  GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Specular[1]");
            lightPositionUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Position[1]");
            
            lightDiffusedUniform[2] = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Diffused[2]");
            lightAmbientUniform[2] =  GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Ambient[2]");
            lightSpecularUniform[2] =  GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Specular[2]");
            lightPositionUniform[2] = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uLight_Position[2]");
            
            materialDiffusedUniform = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uMaterial_Diffused");
            materialAmbientUniform = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uMaterial_Ambient");
            materialSpecularUniform = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uMaterial_Specular");
            materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uMaterial_Shininess");

            tapUniform = GLES32.glGetUniformLocation(shaderProgramObject_PV, "uKeyPressed");
        
            Matrix.setIdentityM(projectionMatrix, 0);
            Matrix.setIdentityM(modelMatrix, 0);
            Matrix.setIdentityM(viewMatrix, 0);

            Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -2.0f);

            Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix_sphere, 0, projectionMatrix, 0);
            
            GLES32.glUniformMatrix4fv(modelMatrixUniform_sphere, 1, false, modelMatrix, 0);
            GLES32.glUniformMatrix4fv(viewMatrixUniform_sphere, 1, false, viewMatrix, 0);
            GLES32.glUniformMatrix4fv(projectionMatrixUniform_sphere, 1, false, projectionMatrix, 0);

            // Lights
            if(lightEnabled == true)
            {
                GLES32.glUniform1i(tapUniform, 1);

                GLES32.glUniform3fv(lightDiffusedUniform[0], 1, light_Diffused_1, 0);
                GLES32.glUniform3fv(lightAmbientUniform[0], 1, light_Ambient_1, 0);
                GLES32.glUniform3fv(lightSpecularUniform[0], 1, light_Specular_1, 0);
                GLES32.glUniform4fv(lightPositionUniform[0], 1, light_Position_1, 0);

                GLES32.glUniform3fv(lightDiffusedUniform[1], 1, light_Diffused_2, 0);
                GLES32.glUniform3fv(lightAmbientUniform[1], 1, light_Ambient_2, 0);
                GLES32.glUniform3fv(lightSpecularUniform[1], 1, light_Specular_2, 0);
                GLES32.glUniform4fv(lightPositionUniform[1], 1, light_Position_2, 0);

                GLES32.glUniform3fv(lightDiffusedUniform[2], 1, light_Diffused_3, 0);
                GLES32.glUniform3fv(lightAmbientUniform[2], 1, light_Ambient_3, 0);
                GLES32.glUniform3fv(lightSpecularUniform[2], 1, light_Specular_3, 0);
                GLES32.glUniform4fv(lightPositionUniform[2], 1, light_Position_3, 0);

                GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused, 0);
                GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient, 0);
                GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular, 0);
                GLES32.glUniform1f(materialShininessUniform, materialShininess);
            }
            else
            {
                GLES32.glUniform1i(tapUniform, 0);
            }

            // Bind VAO
            GLES32.glBindVertexArray(vao_sphere[0]);
            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
            GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
            
            // Unbind VAO
            GLES32.glBindVertexArray(0);
            GLES32.glUseProgram(0);
        }

        if(light_type == 'f')
        {
            GLES32.glUseProgram(shaderProgramObject_PF);

            float[] modelMatrix = new float[16];
            float[] viewMatrix = new float[16];
            float[] projectionMatrix = new float[16];

            // Get Shader Uniform Locations 
            viewMatrixUniform_sphere = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uModelMatrix");
            modelMatrixUniform_sphere = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uViewMatrix");
            projectionMatrixUniform_sphere = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uProjectionMatrix");
        
            lightDiffusedUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Diffused[0]");
            lightAmbientUniform[0] =  GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Ambient[0]");
            lightSpecularUniform[0] =  GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Specular[0]");
            lightPositionUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Position[0]");

            lightDiffusedUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Diffused[1]");
            lightAmbientUniform[1] =  GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Ambient[1]");
            lightSpecularUniform[1] =  GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Specular[1]");
            lightPositionUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Position[1]");
            
            lightDiffusedUniform[2] = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Diffused[2]");
            lightAmbientUniform[2] =  GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Ambient[2]");
            lightSpecularUniform[2] =  GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Specular[2]");
            lightPositionUniform[2] = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uLight_Position[2]");
            
            materialDiffusedUniform = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uMaterial_Diffused");
            materialAmbientUniform = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uMaterial_Ambient");
            materialSpecularUniform = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uMaterial_Specular");
            materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uMaterial_Shininess");
            
            tapUniform = GLES32.glGetUniformLocation(shaderProgramObject_PF, "uKeyPressed");
    
            Matrix.setIdentityM(projectionMatrix, 0);
            Matrix.setIdentityM(modelMatrix, 0);
            Matrix.setIdentityM(viewMatrix, 0);
    
            Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -2.0f);
    
            Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix_sphere, 0, projectionMatrix, 0);
            
            GLES32.glUniformMatrix4fv(modelMatrixUniform_sphere, 1, false, modelMatrix, 0);
            GLES32.glUniformMatrix4fv(viewMatrixUniform_sphere, 1, false, viewMatrix, 0);
            GLES32.glUniformMatrix4fv(projectionMatrixUniform_sphere, 1, false, projectionMatrix, 0);
    
            // Lights
            if(lightEnabled == true)
            {
                GLES32.glUniform1i(tapUniform, 1);
                
                GLES32.glUniform3fv(lightDiffusedUniform[0], 1, light_Diffused_1, 0);
                GLES32.glUniform3fv(lightAmbientUniform[0], 1, light_Ambient_1, 0);
                GLES32.glUniform3fv(lightSpecularUniform[0], 1, light_Specular_1, 0);
                GLES32.glUniform4fv(lightPositionUniform[0], 1, light_Position_1, 0);

                GLES32.glUniform3fv(lightDiffusedUniform[1], 1, light_Diffused_2, 0);
                GLES32.glUniform3fv(lightAmbientUniform[1], 1, light_Ambient_2, 0);
                GLES32.glUniform3fv(lightSpecularUniform[1], 1, light_Specular_2, 0);
                GLES32.glUniform4fv(lightPositionUniform[1], 1, light_Position_2, 0);

                GLES32.glUniform3fv(lightDiffusedUniform[2], 1, light_Diffused_3, 0);
                GLES32.glUniform3fv(lightAmbientUniform[2], 1, light_Ambient_3, 0);
                GLES32.glUniform3fv(lightSpecularUniform[2], 1, light_Specular_3, 0);
                GLES32.glUniform4fv(lightPositionUniform[2], 1, light_Position_3, 0);
    
                GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused, 0);
                GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient, 0);
                GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular, 0);
                GLES32.glUniform1f(materialShininessUniform, materialShininess);
            }
            else
            {
                GLES32.glUniform1i(tapUniform, 0);
            }
    
            // Bind VAO
            GLES32.glBindVertexArray(vao_sphere[0]);
            GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
            GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
            
            // Unbind VAO
            GLES32.glBindVertexArray(0);
            GLES32.glUseProgram(0);
        }

        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);
    }
    
    private void update_cube()
    {
        angle_cube = angle_cube + 0.1f;
        if(angle_cube >= 360.0f)
            angle_cube = angle_cube - 360.0f;
    }
    
    private void update_sphere()
    {
        // Code
        if(lightEnabled == true && light_type == 'v' || light_type == 'f')
        {
            // Red Light
            light_Position_1[0] = 0.0f;
            light_Position_1[1] = 3.0f * (float)Math.cos(angle_light);
            light_Position_1[2] = 4.0f * (float)Math.sin(angle_light);
            light_Position_1[3] = 1.0f;
            
            angle_light = angle_light + 0.005f;
            if(angle_light > 360.0f)
            {
                angle_light = angle_light - 360.0f;
            }

            // Green Light
            light_Position_2[0] = 5.0f * (float)Math.sin(angle_light);
            light_Position_2[1] = 0.0f;
            light_Position_2[2] = 3.0f * (float)Math.cos(angle_light);
            light_Position_2[3] = 1.0f;
            
            angle_light = angle_light + 0.005f;
            if(angle_light > 360.0f)
            {
                angle_light = angle_light - 360.0f;
            }

            // Blue Light
            light_Position_3[0] = 3.0f * (float)Math.cos(angle_light);
            light_Position_3[1] = 6.0f * (float)Math.sin(angle_light);
            light_Position_3[2] = 0.0f;
            light_Position_3[3] = 1.0f;
            
            angle_light = angle_light + 0.005f;
            if(angle_light > 360.0f)
            {
                angle_light = angle_light - 360.0f;
            }
        }
    }

    private void uninitialize_cube()
    {
        // Code
        if(shaderProgramObject_cube > 0)
        {
            GLES32.glUseProgram(shaderProgramObject_cube);

            int[] retVal = new int[1];

            GLES32.glGetProgramiv(shaderProgramObject_cube, GLES32.GL_ATTACHED_SHADERS, retVal, 0);
            if(retVal[0] > 0)
            {
                int numAttachedShaders = retVal[0];
                int[] shaderObjects = new int[numAttachedShaders];
                GLES32.glGetAttachedShaders(shaderProgramObject_cube, numAttachedShaders, retVal, 0, shaderObjects, 0);
                
                for(int i = 0; i < numAttachedShaders; i++)
                {
                    GLES32.glDetachShader(shaderProgramObject_cube, shaderObjects[i]);
                    GLES32.glDeleteShader(shaderObjects[i]);
                    shaderObjects[i] = 0;
                }
            }

            GLES32.glUseProgram(0);
            GLES32.glDeleteProgram(shaderProgramObject_cube);
            shaderProgramObject_cube = 0;

        }
        
        if(vbo_cube_texcoord[0] > 0)
        {
            GLES32.glDeleteBuffers(1, vbo_cube_texcoord, 0);
            vbo_cube_texcoord[0] = 0;
        }

        if(vbo_cube_position[0] > 0)
        {
            GLES32.glDeleteBuffers(1, vbo_cube_position, 0);
            vbo_cube_position[0] = 0;
        }

        if(vao_cube[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_cube, 0);
            vao_cube[0] = 0;
        }

        if(texture_FBO[0] > 0) 
        {
            GLES32.glDeleteTextures(1, texture_FBO, 0);
            texture_FBO[0] = 0;
        }

        if (RBO > 0) 
        {
            GLES32.glDeleteRenderbuffers(1, new int[]{RBO}, 0);
            RBO = 0;
        }

        if (FBO > 0) 
        {
            GLES32.glDeleteFramebuffers(1, new int[]{FBO}, 0);
            FBO = 0;
        }
    }

    private void uninitialize_sphere()
    {
        // Code
        if(light_type == 'v')
        {
            if(shaderProgramObject_PV > 0)
            {
                GLES32.glUseProgram(shaderProgramObject_PV);

                int[] retVal = new int[1];

                GLES32.glGetProgramiv(shaderProgramObject_PV, GLES32.GL_ATTACHED_SHADERS, retVal, 0);
                if(retVal[0] > 0)
                {
                    int numAttachedShaders = retVal[0];
                    int[] shaderObjects = new int[numAttachedShaders];
                    GLES32.glGetAttachedShaders(shaderProgramObject_PV, numAttachedShaders, retVal, 0, shaderObjects, 0);
                    
                    for(int i = 0; i < numAttachedShaders; i++)
                    {
                        GLES32.glDetachShader(shaderProgramObject_PV, shaderObjects[i]);
                        GLES32.glDeleteShader(shaderObjects[i]);
                        shaderObjects[i] = 0;
                    }
                }

                GLES32.glUseProgram(0);
                GLES32.glDeleteProgram(shaderProgramObject_PV);
                shaderProgramObject_PV = 0;
            }
        }

        if(light_type == 'f')
        {
            if(shaderProgramObject_PF > 0)
            {
                GLES32.glUseProgram(shaderProgramObject_PF);

                int[] retVal = new int[1];

                GLES32.glGetProgramiv(shaderProgramObject_PF, GLES32.GL_ATTACHED_SHADERS, retVal, 0);
                if(retVal[0] > 0)
                {
                    int numAttachedShaders = retVal[0];
                    int[] shaderObjects = new int[numAttachedShaders];
                    GLES32.glGetAttachedShaders(shaderProgramObject_PF, numAttachedShaders, retVal, 0, shaderObjects, 0);
                    
                    for(int i = 0; i < numAttachedShaders; i++)
                    {
                        GLES32.glDetachShader(shaderProgramObject_PF, shaderObjects[i]);
                        GLES32.glDeleteShader(shaderObjects[i]);
                        shaderObjects[i] = 0;
                    }
                }
                
                GLES32.glUseProgram(0);
                GLES32.glDeleteProgram(shaderProgramObject_PF);
                shaderProgramObject_PF = 0;
            }
        }

        // Delete VAO
        if(vao_sphere[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_sphere, 0);
            vao_sphere[0]=0;
        }
        
        // Delete position VBO
        if(vbo_sphere_position[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_position, 0);
            vbo_sphere_position[0]=0;
        }
        
        // Delete normal VBO
        if(vbo_sphere_normal[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_normal, 0);
            vbo_sphere_normal[0]=0;
        }
        
        // Delete element VBO
        if(vbo_sphere_element[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_element, 0);
            vbo_sphere_element[0]=0;
        }
    }
}

