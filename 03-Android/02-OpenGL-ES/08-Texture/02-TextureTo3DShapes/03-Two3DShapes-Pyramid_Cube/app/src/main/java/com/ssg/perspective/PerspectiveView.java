package com.ssg.perspective;

import android.content.Context;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.opengl.GLUtils;

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
    private int[] vbo_cube_texcoord = new int[1];

    private int[] vao_pyramid = new int[1];
    private int[] vbo_pyramid = new int[1];
    private int[] vbo_pyramid_texcoord = new int[1];

    private float angle = 0.0f;
    private int[] kundali_texture = new int[1];
    private int[] stone_texture = new int[1];

    private final Context context;

    private int mvpMatrixUniform;
    private int textureSamplerUniform;
    private float[] perspectiveProjectionMatrix = new float[16]; 

    public PerspectiveView(Context _context)
    {
        super(_context);
        context = _context;


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

        // Update
        update();
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
                                                                "in vec2 aTexCoord;\n"+
                                                                "out vec2 oTexCoord;\n"+
                                                                "uniform mat4 uMVPMatrix;\n"+
                                                                "void main(void)\n"+
                                                                "{\n"+
                                                                "gl_Position=uMVPMatrix * aPosition;\n"+
                                                                "oTexCoord = aTexCoord;\n"+
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
                                                                "precision highp float;"+
                                                                "uniform highp sampler2D uTextureSampler;\n"+
                                                                "in vec2 oTexCoord;\n"+
                                                                "out vec4 FragColor;\n"+
                                                                "void main(void)\n"+
                                                                "{\n"+
                                                                "FragColor = texture(uTextureSampler, oTexCoord);\n"+
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
        GLES32.glBindAttribLocation(shaderProgramObject, VertexAttributesEnum.SSG_ATTRIBUTE_TEXCOORD, "aTexCoord");
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
        textureSamplerUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uTextureSampler");

        // Geometry
        final float[] cube_vertices = new float[]
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

        // Vao for cube
        GLES32.glGenVertexArrays(1, vao_cube, 0);
        GLES32.glBindVertexArray(vao_cube[0]);  
        
        // VBO for Position of cube
        GLES32.glGenBuffers(1, vbo_cube, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cube[0]);

        // Prepare triangle vertics array for glBufferData
        ByteBuffer bytebuffer = ByteBuffer.allocateDirect(cube_vertices.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        FloatBuffer positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(cube_vertices);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cube_vertices.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);

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

                                                    
        final float[] pyramid_vertices = new float[]
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
    
        final float[] pyramid_texcoord = new float[]
                                                    {
                                                        // front
                                                        0.5f, 1.0f, // front-top
                                                        0.0f, 0.0f, // front-left
                                                        1.0f, 0.0f, // front-right

                                                        // right
                                                        0.5f, 1.0f, // right-top
                                                        1.0f, 0.0f, // right-left
                                                        0.0f, 0.0f, // right-right

                                                        // back
                                                        0.5f, 1.0f, // back-top
                                                        0.0f, 0.0f, // back-left
                                                        1.0f, 0.0f, // back-right

                                                        // left
                                                        0.5f, 1.0f, // left-top
                                                        1.0f, 0.0f, // left-left
                                                        0.0f, 0.0f, // left-right
                                                    };

        // Vao_vao_pyramid
        GLES32.glGenVertexArrays(1, vao_pyramid, 0);
        GLES32.glBindVertexArray(vao_pyramid[0]);    
        
        // VBO for Position 
        GLES32.glGenBuffers(1, vbo_pyramid, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_pyramid[0]);

        // Prepare triangle vertics array for glBufferData
        bytebuffer = ByteBuffer.allocateDirect(pyramid_vertices.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        positionbuffer = bytebuffer.asFloatBuffer();
        positionbuffer.put(pyramid_vertices);
        positionbuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, pyramid_vertices.length * 4, positionbuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);

        // VBO Texcoord
        GLES32.glGenBuffers(1, vbo_pyramid_texcoord, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_pyramid_texcoord[0]);

        bytebuffer = ByteBuffer.allocateDirect(pyramid_texcoord.length * 4);
        bytebuffer.order(ByteOrder.nativeOrder());
        textureBuffer = bytebuffer.asFloatBuffer();
        textureBuffer.put(pyramid_texcoord);
        textureBuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, pyramid_texcoord.length * 4, textureBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_TEXCOORD, 2, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_TEXCOORD);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);


        // Depth enable settings
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);


        // Clear color
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


        kundali_texture[0] = loadGLTexture(R.raw.kundali);
        stone_texture[0] = loadGLTexture(R.raw.stone);


        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
    }

    private int loadGLTexture(int imageResourceID)
    {
        // Create bitmap factory options object
        BitmapFactory.Options options = new BitmapFactory.Options();

        // Dont scale image
        options.inScaled = false;

        // Create the bitmap image from image resource
        Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), imageResourceID, options);

        // Create texture
        int[] texture = new int[1];
        GLES32.glGenTextures(1, texture, 0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture[0]);
        GLES32.glPixelStorei(GLES32.GL_UNPACK_ALIGNMENT, 1);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR_MIPMAP_LINEAR);
        GLUtils.texImage2D(GLES32.GL_TEXTURE_2D, 0, bitmap, 0);
        GLES32.glGenerateMipmap(GLES32.GL_TEXTURE_2D);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);

        return(texture[0]);
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
        
        // Cube
        GLES32.glUseProgram(shaderProgramObject);

        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];
        float[] rotationMatrix = new float[16];
        float[] rotationMatrixX = new float[16];
        float[] rotationMatrixY = new float[16];
        float[] rotationMatrixZ = new float[16];
        float[] scaleMatrix = new float[16];

        // Cube
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.setIdentityM(rotationMatrix,0);
        Matrix.setIdentityM(rotationMatrixX,0);
        Matrix.setIdentityM(rotationMatrixY,0);
        Matrix.setIdentityM(rotationMatrixZ,0);
        Matrix.setIdentityM(scaleMatrix,0);

        Matrix.translateM(modelViewMatrix, 0, 2.0f, 0.0f, -5.0f);    
        Matrix.scaleM(scaleMatrix,0, 0.8f, 0.8f, 0.8f); 

        Matrix.setRotateM(rotationMatrixX,0, angle, 1.0f, 0.0f, 0.0f);
        Matrix.setRotateM(rotationMatrixY,0, angle, 0.0f, 1.0f, 0.0f);
        Matrix.setRotateM(rotationMatrixZ,0, angle, 0.0f, 0.0f, 1.0f);

        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, scaleMatrix, 0);
        Matrix.multiplyMM(rotationMatrix, 0, rotationMatrixZ, 0, rotationMatrixY, 0);
        Matrix.multiplyMM(rotationMatrix, 0, rotationMatrix, 0, rotationMatrixX, 0);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix, 0);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
        
        GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, kundali_texture[0]);
        GLES32.glUniform1i(textureSamplerUniform, 0);

        GLES32.glBindVertexArray(vao_cube[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);

        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        // Pyramid
        GLES32.glUseProgram(shaderProgramObject);

        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);

        Matrix.translateM(modelViewMatrix, 0, -2.0f, 0.0f, -5.0f);
        Matrix.setRotateM(rotationMatrix,0, angle, 0.0f, 1.0f, 0.0f);

        Matrix.multiplyMM(modelViewMatrix,0, modelViewMatrix,0, rotationMatrix,0);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
        
        GLES32.glUniformMatrix4fv(mvpMatrixUniform, 1, false, modelViewProjectionMatrix, 0);
        
        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, stone_texture[0]);
        GLES32.glUniform1i(textureSamplerUniform, 0);

        GLES32.glBindVertexArray(vao_pyramid[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);
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

        if(kundali_texture[0] > 0)
        {
            GLES32.glDeleteTextures(1, kundali_texture, 0);
            kundali_texture[0] = 0;
        }

        if(stone_texture[0] > 0)
        {
            GLES32.glDeleteTextures(1, stone_texture, 0);
            stone_texture[0] = 0;
        }

        if(vbo_cube_texcoord[0] > 0)
        {
            GLES32.glDeleteBuffers(1, vbo_cube_texcoord, 0);
            vbo_cube_texcoord[0] = 0;
        }

        if(vbo_pyramid_texcoord[0] > 0)
        {
            GLES32.glDeleteBuffers(1, vbo_pyramid_texcoord, 0);
            vbo_pyramid_texcoord[0] = 0;
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
