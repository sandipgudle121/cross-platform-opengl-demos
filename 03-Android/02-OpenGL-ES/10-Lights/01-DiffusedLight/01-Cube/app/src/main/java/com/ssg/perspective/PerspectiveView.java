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
    private int[] vao_cube = new int[1];
    private int[] vbo_cube = new int[1];
    private int[] vbo_normal = new int[1];

    private float[] materialDiffused = {0.5f, 0.5f, 0.5f, 1.0f};
    private float[] lightdiffused = {1.0f, 1.0f, 1.0f, 1.0f};
    private float[] lightPosition = {0.0f, 0.0f, 2.0f, 1.0f};

    private float angle = 0.0f;
    private boolean lightEnabled = false;
    private boolean animationEnabled = false;

    private int lightdiffuseduniform;
    private int materialdiffusedUniform;
    private int lightPositionUniform;

    private int tapUniform;
    private int modelViewmatrixUniform;
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
        final String vertexShaderSourceCode = String.format(   
                                                                "#version 320 es\n"+
                                                                "\n"+
                                                                "in vec4 aPosition;\n"+
                                                                "in vec3 aNormal;\n"+
                                                                "in vec4 aColor;\n"+
                                                                "out vec4 oColor;\n"+
                                                        "uniform mat4 uModelViewMatrix;\n"+
                                                                "uniform mat4 uProjectionMatrix;\n"+
                                                                "uniform vec3 uLd;\n"+
                                                                "uniform vec3 uKd;\n"+
                                                                "uniform vec4 uLightPosition;\n"+
                                                                "uniform int uKeyPressed;\n"+
                                                                "out vec3 oDiffusedLight;\n"+
                                                                "void main(void)\n"+
                                                                "{\n"+
                                                                "if(uKeyPressed == 1)\n"+
                                                                "{\n"+
                                                                "vec4 iPosition = uModelViewMatrix * aPosition;\n"+
                                                                "mat3 normalMatrix = mat3(transpose(inverse(uModelViewMatrix)));\n"+
                                                                "vec3 n = normalize(normalMatrix * aNormal);\n"+
                                                                "vec3 s = normalize(vec3(uLightPosition - iPosition));\n"+
                                                                "oDiffusedLight = uLd * uKd * dot(s,n);\n"+
                                                                "}\n"+
                                                                "else\n"+
                                                                "{\n"+
                                                                "oDiffusedLight = vec3(0.0f, 0.0f, 0.0f);\n"+
                                                                "}\n"+
                                                                "gl_Position = uProjectionMatrix * uModelViewMatrix * aPosition;\n"+
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
        final String fragmentShaderSourceCode = String.format(   
                                                                "#version 320 es\n"+
                                                                "\n"+
                                                                "precision highp  float;"+
                                                                "precision highp  int;"+
                                                                "in vec3 oDiffusedLight;\n"+
                                                                "uniform int uKeyPressed;\n"+
                                                                "out vec4 FragColor;\n"+
                                                                "void main(void)\n"+
                                                                "{\n"+
                                                                "if(uKeyPressed == 1)\n"+
                                                                "{\n"+
                                                                "FragColor = vec4(oDiffusedLight, 1.0);\n"+
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
        modelViewmatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uModelViewMatrix");
        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
        lightdiffuseduniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLd");
        materialdiffusedUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKd");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uLightposition");
        tapUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uKeyPressed");

        // Geometry
        // Cube
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
        
        // Cube normals                                                    
        final float[] cube_normals = new float[]
        {
            // front surface
            0.0f,  0.0f,  1.0f, // top-right of front
            0.0f,  0.0f,  1.0f, // top-left of front
            0.0f,  0.0f,  1.0f, // bottom-left of front
            0.0f,  0.0f,  1.0f, // bottom-right of front

            // right surface
            1.0f,  0.0f,  0.0f, // top-right of right
            1.0f,  0.0f,  0.0f, // top-left of right
            1.0f,  0.0f,  0.0f, // bottom-left of right
            1.0f,  0.0f,  0.0f, // bottom-right of right

            // back surface
            0.0f,  0.0f, -1.0f, // top-right of back
            0.0f,  0.0f, -1.0f, // top-left of back
            0.0f,  0.0f, -1.0f, // bottom-left of back
            0.0f,  0.0f, -1.0f, // bottom-right of back

            // left surface
            -1.0f,  0.0f,  0.0f, // top-right of left
            -1.0f,  0.0f,  0.0f, // top-left of left
            -1.0f,  0.0f,  0.0f, // bottom-left of left
            -1.0f,  0.0f,  0.0f, // bottom-right of left

            // top surface
            0.0f,  1.0f,  0.0f, // top-right of top
            0.0f,  1.0f,  0.0f, // top-left of top
            0.0f,  1.0f,  0.0f, // bottom-left of top
            0.0f,  1.0f,  0.0f, // bottom-right of top

            // bottom surface
            0.0f, -1.0f,  0.0f, // top-right of bottom
            0.0f, -1.0f,  0.0f, // top-left of bottom
            0.0f, -1.0f,  0.0f, // bottom-left of bottom
            0.0f, -1.0f,  0.0f, // bottom-right of bottom
        };

        // Vao for cube
        GLES32.glGenVertexArrays(1, vao_cube, 0);
        GLES32.glBindVertexArray(vao_cube[0]);    
        
        // VBO for Position of cube
        GLES32.glGenBuffers(1, vbo_cube, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cube[0]);

        // Prepare triangle vertics array for glBufferData
        ByteBuffer bytebuffer = ByteBuffer.allocateDirect(cube_position.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        FloatBuffer positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(cube_position);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cube_position.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        

        // VBO of cube normal
        GLES32.glGenBuffers(1, vbo_normal, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_normal[0]);

        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(cube_normals.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(cube_normals);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cube_normals.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
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

        Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -6.0f);    
        Matrix.setRotateM(rotationMatrixX,0, angle, 1.0f, 0.0f, 0.0f);
        Matrix.setRotateM(rotationMatrixY,0, angle, 0.0f, 1.0f, 0.0f);
        Matrix.setRotateM(rotationMatrixZ,0, angle, 0.0f, 0.0f, 1.0f);

        Matrix.multiplyMM(rotationMatrix, 0, rotationMatrixZ, 0, rotationMatrixY, 0);
        Matrix.multiplyMM(rotationMatrix, 0, rotationMatrix, 0, rotationMatrixX, 0);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix, 0);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
        
        GLES32.glUniformMatrix4fv(modelViewmatrixUniform, 1, false, modelViewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            GLES32.glUniform3fv(lightdiffuseduniform, 1, lightdiffused, 0);
            GLES32.glUniform3fv(materialdiffusedUniform, 1, materialDiffused, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, lightPosition, 0);
        }
        else
        {
            GLES32.glUniform1i(tapUniform, 0);
        }

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

    private void update()
    {
        angle = angle + 1.5f;
        if(angle >= 360.0f)
        {
            angle = angle - 360.0f;
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
        
        if(vbo_normal[0] > 0)
        {
            GLES32.glDeleteBuffers(1, vbo_normal, 0);
            vbo_normal[0] = 0;
        }

        if(vbo_cube[0] > 0)
        {
            GLES32.glDeleteBuffers(1, vbo_cube, 0);
            vbo_cube[0] = 0;
        }

        if(vao_cube[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_cube, 0);
            vao_cube[0] = 0;
        }
    }
}
