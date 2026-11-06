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
    private int[] VAO = new int[1];
    private int[] VBO_CUBE = new int[1];
    private int[] VBO_TEXCOORD = new int[1];
    private int[] VBO_COLOR = new int[1];

    private float angle = 0.0f;
    private int[] marble_texture = new int[1];

    private float[] lightDiffused = { 1.0f, 1.0f, 1.0f, 1.0f };
    private float[] lightAmbient = { 0.1f, 0.1f, 0.1f, 1.0f };
    private float[] lightSpecular = { 1.0f, 1.0f, 1.0f, 1.0f };
    private float[] lightPosition = { 0.0f, 0.0f, 2.0f, 1.0f };

    private float[] materialDiffused = { 1.0f, 1.0f, 1.0f, 1.0f };
    private float[] materialAmbient = { 0.0f, 0.0f, 0.0f, 1.0f };
    private float[] materialSpecular = { 1.0f, 1.0f, 1.0f, 1.0f };
    private float materialShininess = 128.0f;

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

    private int textureSamplerUniform;
    private int viewMatrixUniform;
    private int modelMatrixUniform;
    private int projectionMatrixUniform;

    private final Context context;

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
            "precision highp float;\n"+
            "in vec4 aPosition;\n"+
            "in vec2 aTexCoord;\n"+
            "in vec3 aNormal;\n"+
            "in vec4 aColor;\n"+
            "out vec4 oColor;\n"+
            "out vec2 oTexCoord;\n"+
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
            "oTransformed_Normals = vec3(0.0, 0.0, 0.0);\n"+
            "oLight_Direction = vec3(0.0, 0.0, 0.0);\n"+
            "oViewer_Vector = vec3(0.0, 0.0, 0.0);\n"+
            "}\n"+
            "oColor = aColor;\n"+
            "oTexCoord = aTexCoord;\n"+
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
            "precision highp float;\n"+
            "precision highp  int;\n"+
            "in vec2 oTexCoord;\n"+
            "in vec4 oColor;\n"+
            "\n"+
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
            "uniform sampler2D uTextureSampler;\n"+
            "\n"+
            "out vec4 FragColor;\n"+
            "void main(void)\n"+
            "{\n"+
            "vec3 phong_ADS_Light;\n"+
            "if(uKeyPressed == 1)\n"+
            "{\n"+
            "vec3 normalized_Transformed_Normals = normalize(oTransformed_Normals);\n"+
            "vec3 normalized_Light_Direction = normalize(oLight_Direction);\n"+
            "vec3 normalized_Viewer_Vector = normalize(oViewer_Vector);\n"+
            "\n"+
            "vec3 lightAmbient = uLight_Ambient * uMaterial_Ambient;\n"+
            "vec3 lightDiffused = uLight_Diffused * uMaterial_Diffused * max(dot(normalized_Light_Direction, normalized_Transformed_Normals), 0.0);\n"+
            "vec3 reflection_Vector = reflect(-normalized_Light_Direction, normalized_Transformed_Normals);\n"+
            "vec3 lightSpecular = uLight_Specular * uMaterial_Specular * pow(max(dot(reflection_Vector, normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
            "phong_ADS_Light = lightAmbient + lightDiffused + lightSpecular;\n"+
            "}\n"+
            "else\n"+
            "{\n"+
            "phong_ADS_Light = vec3(1.0, 1.0, 1.0);\n"+
            "}\n"+
            "vec3 texColor = vec3(texture(uTextureSampler, oTexCoord));\n"+
            "FragColor = vec4(texColor * vec3(oColor) * phong_ADS_Light, 1.0);\n"+
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
        GLES32.glBindAttribLocation(shaderProgramObject, VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, "aColor");
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
        textureSamplerUniform = GLES32.glGetUniformLocation(shaderProgramObject, "uTextureSampler");

        // Geometry
        final float[] cube_PCNT = new float[]
        {
            // front
            // position				// color			 // normals				// texcoords
            1.0f,  1.0f,  1.0f,	    1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 	1.0f, 0.0f, 0.0f,	 0.0f,  0.0f,  1.0f,	1.0f, 0.0f,
                            
            // right			 
            // position				// color			 // normals				// texcoords
            1.0f,  1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
            1.0f,  1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
            1.0f, -1.0f,  1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,	0.0f, 0.0f, 1.0f,	 1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
                            
            // back				 
            // position				// color			 // normals				// texcoords
            1.0f,  1.0f, -1.0f,	    1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,	1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,	    1.0f, 1.0f, 0.0f,	 0.0f,  0.0f, -1.0f,	1.0f, 0.0f,
                            
            // left				 
            // position				// color			 // normals				// texcoords
            -1.0f,  1.0f,  1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,	1.0f, 0.0f, 1.0f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
                            
            // top				 
            // position				// color			 // normals				// texcoords
            1.0f,  1.0f, -1.0f,	    0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	0.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,	0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	0.0f, 0.0f,
            1.0f,  1.0f,  1.0f,	    0.0f, 1.0f, 0.0f,	 0.0f,  1.0f,  0.0f,	1.0f, 0.0f,
                            
            // bottom			 
            // position				// color			 // normals				// texcoords
            1.0f, -1.0f,  1.0f,	    1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	1.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,	1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,	    1.0f, 0.5f, 0.0f,	 0.0f, -1.0f,  0.0f,	1.0f, 0.0f,     
        };

        // Vao for cube
        GLES32.glGenVertexArrays(1, VAO, 0);
        GLES32.glBindVertexArray(VAO[0]);  
        
        // VBO
        GLES32.glGenBuffers(1, VBO_CUBE, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, VBO_CUBE[0]);

        // Prepare buffer for vertex data (positions, colors, normals, texcoords)
        ByteBuffer bytebuffer = ByteBuffer.allocateDirect(cube_PCNT.length * 4); // 4 bytes per float
        bytebuffer.order(ByteOrder.nativeOrder()); // Ensures byte order is correct for OpenGL
        FloatBuffer positionBuffer = bytebuffer.asFloatBuffer();
        positionBuffer.put(cube_PCNT); // Put vertex data into the buffer
        positionBuffer.position(0); // Reset buffer position to the start

        // Transfer data to GPU with glBufferData
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cube_PCNT.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);

        // POSITION 
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 11 * 4, 0);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_POSITION);

        // COLOR 
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 11 * 4, 3 * 4);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_COLOR);

        // NORMAL 
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 11 * 4, 6 * 4);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_NORMAL);

        // TEXCOORD 
        GLES32.glVertexAttribPointer(VertexAttributesEnum.SSG_ATTRIBUTE_TEXCOORD, 2, GLES32.GL_FLOAT, false, 11 * 4, 9 * 4);
        GLES32.glEnableVertexAttribArray(VertexAttributesEnum.SSG_ATTRIBUTE_TEXCOORD);

        // Unbind the buffer after finishing
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        // Depth enable settings
        GLES32.glClearDepthf(1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        // Clear color
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        marble_texture[0] = loadGLTexture(R.raw.marble);
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
        GLES32.glUseProgram(shaderProgramObject);

        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];
        float[] translationMatrix = new float[16];
        float[] scaleMatrix = new float[16];
        float[] rotationMatrixX = new float[16];
        float[] rotationMatrixY = new float[16];
        float[] rotationMatrixZ = new float[16];
        float[] rotationMatrix = new float[16];
        float[] viewMatrix = new float[16];

        // Initialize matrices
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);
        Matrix.setIdentityM(translationMatrix, 0);
        Matrix.setIdentityM(scaleMatrix, 0);
        Matrix.setIdentityM(rotationMatrixX, 0);
        Matrix.setIdentityM(rotationMatrixY, 0);
        Matrix.setIdentityM(rotationMatrixZ, 0);
        Matrix.setIdentityM(rotationMatrix, 0);
        Matrix.setIdentityM(viewMatrix, 0);

        // Set up translation, scale, and rotation matrices
        Matrix.translateM(translationMatrix, 0, 0.0f, 0.0f, -5.0f); // Move object back along z-axis
        Matrix.scaleM(scaleMatrix, 0, 1.0f, 1.0f, 1.0f); // Scale the object

        // Set up the rotation matrices
        Matrix.setRotateM(rotationMatrixX, 0, angle, 1.0f, 0.0f, 0.0f); // Rotate around x-axis
        Matrix.setRotateM(rotationMatrixY, 0, angle, 0.0f, 1.0f, 0.0f); // Rotate around y-axis
        Matrix.setRotateM(rotationMatrixZ, 0, angle, 0.0f, 0.0f, 1.0f); // Rotate around z-axis

        // Combine rotation matrices (Z * Y * X)
        Matrix.multiplyMM(rotationMatrix, 0, rotationMatrixZ, 0, rotationMatrixY, 0);
        Matrix.multiplyMM(rotationMatrix, 0, rotationMatrix, 0, rotationMatrixX, 0);

        // Combine translation, scale, and rotation to form the model matrix
        Matrix.multiplyMM(modelViewMatrix, 0, translationMatrix, 0, scaleMatrix, 0); // Apply translation and scaling
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix, 0); // Apply rotation to modelViewMatrix

        // Assuming perspectiveProjectionMatrix is defined elsewhere
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);

        // Activate and bind the marble texture
        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, marble_texture[0]);
        GLES32.glUniform1i(textureSamplerUniform, 0);

        // Pass matrices to shader
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelViewMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

        // Lights
        if(lightEnabled == true)
        {
            GLES32.glUniform1i(tapUniform, 1);
            
            GLES32.glUniform3fv(lightDiffusedUniform, 1, lightDiffused, 0);
            GLES32.glUniform3fv(lightAmbientUniform, 1, lightAmbient, 0);
            GLES32.glUniform3fv(lightSpecularUniform, 1, lightSpecular, 0);
            GLES32.glUniform4fv(lightPositionUniform, 1, lightPosition, 0);

            GLES32.glUniform3fv(materialDiffusedUniform, 1, materialDiffused, 0);
            GLES32.glUniform3fv(materialAmbientUniform, 1, materialAmbient, 0);
            GLES32.glUniform3fv(materialSpecularUniform, 1, materialSpecular, 0);
            GLES32.glUniform1f(materialShininessUniform, materialShininess);
        }
        else
        {
            GLES32.glUniform1i(tapUniform, 0);
        }

        GLES32.glBindVertexArray(VAO[0]);
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

        if(marble_texture[0] > 0)
        {
            GLES32.glDeleteTextures(1, marble_texture, 0);
            marble_texture[0] = 0;
        }

        if(VBO_TEXCOORD[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_TEXCOORD, 0);
            VBO_TEXCOORD[0] = 0;
        }

        if(VBO_CUBE[0] > 0)
        {
            GLES32.glDeleteBuffers(1, VBO_CUBE, 0);
            VBO_CUBE[0] = 0;
        }

        if(VAO[0] > 0)
        {
            GLES32.glDeleteVertexArrays(1, VAO, 0);
            VAO[0] = 0;
        }

    }
}
