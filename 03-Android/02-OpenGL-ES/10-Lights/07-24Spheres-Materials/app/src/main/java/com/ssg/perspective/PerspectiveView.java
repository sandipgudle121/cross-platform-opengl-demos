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
import android.opengl.Matrix;

// Event related packages
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnDoubleTapListener;
import android.view.GestureDetector.OnGestureListener;

// Sphere related packages
import java.nio.ShortBuffer;

public class PerspectiveView extends GLSurfaceView implements GLSurfaceView.Renderer, OnDoubleTapListener, OnGestureListener
{
    // Class variables
    private GestureDetector gesture_detector;
    
    private int shaderProgramObject;

    private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];

    private float[] light_Diffused = { 1.0f, 1.0f, 1.0f, 1.0f };
    private float[] light_Ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
    private float[] light_Specular = { 1.0f,1.0f, 1.0f, 1.0f };
    private float[] light_Position = { 0.0f, 0.0f, 0.0f, 1.0f };

    private int numVertices;
    private int numElements;

    private static final int AXIS_X = 0;
    private static final int AXIS_Y = 1;
    private static final int AXIS_Z = 2;

    // Track the current rotation axis
    private int currentAxis = AXIS_X;  // Start with rotation around the X-axis

    private float angle = 0.0f;
    private float SPEED = 0.05f;
    private boolean lightEnabled = false;
    private boolean animationEnabled = false;
    private int tapUniform;

    private int lightDiffusedUniform;
    private int lightAmbientUniform;
    private int lightSpecularUniform;
    private int lightPositionUniform;

    private int materialDiffusedUniform;
    private int materialAmbientUniform;
    private int materialSpecularUniform;
    private int materialShininessUniform;

    private int viewMatrixUniform;
    private int modelMatrixUniform;
    private int projectionMatrixUniform;
    
    private float[] perspectiveProjectionMatrix = new float[16]; 

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
        initialize(gl);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
        // Code
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl)
    {
        // Code 
        display();

        if(animationEnabled == true)
        {
            update();
        }
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
        return(true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
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
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        if(lightEnabled == true)
        {
            currentAxis = (currentAxis + 1) % 3;
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
        if(animationEnabled == false)
        {
            animationEnabled = true;
        }
        else
        {
            animationEnabled = false;
        }
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
    {
        // Code
        uninitialize();

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
    private void initialize(GL10 gl)
    {
        // Code
        // Print OpenGL ES Info
        printGLInfo(gl);

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
            "uniform vec4 uLight_Position;\n"+
            "\n"+
            "uniform int uKeyPressed;\n"+
            "out vec3 oTransformed_Normals;\n"+
            "out vec3 oLight_Direction;\n"+
            "out vec3 oViewer_Vector;\n"+
            "void main(void)\n"+
            "{\n"+
            "if(uKeyPressed == 1)\n"+
            "{\n"+
            "vec4 eye_Coordinates = uViewMatrix * uModelMatrix * aPosition;\n"+
            "oTransformed_Normals = mat3(uViewMatrix * uModelMatrix) * aNormal;\n"+
            "oLight_Direction = vec3(uLight_Position - eye_Coordinates);\n"+
            "oViewer_Vector = -eye_Coordinates.xyz;\n"+
            "}\n"+
            "else\n"+
            "{\n"+
            "oTransformed_Normals = vec3(0.0f, 0.0f, 0.0);\n"+
            "oLight_Direction = vec3(0.0f, 0.0f, 0.0);\n"+
            "oViewer_Vector = vec3(0.0f, 0.0f, 0.0);\n"+
            "}\n"+
            "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"+
            "}\n"                                                   
        );
        
        int vertexShaderObject_PF = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        GLES32.glShaderSource(vertexShaderObject_PF, vertexShaderSourceCode_PF);
        GLES32.glCompileShader(vertexShaderObject_PF);

        int[] shaderCompileStatus = new int[1];
        int[] infoLogLength = new int[1];
        String infoLog = null;

        GLES32.glGetShaderiv(vertexShaderObject_PF, GLES32.GL_COMPILE_STATUS, shaderCompileStatus, 0);
        if(shaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject_PF, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetShaderInfoLog(vertexShaderObject_PF);
                System.out.println("SSG: Vertex shader complilation :" + infoLog);
                uninitialize();
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
            "in vec3 oLight_Direction;\n"+
            "in vec3 oViewer_Vector;\n"+
            "\n"+
            "uniform vec3 uLight_Ambient;\n"+
            "uniform vec3 uLight_Specular;\n"+
            "uniform vec3 uLight_Diffused;\n"+
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
            "vec3 normalized_Light_Direction;\n"+
            "vec3 lightDiffused;\n"+
            "vec3 lightAmbient;\n"+
            "vec3 reflection_Vector;\n"+
            "vec3 lightSpecular;\n"+
            "vec3 normalized_Transformed_Normals = normalize(oTransformed_Normals);\n"+
            "normalized_Light_Direction = normalize(oLight_Direction);\n"+
            "vec3 normalized_Viewer_Vector = normalize(oViewer_Vector);\n"+
            "\n"+
            "lightAmbient = uLight_Ambient * uMaterial_Ambient;\n"+
            "lightDiffused = uLight_Diffused * uMaterial_Diffused * max(dot(normalized_Light_Direction, normalized_Transformed_Normals), 0.0);\n"+
            "reflection_Vector = reflect(-normalized_Light_Direction, normalized_Transformed_Normals);\n"+
            "lightSpecular = uLight_Specular* uMaterial_Specular * pow(max(dot(reflection_Vector, normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
            "phong_ADS_Light = phong_ADS_Light + lightAmbient + lightDiffused + lightSpecular;\n"+
            "}\n"+
            "else\n"+
            "{\n"+
            "phong_ADS_Light = vec3(1.0f, 1.0f, 1.0);\n"+
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
                uninitialize();
                System.exit(0);
            }
        }

        // Shader program
        shaderProgramObject = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramObject, vertexShaderObject_PF);
        GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject_PF);
        GLES32.glBindAttribLocation(shaderProgramObject, VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, "aPosition");
        GLES32.glBindAttribLocation(shaderProgramObject, VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL, "aNormal");
        GLES32.glLinkProgram(shaderProgramObject);

        int[] programLinkStatus = new int[1];
        infoLogLength[0] = 0;
        infoLog = null;

        GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, programLinkStatus, 0);
        if(programLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetProgramInfoLog(shaderProgramObject);
                System.out.println("SSG: Shader program link log :" + infoLog);
                uninitialize();
                System.exit(0);
            }
        }

        // Get Shader Uniform Locations 
        viewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uModelMatrix");
        modelMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uViewMatrix");
        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
    
        lightDiffusedUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Diffused");
        lightAmbientUniform =  GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Ambient");
        lightSpecularUniform =  GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Specular");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Position");
        
        materialDiffusedUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterial_Diffused");
        materialAmbientUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterial_Ambient");
        materialSpecularUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterial_Specular");
        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterial_Shininess");
        tapUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKeyPressed");

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
        
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,sphere_vertices.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
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
        
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, sphere_normals.length * 4, verticesBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
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
        
        GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER, sphere_elements.length * 2,elementsBuffer, GLES32.GL_STATIC_DRAW);      
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,0);
        GLES32.glBindVertexArray(0);

        // Depth enable settings
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        // Clear color
        GLES32.glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
    }

    private void printGLInfo(GL10 gl)
    {
        // Code
        System.out.println("SSG: OpenGL-ES renderer :" + gl.glGetString(GL10.GL_RENDERER));
        System.out.println("SSG: OpenGL-ES Version :" + gl.glGetString(GL10.GL_VERSION));
        System.out.println("SSG: OpneGL-ES Shading Language version :" + gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION));
    }

    private void resize(int width, int height)
    {
        // Code
        if(height <= 0)
            height = 1;
        
        GLES32.glViewport(0, 0, width, height);
        float aspectRatio = (float)width / (float)height;
        Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f, aspectRatio, 0.1f, 100.0f);
    }

    private void display()
    {
        // local variables
        int width = getWidth();
        int height = getHeight();

        // Calculate the width and height of each sub-viewport
        int subViewportWidth = width / 6;  // 6 columns
        int subViewportHeight = height / 4;  // 4 rows

        float[] materialDiffused_1 = { 0.7568f, 0.61424f, 0.07568f, 1.0f };   
        float[] materialAmbient_1 = { 0.0215f, 0.1745f, 0.0215f, 1.0f };
        float[] materialSpecular_1 = { 0.633f, 0.727811f, 0.633f, 1.0f };
        float materialShininess_1 = 0.06f * 128.0f;

        float[] materialDiffused_2 = { 0.54f, 0.89f, 0.63f, 1.0f };   
        float[] materialAmbient_2 = { 0.135f, 0.2225f, 0.1573f, 1.0f };
        float[] materialSpecular_2 = { 0.316228f, 0.316228f, 0.316228f, 1.0f };
        float materialShininess_2 = 0.1f * 128.0f;

        float[] materialDiffused_3 = { 0.18275f, 0.17f, 0.22525f, 1.0f };   
        float[] materialAmbient_3 = { 0.05375f, 0.05f, 0.06625f, 1.0f };
        float[] materialSpecular_3 = { 0.332741f, 0.328634f, 0.346435f, 1.0f };
        float materialShininess_3 = 0.3f * 128.0f;

        float[] materialDiffused_4 = { 1.0f, 0.829f, 0.829f, 1.0f };   
        float[] materialAmbient_4 = { 0.25f, 0.20725f, 0.20725f, 1.0f };
        float[] materialSpecular_4 = { 0.296648f, 0.296648f, 0.296648f, 1.0f };
        float materialShininess_4 = 0.088f * 128.0f;

        float[] materialDiffused_5 = { 0.61424f, 0.04136f, 0.04136f, 1.0f };   
        float[] materialAmbient_5 = { 0.1745f, 0.01175f, 0.01175f, 1.0f };
        float[] materialSpecular_5 = { 0.727811f, 0.626959f, 0.626959f, 1.0f };
        float materialShininess_5 = 0.6f * 128.0f;

        float[] materialDiffused_6 = { 0.396f, 0.74151f, 0.69102f, 1.0f };   
        float[] materialAmbient_6 = { 0.1f, 0.18725f, 0.1745f, 1.0f };
        float[] materialSpecular_6 = { 0.297554f, 0.30829f, 0.306668f, 1.0f };
        float materialShininess_6 = 0.1f * 128.0f;

        float[] materialDiffused_7 = { 0.780392f, 0.568627f, 0.113725f, 1.0f };   
        float[] materialAmbient_7 = { 0.329412f, 0.223529f, 0.027451f, 1.0f };
        float[] materialSpecular_7 = { 0.992157f, 0.941176f, 0.807843f, 1.0f };
        float materialShininess_7 = 0.21794872f * 128.0f;

        float[] materialDiffused_8 = { 0.714f, 0.4284f, 0.18144f, 1.0f };   
        float[] materialAmbient_8 = { 0.2125f, 0.1275f, 0.054f, 1.0f };
        float[] materialSpecular_8 = { 0.393548f, 0.271906f, 0.166721f, 1.0f };
        float materialShininess_8 = 0.2f * 128.0f;

        float[] materialDiffused_9 = { 0.4f, 0.4f, 0.4f, 1.0f };   
        float[] materialAmbient_9 = { 0.25f, 0.25f, 0.25f, 1.0f };
        float[] materialSpecular_9 = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
        float materialShininess_9 = 0.6f * 128.0f;

        float[] materialDiffused_10 = { 0.7038f, 0.27048f, 0.0828f, 1.0f };   
        float[] materialAmbient_10 = { 0.19125f, 0.0735f, 0.0225f, 1.0f };
        float[] materialSpecular_10 = { 0.256777f, 0.137622f, 0.086014f, 1.0f };
        float materialShininess_10 = 0.1f * 128.0f;

        float[] materialDiffused_11 = { 0.75164f, 0.60648f, 0.22648f, 1.0f };   
        float[] materialAmbient_11 = { 0.24725f, 0.1995f, 0.0745f, 1.0f };
        float[] materialSpecular_11 = { 0.608281f, 0.555802f, 0.366065f, 1.0f };
        float materialShininess_11 = 0.4f * 128.0f;

        float[] materialDiffused_12 = { 0.50754f, 0.50754f, 0.50754f, 1.0f };   
        float[] materialAmbient_12 = { 0.19225f, 0.19225f, 0.19225f, 1.0f };
        float[] materialSpecular_12 = { 0.508273f, 0.508273f, 0.508273f, 1.0f };
        float materialShininess_12 =  0.4f * 128.0f;

        float[] materialDiffused_13 = { 0.01f, 0.01f, 0.01f, 1.0f };   
        float[] materialAmbient_13 = { 0.0f, 0.0f, 0.0f, 1.0f };
        float[] materialSpecular_13 = { 0.50f, 0.50f, 0.50f, 1.0f };
        float materialShininess_13 = 0.25f * 128.0f;

        float[] materialDiffused_14 = { 0.0f, 0.50980392f, 0.50980392f, 1.0f };   
        float[] materialAmbient_14 = { 0.0f, 0.1f, 0.06f, 1.0f };
        float[] materialSpecular_14 = { 0.50196078f, 0.50196078f, 0.50196078f, 1.0f };
        float materialShininess_14 = 0.25f * 128.0f;

        float[] materialDiffused_15 = { 0.1f, 0.35f, 0.1f, 1.0f };   
        float[] materialAmbient_15 = { 0.0f, 0.0f, 0.0f, 1.0f };
        float[] materialSpecular_15 = { 0.45f, 0.45f, 0.45f, 1.0f };
        float materialShininess_15 = 0.25f * 128.0f;

        float[] materialDiffused_16 = { 0.5f, 0.0f, 0.0f, 1.0f };   
        float[] materialAmbient_16 = { 0.0f, 0.0f, 0.0f, 1.0f };
        float[] materialSpecular_16 = { 0.7f, 0.6f, 0.6f, 1.0f };
        float materialShininess_16 = 0.25f * 128.0f;

        float[] materialDiffused_17 = { 0.55f, 0.55f, 0.55f, 1.0f };   
        float[] materialAmbient_17 = { 0.0f, 0.0f, 0.0f, 1.0f };
        float[] materialSpecular_17 = { 0.70f, 0.70f, 0.70f, 1.0f };
        float materialShininess_17 = 0.25f * 128.0f;

        float[] materialDiffused_18 = { 0.5f, 0.5f, 0.0f, 1.0f };   
        float[] materialAmbient_18 = { 0.0f, 0.0f, 0.0f, 1.0f };
        float[] materialSpecular_18 = { 0.60f, 0.60f, 0.50f, 1.0f };
        float materialShininess_18 = 0.25f * 128.0f;

        float[] materialDiffused_19 = { 0.01f, 0.01f, 0.01f, 1.0f };   
        float[] materialAmbient_19 = { 0.02f, 0.02f, 0.02f, 1.0f };
        float[] materialSpecular_19 = { 0.4f, 0.4f, 0.4f, 1.0f };
        float materialShininess_19 = 0.078125f * 128.0f;

        float[] materialDiffused_20 = { 0.4f, 0.5f, 0.5f, 1.0f };   
        float[] materialAmbient_20 = { 0.0f, 0.05f, 0.05f, 1.0f };
        float[] materialSpecular_20 = { 0.04f, 0.7f, 0.7f, 1.0f };
        float materialShininess_20 = 0.078125f * 128;

        float[] materialDiffused_21 = { 0.4f, 0.5f, 0.4f, 1.0f };   
        float[] materialAmbient_21 = { 0.0f, 0.05f, 0.0f, 1.0f };
        float[] materialSpecular_21 = { 0.04f, 0.7f, 0.04f, 1.0f };
        float materialShininess_21 = 0.078125f * 128.0f;

        float[] materialDiffused_22 = { 0.5f, 0.4f, 0.4f, 1.0f };   
        float[] materialAmbient_22 = { 0.02f, 0.0f, 0.0f, 1.0f };
        float[] materialSpecular_22 = { 0.7f, 0.04f, 0.04f, 1.0f };
        float materialShininess_22 = 0.078125f * 128.0f;

        float[] materialDiffused_23 = { 0.5f, 0.5f, 0.5f, 1.0f };   
        float[] materialAmbient_23 = { 0.05f, 0.05f, 0.05f, 1.0f };
        float[] materialSpecular_23 = { 0.7f, 0.7f, 0.7f, 1.0f };
        float materialShininess_23 = 0.078125f * 128.0f;

        float[] materialDiffused_24 = { 0.5f, 0.5f, 0.4f, 1.0f };   
        float[] materialAmbient_24 = { 0.05f, 0.05f, 0.0f, 1.0f };
        float[] materialSpecular_24 = { 0.7f, 0.7f, 0.04f, 1.0f };
        float materialShininess_24 = 0.078125f * 128.0f;

        // Code
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        // Sphere 1
        GLES32.glUseProgram(shaderProgramObject);

        // Row 1 (from top to bottom)
        GLES32.glViewport(0, 50 +  3 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        float[] modelMatrix = new float[16];
        float[] viewMatrix = new float[16];
        float[] projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_1, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_1, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_1, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_1);
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

        // Sphere 2
        GLES32.glUseProgram(shaderProgramObject);

        // Row 1 (from top to bottom)
        GLES32.glViewport(subViewportWidth, 50 +  3 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_2, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_2, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_2, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_2);
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

        // Sphere 3
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(2 * subViewportWidth, 50 +  3 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_3, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_3, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_3, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_3);
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

        // Sphere 4
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(3 * subViewportWidth, 50 +  3 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_4, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_4, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_4, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_4);
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
        

        // Sphere 5
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(4 * subViewportWidth, 50 + 3 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_5, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_5, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_5, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_5);
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

        // Sphere 6
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(5 * subViewportWidth, 50 + 3 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_6, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_6, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_6, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_6);
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


        // Sphere 7
        GLES32.glUseProgram(shaderProgramObject);

        // Row 2
        GLES32.glViewport(0, 50 + 2 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_7, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_7, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_7, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_7);
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


        // Sphere 8
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(subViewportWidth, 50 + 2 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_8, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_8, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_8, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_8);
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


        // Sphere 9
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(2 * subViewportWidth, 50 + 2 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_9, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_9, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_9, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_9);
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


        // Sphere 10
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(3 * subViewportWidth, 50 + 2 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_10, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_10, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_10, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_10);
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


        // Sphere 11
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(4 * subViewportWidth, 50 + 2 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_11, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_11, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_11, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_11);
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


        // Sphere 12
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(5 * subViewportWidth, 50 + 2 * subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_12, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_12, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_12, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_12);
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


        // Sphere 13
        GLES32.glUseProgram(shaderProgramObject);

        // Row 3
        GLES32.glViewport(0, 50 + subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_13, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_13, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_13, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_13);
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


        // Sphere 14
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(subViewportWidth, 50 + subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_14, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_14, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_14, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_14);
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


        // Sphere 15
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(2 * subViewportWidth, 50 + subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_15, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_15, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_15, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_15);
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


        // Sphere 16
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(3 * subViewportWidth, 50 + subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_16, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_16, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_16, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_16);
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


        // Sphere 17
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(4 * subViewportWidth, 50 + subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_17, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_17, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_17, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_17);
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


        // Sphere 18
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(5 * subViewportWidth, 50 + subViewportHeight, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_18, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_18, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_18, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_18);
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


        // Sphere 19
        GLES32.glUseProgram(shaderProgramObject);

        // Row 4
        GLES32.glViewport(0, 50 + 0, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_19, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_19, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_19, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_19);
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


        // Sphere 20
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(subViewportWidth, 50 + 0, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_20, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_20, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_20, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_20);
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


        // Sphere 21
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(2 * subViewportWidth, 50 + 0, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_21, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_21, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_21, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_21);
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


        // Sphere 22
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(3 * subViewportWidth, 50 + 0, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_22, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_22, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_22, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_22);
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


        // Sphere 23
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(4 * subViewportWidth, 50 + 0, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_23, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_23, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_23, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_23);
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


        // Sphere 24
        GLES32.glUseProgram(shaderProgramObject);
        GLES32.glViewport(5 * subViewportWidth, 50 + 0, subViewportWidth, (int)(subViewportHeight / 1.5f));

        modelMatrix = new float[16];
        viewMatrix = new float[16];
        projectionMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -1.5f);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightDiffusedUniform, 1, light_Diffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, light_Ambient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, light_Specular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, light_Position, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused_24, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient_24, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular_24, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess_24);
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



        // Render
        requestRender();    // Swapbuffer equalent
    }

    private void update()
    {
        if(lightEnabled) 
        {
            switch(currentAxis)
            {
                case AXIS_X:
                    light_Position[0] = 4.0f * (float) Math.sin(angle);
                    light_Position[1] = 0.0f;
                    light_Position[2] = 3.0f * (float) Math.cos(angle);
                    break;

                case AXIS_Y:
                    light_Position[0] = 0.0f;
                    light_Position[1] = 3.0f * (float) Math.cos(angle);
                    light_Position[2] = 4.0f * (float) Math.sin(angle);
                    break;

                case AXIS_Z:
                    light_Position[0] = 3.0f * (float) Math.cos(angle);
                    light_Position[1] = 4.0f * (float) Math.sin(angle);
                    light_Position[2] = 0.0f;
                    break;
            }

            light_Position[3] = 1.0f;
    
            angle += SPEED;
            if (angle > 360.0f) {
                angle -= 360.0f;
            }
        }
    }

    private void uninitialize()
    {
        // Code
        if(shaderProgramObject > 0)
        {
            GLES32.glUseProgram(shaderProgramObject);

            int[] retVal = new int[1];

            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_ATTACHED_SHADERS, retVal, 0);
            if(retVal[0] > 0)
            {
                int numAttachedShaders = retVal[0];
                int[] shaderObjects = new int[numAttachedShaders];
                GLES32.glGetAttachedShaders(shaderProgramObject, numAttachedShaders, retVal, 0, shaderObjects, 0);
                
                for(int i = 0; i < numAttachedShaders; i++)
                {
                    GLES32.glDetachShader(shaderProgramObject, shaderObjects[i]);
                    GLES32.glDeleteShader(shaderObjects[i]);
                    shaderObjects[i] = 0;
                }
            }

            GLES32.glUseProgram(0);
            GLES32.glDeleteProgram(shaderProgramObject);
            shaderProgramObject = 0;
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
