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
    private int[] vao = new int[1];
    private int[] vbo = new int[1];
    private int mvpMatrixUniform;
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
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
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
                                                                "uniform mat4 uMVPMatrix;\n"+
                                                                "void main(void)\n"+
                                                                "{\n"+
                                                                "gl_Position=uMVPMatrix * aPosition;\n"+
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
                                                                "out vec4 FragColor;\n"+
                                                                "void main(void)\n"+
                                                                "{\n"+
                                                                "FragColor = vec4(1.0,1.0,1.0,1.0);\n"+
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
        mvpMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uMVPMatrix");

        // Geometry
        final float[] square_position = new float[]
                                              {
                                                    1.0f, 1.0f, 0.0f,
                                                    -1.0f, 1.0f, 0.0f,
                                                    -1.0f, -1.0f, 0.0f,
                                                    1.0f, -1.0f, 0.0f
                                              };
        // Vao
        GLES32.glGenVertexArrays(1, vao, 0);
        GLES32.glBindVertexArray(vao[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, vbo, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);

        // Prepare triangle vertics array for glBufferData
        ByteBuffer bytebuffer = ByteBuffer.allocateDirect(square_position.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        FloatBuffer positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(square_position);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, square_position.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        // Depth enable settings
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        // Enabling culling
        GLES32.glEnable(GLES32.GL_CULL_FACE);

        // Clear color
        GLES32.glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

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

        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -3.0f);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
        GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glBindVertexArray(vao[0]);

        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glBindVertexArray(0);

        GLES32.glUseProgram(0);

        // Render
        requestRender();    // Swapbuffer equalent
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

        if(vbo[0] > 0)
        {
            GLES32.glDeleteBuffers(1, vbo, 0);
            vbo[0] = 0;
        }

        if(vao[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, vao, 0);
            vao[0] = 0;
        }

    }
}
