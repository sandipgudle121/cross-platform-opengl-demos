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

public class PerspectiveView extends GLSurfaceView implements GLSurfaceView.Renderer, OnDoubleTapListener, OnGestureListener
{
    // Class variables
    private GestureDetector gesture_detector;
    
    private int shaderProgramObject;
    private int[] vao_pyramid = new int[1];
    private int[] vbo_pyramid = new int[1];
    private int[] vbo_normal = new int[1];

    private float[] lightDiffused_0 = { 1.0f, 0.0f, 0.0f, 1.0f};
    private float[] lightAmbient_0 = { 0.0f, 0.0f, 0.0f, 1.0f};
    private float[] lightSpecular_0 = { 1.0f, 0.0f, 0.0f, 1.0f};
    private float[] lightPosition_0 = { -2.0f, 0.0f, 0.0f, 1.0f};

    private float[] lightDiffused_1 = { 0.0f, 0.0f, 1.0f, 1.0f};
    private float[] lightAmbient_1 = { 0.0f, 0.0f, 0.0f, 1.0f};
    private float[] lightSpecular_1 = { 0.0f, 0.0f, 1.0f, 1.0f};
    private float[] lightPosition_1 = { 2.0f, 0.0f, 0.0f, 1.0f};

    private float[] materialDiffused = {1.0f, 1.0f, 1.0f, 1.0f};
    private float[] materialAmbient = {0.0f, 0.0f, 0.0f, 1.0f};
    private float[] materialSpecular = {1.0f, 1.0f, 1.0f, 1.0f};
    private float materialShininess = 128.0f;

    private float angle = 0.0f;
    private boolean lightEnabled = false;
    private boolean animationEnabled = false;
    private int tapUniform;

    private int[] lightDiffusedUniform = new int[2];
    private int[] lightAmbientUniform = new int[2];
    private int[] lightSpecularUniform = new int[2];
    private int[] lightPositionUniform = new int[2];

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
        // Display
        display();

        // Update
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
        if(animationEnabled == false)
        {
            animationEnabled = true;
        }
        else
        {
            animationEnabled = false;
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
        final String vertexShaderSourceCode = String.format
        (   
            "#version 320 es\n"+
            "\n"+
            "precision highp float;"+
            "precision highp int;"+
            "in vec4 aPosition;\n"+
            "in vec3 aNormal;\n"+
            "\n"+
            "uniform mat4 uViewMatrix;\n"+
            "uniform mat4 uModelMatrix;\n"+
            "uniform mat4 uProjectionMatrix;\n"+
            "\n"+
            "uniform vec3 uLight_Ambient[2];\n"+
            "uniform vec3 uLight_Specular[2];\n"+
            "uniform vec3 uLight_Diffused[2];\n"+
            "uniform vec4 uLight_Position[2];\n"+
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
            "vec3 lightAmbient[2];\n"+
            "vec3 lightDiffused[2];\n"+
            "vec3 lightSpecular[2];\n"+
            "vec3 reflection_Vector[2];\n"+
            "vec3 light_Direction[2];\n"+
            "for( int i = 0; i < 2; i++ )\n"+
            "{\n"+
            "lightAmbient[i] = uLight_Ambient[i] * uMaterial_Ambient;\n"+
            "light_Direction[i] = normalize(vec3(uLight_Position[i] - eye_Coordinates));\n"+
            "lightDiffused[i] = uLight_Diffused[i] * uMaterial_Diffused * max(dot(light_Direction[i], transformed_Normals), 0.0);\n"+
            "reflection_Vector[i] = reflect(-light_Direction[i], transformed_Normals);\n"+
            "vec3 viewer_Vector = normalize(-eye_Coordinates.xyz);\n"+
            "lightSpecular[i] = uLight_Specular[i] * uMaterial_Specular * pow(max(dot(reflection_Vector[i], viewer_Vector), 0.0), uMaterial_Shininess);\n"+
            "oPhong_ADS_Light = oPhong_ADS_Light + lightAmbient[i] + lightDiffused[i] + lightSpecular[i];\n"+
            "}\n"+
            "}\n"+
            "else\n"+
            "{\n"+
            "oPhong_ADS_Light = vec3(0.0f, 0.0f, 0.0f);\n"+
            "}\n"+
            "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"+
            "}\n"
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
                uninitialize();
                System.exit(0);
            }
        }

        // Fragment shader
        final String fragmentShaderSourceCode = String.format
        (   
            "#version 320 es\n"+
            "\n"+
            "precision highp float;"+
            "precision highp int;"+
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
                uninitialize();
                System.exit(0);
            }
        }

        // Shader program
        shaderProgramObject = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);
        GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);
        GLES32.glBindAttribLocation(shaderProgramObject, VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, "aPosition");
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
    
        lightDiffusedUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Diffused[0]");
        lightAmbientUniform[0] =  GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Ambient[0]");
        lightSpecularUniform[0] =  GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Specular[0]");
        lightPositionUniform[0] = GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Position[0]");

        lightDiffusedUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Diffused[1]");
        lightAmbientUniform[1] =  GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Ambient[1]");
        lightSpecularUniform[1] =  GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Specular[1]");
        lightPositionUniform[1] = GLES32.glGetUniformLocation(shaderProgramObject, "uLight_Position[1]");
        
        materialDiffusedUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterial_Diffused");
        materialAmbientUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterial_Ambient");
        materialSpecularUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterial_Specular");
        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMaterial_Shininess");
        
        tapUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKeyPressed");
        
        // Geometry
        // Pyramid
        final float[] pyramid_position = new float[]
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

        final float[] pyramid_normals = new float[]
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

        // Vao pyramid
        GLES32.glGenVertexArrays(1, vao_pyramid, 0);
        GLES32.glBindVertexArray(vao_pyramid[0]);    
        
        // VBO for Position of pyramid
        GLES32.glGenBuffers(1, vbo_pyramid, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_pyramid[0]);

        // Prepare triangle vertics array for glBufferData
        ByteBuffer bytebuffer = ByteBuffer.allocateDirect(pyramid_position.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        FloatBuffer positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(pyramid_position);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, pyramid_position.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        
        // VBO of cube normal
        GLES32.glGenBuffers(1, vbo_normal, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_normal[0]);

        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(pyramid_normals.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(pyramid_normals);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, pyramid_normals.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL);
        
        // Unbind VAO & VBO
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        // Depth enable settings
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);


        // Clear color
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

        Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f, (float)width/ (float)height, 0.1f, 100.0f);
    }

    private void display()
    {
        // Code
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgramObject);

        float[] modelMatrix = new float[16];
        float[] viewMatrix = new float[16];
        float[] projectionMatrix = new float[16];
        float[] rotationMatrix = new float[16];

        Matrix.setIdentityM(projectionMatrix, 0);
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);
        Matrix.setIdentityM(rotationMatrix, 0);

        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -5.0f);
        Matrix.setRotateM(rotationMatrix,0, angle, 0.0f, 1.0f, 0.0f);
        
        Matrix.multiplyMM(modelMatrix,0, modelMatrix,0, rotationMatrix,0);
        Matrix.multiplyMM(projectionMatrix, 0, perspectiveProjectionMatrix, 0, projectionMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, projectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);

            GLES32.glUniform3fv(lightDiffusedUniform[0], 1, lightDiffused_0, 0);
            GLES32.glUniform3fv(lightAmbientUniform[0], 1, lightAmbient_0, 0);
            GLES32.glUniform3fv(lightSpecularUniform[0], 1, lightSpecular_0, 0);
            GLES32.glUniform4fv(lightPositionUniform[0], 1, lightPosition_0, 0);

            GLES32.glUniform3fv(lightDiffusedUniform[1], 1, lightDiffused_1, 0);
            GLES32.glUniform3fv(lightAmbientUniform[1], 1, lightAmbient_1, 0);
            GLES32.glUniform3fv(lightSpecularUniform[1], 1, lightSpecular_1, 0);
            GLES32.glUniform4fv(lightPositionUniform[1], 1, lightPosition_1, 0);

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
        GLES32.glBindVertexArray(vao_pyramid[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);

        // Unbind VAO
        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        // Render
        requestRender();    // Swapbuffer equalent
    }

    private void update()
    {
        angle = angle + 1.5f;
        if(angle >= 360.0f)
            angle = angle - 360.0f;
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

        if(vbo_pyramid[0] > 0)
        {
            GLES32.glDeleteBuffers(1, vbo_pyramid, 0);
            vbo_pyramid[0] = 0;
        }

        if(vao_pyramid[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_pyramid, 0);
            vao_pyramid[0] = 0;
        }

    }
}
