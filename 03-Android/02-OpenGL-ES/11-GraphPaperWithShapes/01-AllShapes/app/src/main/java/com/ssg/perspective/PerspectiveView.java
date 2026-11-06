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

    // Green Line
    private int[] VAO_Green_Line = new int[1];
    private int[] VBO_Green_Line_Color = new int[1];
    private int[] VBO_Green_Line = new int[1];

    // Red Line
    private int[] VAO_Red_Line = new int[1];
    private int[] VBO_Red_Line_Color = new int[1];
    private int[] VBO_Red_Line = new int[1];

    // Horizontal Blue Lines
    private int[] VAO_Horizontal_Blue_Line = new int[1];
    private int[] VBO_Horizontal_Blue_Line_Color = new int[1];
    private int[] VBO_Horizontal_Blue_Line = new int[1];

    // Vertical Blue Lines
    private int[] VAO_Vertical_Blue_Line = new int[1];
    private int[] VBO_Vertical_Blue_Line_Color = new int[1];
    private int[] VBO_Vertical_Blue_Line = new int[1];

    // Hollow Triangle
    private int[] VAO_Triangle = new int[1];
    private int[] VBO_Triangle = new int[1];
    private int[] VBO_Triangle_Color = new int[1];

    // Hollow Square
    private int[] VAO_Square = new int[1];
    private int[] VBO_Square = new int[1];
    private int[] VBO_Square_Color = new int[1];

    // Hollow Circle
    private int[] VAO_Circle = new int[1];
    private int[] VBO_Circle = new int[1];
    private int[] VBO_Circle_Color = new int[1];
    
    private static final int Circle_Points = 360;

    private int single_tap = 0;

    private boolean GRAPH = false;
    private boolean SQUARE = false;
    private boolean TRIANGLE = false;
    private boolean CIRCLE = false;

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
        single_tap++;

        switch (single_tap) 
        {
            case 1:
                if(GRAPH == true)
                {
                    GRAPH = false;
                }
                else
                {
                    GRAPH = true;
                }
                break;
            case 2:
                if(SQUARE == true)
                {
                    SQUARE = false;
                }
                else
                {
                    SQUARE = true;
                }
                break;
            case 3:
                if(CIRCLE == true)
                {
                    CIRCLE = false;
                }
                else
                {
                    CIRCLE = true;
                }
                break;
            case 4:
                if(TRIANGLE == true)
                {
                    TRIANGLE = false;
                }
                else
                {
                    TRIANGLE = true;
                }
                break;
            
            case 5:
                GRAPH = false;
                break;

            case 6:
                SQUARE = false;
                break;

            case 7:
                CIRCLE = false;
                break;
            
            case 8: 
                TRIANGLE = false;
                break;

            case 9:
                single_tap = 0;
                break;
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
            "in vec4 aPosition;\n"+
            "in vec4 aColor;\n"+
            "out vec4 oColor;\n"+
            "uniform mat4 uMVPMatrix;\n"+
            "void main(void)\n"+
            "{\n"+
            "gl_Position=uMVPMatrix * aPosition;\n"+
            "oColor=aColor;\n"+
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
            "precision highp float;\n"+
            "in vec4 oColor;\n"+
            "out vec4 FragColor;\n"+
            "void main(void)\n"+
            "{\n"+
            "FragColor=oColor;\n"+
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
        GLES32.glBindAttribLocation(shaderProgramObject, VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, "aColor");
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
        final float[] Red_Line_Vertices = new float[]
        {
            -2.0f, 0.0f, 0.0f,
            2.0f, 0.0f, 0.0f
        };

        final float[] Red_Line_Color = new float[]
        {
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f
        };

        final float[] Green_Line_Vertices = new float[]
        {
            0.0f, 2.0f, 0.0f,
            0.0f, -2.0f, 0.0f
        };

        final float[] Green_Line_Color = new float[]
        {
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };

        final float[] Horizontal_Line_Vertices = new float[]
        {
            -2.0f, 2.0f, 0.0f,
            2.0f, 2.0f, 0.0f   
        };

        final float[] Horizontal_Line_Color = new float[]
        {
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f
        };

        final float[] Vertical_Line_Vertices = new float[]
        {
            2.0f, 2.0f, 0.0f,
            2.0f, -2.0f, 0.0f 
        };

        final float[] Vertical_Line_Color = new float[]
        {
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f
        };

        final float[] Hollow_Triangle_Vertices = new float[]
        {
            0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f
        };

        final float[] Hollow_Triangle_Color = new float[]
        {
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f
        };


        final float[] Hollow_Square_Vertices = new float[]
        {
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f  
        };

        final float[] Hollow_Square_Color = new float[]
        {
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f
        };

        float[] Circle_Vertices = new float[Circle_Points * 3];

        for(int i = 0; i < Circle_Points; i++)
        {
            double angle = 2 * Math.PI * i / Circle_Points;
            Circle_Vertices[i * 3] = (float)Math.cos(angle);
            Circle_Vertices[i * 3 + 1] = (float)Math.sin(angle);
            Circle_Vertices[i * 3 + 2] = 0.0f;
        }

        // ----------------------Red Line-----------------------------------
        // VAO
        GLES32.glGenVertexArrays(1, VAO_Red_Line, 0);
        GLES32.glBindVertexArray(VAO_Red_Line[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, VBO_Red_Line, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Red_Line[0]);
        // Prepare triangle vertics array for glBufferData
        ByteBuffer bytebuffer = ByteBuffer.allocateDirect(Red_Line_Vertices.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        FloatBuffer positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(Red_Line_Vertices);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Red_Line_Vertices.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        
        // VBO for color
        GLES32.glGenBuffers(1, VBO_Red_Line_Color, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Red_Line_Color[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Red_Line_Color.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        FloatBuffer colorbuffer = bytebuffer.asFloatBuffer();
        colorbuffer.put(Red_Line_Color);
        colorbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Red_Line_Color.length * 4, colorbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);


        // ----------------------Green Line-----------------------------------
        // VAO
        GLES32.glGenVertexArrays(1, VAO_Green_Line, 0);
        GLES32.glBindVertexArray(VAO_Green_Line[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, VBO_Green_Line, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Green_Line[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Green_Line_Vertices.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(Green_Line_Vertices);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Green_Line_Vertices.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        
        // VBO for color
        GLES32.glGenBuffers(1, VBO_Green_Line_Color, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Green_Line_Color[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Green_Line_Color.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        colorbuffer = bytebuffer.asFloatBuffer();
        colorbuffer.put(Green_Line_Color);
        colorbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Green_Line_Color.length * 4, colorbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);


        // ---------------------- Horizontal Line-----------------------------------
        // VAO
        GLES32.glGenVertexArrays(1, VAO_Horizontal_Blue_Line, 0);
        GLES32.glBindVertexArray(VAO_Horizontal_Blue_Line[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, VBO_Horizontal_Blue_Line, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Horizontal_Blue_Line[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Horizontal_Line_Vertices.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(Horizontal_Line_Vertices);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Horizontal_Line_Vertices.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        
        // VBO for color
        GLES32.glGenBuffers(1, VBO_Horizontal_Blue_Line_Color, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Horizontal_Blue_Line_Color[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Horizontal_Line_Color.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        colorbuffer = bytebuffer.asFloatBuffer();
        colorbuffer.put(Horizontal_Line_Color);
        colorbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Horizontal_Line_Color.length * 4, colorbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);


        // ---------------------- Vertical Line-----------------------------------
        // VAO
        GLES32.glGenVertexArrays(1, VAO_Vertical_Blue_Line, 0);
        GLES32.glBindVertexArray(VAO_Vertical_Blue_Line[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, VBO_Vertical_Blue_Line, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Vertical_Blue_Line[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Vertical_Line_Vertices.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(Vertical_Line_Vertices);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Vertical_Line_Vertices.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        
        // VBO for color
        GLES32.glGenBuffers(1, VBO_Vertical_Blue_Line_Color, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Vertical_Blue_Line_Color[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Vertical_Line_Color.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        colorbuffer = bytebuffer.asFloatBuffer();
        colorbuffer.put(Vertical_Line_Color);
        colorbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Vertical_Line_Color.length * 4, colorbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);
        

        //---------------------- Hollow Square --------------------------------
        // VAO
        GLES32.glGenVertexArrays(1, VAO_Square, 0);
        GLES32.glBindVertexArray(VAO_Square[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, VBO_Square, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Square[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Hollow_Square_Vertices.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(Hollow_Square_Vertices);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Hollow_Square_Vertices.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        
        // VBO for color
        GLES32.glGenBuffers(1, VBO_Square_Color, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Square_Color[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Hollow_Square_Color.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        colorbuffer = bytebuffer.asFloatBuffer();
        colorbuffer.put(Hollow_Square_Color);
        colorbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Hollow_Square_Color.length * 4, colorbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        //========================== Hollow Triangle ==========================
        // VAO
        GLES32.glGenVertexArrays(1, VAO_Triangle, 0);
        GLES32.glBindVertexArray(VAO_Triangle[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, VBO_Triangle, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Triangle[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Hollow_Triangle_Vertices.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(Hollow_Triangle_Vertices);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Hollow_Triangle_Vertices.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        
        // VBO for color
        GLES32.glGenBuffers(1, VBO_Triangle_Color, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Triangle_Color[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Hollow_Triangle_Color.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        colorbuffer = bytebuffer.asFloatBuffer();
        colorbuffer.put(Hollow_Triangle_Color);
        colorbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Hollow_Triangle_Color.length * 4, colorbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);


        //==================== Hollow Circle ============================
        // VAO
        GLES32.glGenVertexArrays(1, VAO_Circle, 0);
        GLES32.glBindVertexArray(VAO_Circle[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, VBO_Circle, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_Circle[0]);
        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(Circle_Vertices.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(Circle_Vertices);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, Circle_Vertices.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        // Color 
        GLES32.glVertexAttrib3f(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, 1.0f, 1.0f, 0.0f);
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

        if(GRAPH == true)
        {
            // ------------------------------ Horizontal Blue Lines -----------------------
            for(int i = 0; i < 81; i++)
            {
                GLES32.glUseProgram(shaderProgramObject);

                float[] modelViewMatrix = new float[16];
                float[] modelViewProjectionMatrix = new float[16];

                Matrix.setIdentityM(modelViewMatrix, 0);
                Matrix.setIdentityM(modelViewProjectionMatrix, 0);

                Matrix.translateM(modelViewMatrix, 0, 0.0f, 2.0f * (- i) * 0.025f, -5.0f);

                Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(VAO_Horizontal_Blue_Line[0]);
                GLES32.glLineWidth(0.5f);
                GLES32.glDrawArrays(GLES32.GL_LINES, 0, 3);
                GLES32.glBindVertexArray(0);

                GLES32.glUseProgram(0);
            }

            // ------------------------------ Vertical Blue Lines -----------------------
            for(int i = 0; i < 81; i++)
            {
                GLES32.glUseProgram(shaderProgramObject);

                float[] modelViewMatrix = new float[16];
                float[] modelViewProjectionMatrix = new float[16];

                Matrix.setIdentityM(modelViewMatrix, 0);
                Matrix.setIdentityM(modelViewProjectionMatrix, 0);

                Matrix.translateM(modelViewMatrix, 0, 2.0f * (- i) * 0.025f, 0.0f, -5.0f);

                Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(VAO_Vertical_Blue_Line[0]);
                GLES32.glLineWidth(0.5f);
                GLES32.glDrawArrays(GLES32.GL_LINES, 0, 3);
                GLES32.glBindVertexArray(0);

                GLES32.glUseProgram(0);
            }

        //=================== 5th Thick Line ==========================
            for(int i = 0; i < 81; i++)
            {
                GLES32.glUseProgram(shaderProgramObject);

                float[] modelViewMatrix = new float[16];
                float[] modelViewProjectionMatrix = new float[16];

                Matrix.setIdentityM(modelViewMatrix, 0);
                Matrix.setIdentityM(modelViewProjectionMatrix, 0);

                Matrix.translateM(modelViewMatrix, 0, 0.0f, 2.0f * (- i) * 0.025f, -5.0f);

                Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(VAO_Horizontal_Blue_Line[0]);
                if(i % 5 == 0) 
                {
                    GLES32.glLineWidth(2.0f); // Thicker line
                } 
                else 
                {
                    GLES32.glLineWidth(0.5f); // Regular line
                }
                GLES32.glDrawArrays(GLES32.GL_LINES, 0, 3);
                GLES32.glBindVertexArray(0);

                GLES32.glUseProgram(0);
            }

            for(int i = 0; i < 81; i++)
            {
                GLES32.glUseProgram(shaderProgramObject);

                float[] modelViewMatrix = new float[16];
                float[] modelViewProjectionMatrix = new float[16];

                Matrix.setIdentityM(modelViewMatrix, 0);
                Matrix.setIdentityM(modelViewProjectionMatrix, 0);

                Matrix.translateM(modelViewMatrix, 0, 2.0f * (- i) * 0.025f, 0.0f, -5.0f);

                Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
                GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

                GLES32.glBindVertexArray(VAO_Vertical_Blue_Line[0]);
                if(i % 5 == 0) 
                {
                    GLES32.glLineWidth(2.0f); // Thicker line
                } 
                else 
                {
                    GLES32.glLineWidth(0.5f); // Regular line
                }
                GLES32.glDrawArrays(GLES32.GL_LINES, 0, 3);
                GLES32.glBindVertexArray(0);

                GLES32.glUseProgram(0);
            }

        //---------------------- Red Line ------------------------------------
            GLES32.glUseProgram(shaderProgramObject);

            float[] modelViewMatrix = new float[16];
            float[] modelViewProjectionMatrix = new float[16];

            Matrix.setIdentityM(modelViewMatrix, 0);
            Matrix.setIdentityM(modelViewProjectionMatrix, 0);

            Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -5.0f);

            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

            GLES32.glBindVertexArray(VAO_Red_Line[0]);
            GLES32.glLineWidth(2.0f);
            GLES32.glDrawArrays(GLES32.GL_LINES, 0, 3);
            GLES32.glBindVertexArray(0);

            GLES32.glUseProgram(0);

        //----------------------------- Green Line ---------------------------
            GLES32.glUseProgram(shaderProgramObject);

            modelViewMatrix = new float[16];
            modelViewProjectionMatrix = new float[16];

            Matrix.setIdentityM(modelViewMatrix, 0);
            Matrix.setIdentityM(modelViewProjectionMatrix, 0);

            Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -5.0f);

            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

            GLES32.glBindVertexArray(VAO_Green_Line[0]);
            GLES32.glLineWidth(2.0f);
            GLES32.glDrawArrays(GLES32.GL_LINES, 0, 3);
            GLES32.glBindVertexArray(0);

            GLES32.glUseProgram(0);
            }
        
        if(SQUARE  == true)
        {
            //========================== Hollow Square ==========================
            GLES32.glUseProgram(shaderProgramObject);

            float[] modelViewMatrix = new float[16];
            float[] modelViewProjectionMatrix = new float[16];

            Matrix.setIdentityM(modelViewMatrix, 0);
            Matrix.setIdentityM(modelViewProjectionMatrix, 0);

            Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -5.0f);

            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

            GLES32.glBindVertexArray(VAO_Square[0]);
            GLES32.glLineWidth(2.0f);
            GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 4);
            GLES32.glBindVertexArray(0);

            GLES32.glUseProgram(0);
        }

        if(CIRCLE == true)
        {
            //========================== Hollow Circle ==========================
            GLES32.glUseProgram(shaderProgramObject);

            float[] modelViewMatrix = new float[16];
            float[] modelViewProjectionMatrix = new float[16];

            Matrix.setIdentityM(modelViewMatrix, 0);
            Matrix.setIdentityM(modelViewProjectionMatrix, 0);

            Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -5.0f);

            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

            GLES32.glBindVertexArray(VAO_Circle[0]);
            GLES32.glLineWidth(2.0f);
            GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, Circle_Points);
            GLES32.glBindVertexArray(0);

            GLES32.glUseProgram(0);
        }

        if(TRIANGLE == true)
        {
            //========================== Hollow Triangle ==========================
            GLES32.glUseProgram(shaderProgramObject);

            float[] modelViewMatrix = new float[16];
            float[] modelViewProjectionMatrix = new float[16];

            Matrix.setIdentityM(modelViewMatrix, 0);
            Matrix.setIdentityM(modelViewProjectionMatrix, 0);

            Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -5.0f);

            Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
            GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

            GLES32.glBindVertexArray(VAO_Triangle[0]);
            GLES32.glLineWidth(2.0f);
            GLES32.glDrawArrays(GLES32.GL_LINE_LOOP, 0, 3);
            GLES32.glBindVertexArray(0);

            GLES32.glUseProgram(0);
        }
    
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

        //====================== Hollow Circle =============================
        if(VBO_Circle[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Circle, 0);
            VBO_Circle[0] = 0;
        }

        if(VAO_Circle[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, VAO_Circle, 0);
            VAO_Circle[0] = 0;
        }

        //====================== Hollow Triangle ============================
        if(VBO_Triangle_Color[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Triangle_Color, 0);
            VBO_Triangle_Color[0] = 0;
        }

        if(VBO_Triangle[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Triangle, 0);
            VBO_Triangle[0] = 0;
        }

        if(VAO_Triangle[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, VAO_Triangle, 0);
            VAO_Triangle[0] = 0;
        }

        //====================== Hollow Square ============================
        if(VBO_Square_Color[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Square_Color, 0);
            VBO_Square_Color[0] = 0;
        }

        if(VBO_Square[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Square, 0);
            VBO_Square[0] = 0;
        }

        if(VAO_Square[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, VAO_Square, 0);
            VAO_Square[0] = 0;
        }
        
        // ========================== Horizontal Lines =======================
        if(VBO_Horizontal_Blue_Line_Color[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Horizontal_Blue_Line_Color, 0);
            VBO_Horizontal_Blue_Line_Color[0] = 0;
        }

        if(VBO_Horizontal_Blue_Line[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Horizontal_Blue_Line, 0);
            VBO_Horizontal_Blue_Line[0] = 0;
        }

        if(VAO_Horizontal_Blue_Line[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, VAO_Horizontal_Blue_Line, 0);
            VAO_Horizontal_Blue_Line[0] = 0;
        }
        
        // ========================== Vertical Lines =======================
        if(VBO_Vertical_Blue_Line_Color[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Vertical_Blue_Line_Color, 0);
            VBO_Vertical_Blue_Line_Color[0] = 0;
        }

        if(VBO_Vertical_Blue_Line[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Vertical_Blue_Line, 0);
            VBO_Vertical_Blue_Line[0] = 0;
        }

        if(VAO_Vertical_Blue_Line[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, VAO_Vertical_Blue_Line, 0);
            VAO_Vertical_Blue_Line[0] = 0;
        }

        //-------------------- Green Line ------------------------
        if(VBO_Green_Line_Color[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Green_Line_Color, 0);
            VBO_Green_Line_Color[0] = 0;
        }

        if(VBO_Green_Line[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Green_Line, 0);
            VBO_Green_Line[0] = 0;
        }

        if(VAO_Green_Line[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, VAO_Green_Line, 0);
            VAO_Green_Line[0] = 0;
        }
        
        //---------------------- Red Line ------------------------
        if(VBO_Red_Line_Color[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Red_Line_Color, 0);
            VBO_Red_Line_Color[0] = 0;
        }

        if(VBO_Red_Line[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_Red_Line, 0);
            VBO_Red_Line[0] = 0;
        }

        if(VAO_Red_Line[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, VAO_Red_Line, 0);
            VAO_Red_Line[0] = 0;
        }
    }
}
