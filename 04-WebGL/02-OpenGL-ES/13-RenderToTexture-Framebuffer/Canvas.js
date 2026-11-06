var canvas = null;
var gl = null;
var bFullscreen = false;

var canvas_original_width;
var canvas_original_height;

// WebGL related variables
const vertexAttributeEnum = 
{
    SSG_ATTRIBUTE_POSITION:0,
    SSG_ATTRIBUTE_TEXCOORD:1,
    SSG_ATTRIBUTE_NORMAL:2
};

// Cube related variables
var shaderProgramObject_cube = null;

var vao_cube = null;
var vbo_cube = null;
var vbo_texcoord = null;

var kundali_texture;
var cube_angle = 0.0;

var textureSamplerUniform_cube = 0;
var mvpMatrixUniform_cube;
var perspectiveProjectionMatrix_cube;

// Sphere related variables
var shaderProgramObject_PV = null;
var shaderProgramObject_PF = null;

var sphere = null;
var vbo_normal = null;

var modelMatrixUniform_sphere = null;
var viewMatrixUniform_sphere = null;
var projectionMatrixUniform_sphere = null;

var light_Diffused_Uniform = [];
var light_Ambient_Uniform = [];
var light_Position_Uniform = [];
var light_Specular_Uniform = [];

var material_Diffused_Uniform = null;
var material_Ambient_Uniform = null;
var material_Specular_Uniform = null;
var material_Shininess_Uniform = null;

var keyPressedUniform_sphere = null;

var bLightingEnabled = false;
var light_type = 'v';
var light_angle = 0.0;

var lightPosition_1 = [ 0.0, 0.0, 0.0, 1.0]; 
var lightAmbient_1 = [ 0.0, 0.0, 0.0 ];      
var lightSpecular_1 = [ 1.0, 0.0, 0.0];        
var lightDiffused_1 = [ 1.0, 0.0, 0.0 ];

var lightPosition_2 = [ 0.0, 0.0, 0.0, 1.0]; 
var lightAmbient_2 = [ 0.0, 0.0, 0.0 ];      
var lightSpecular_2 = [ 0.0, 1.0, 0.0];        
var lightDiffused_2 = [ 0.0, 1.0, 0.0 ];

var lightPosition_3 = [ 0.0, 0.0, 0.0, 1.0]; 
var lightAmbient_3 = [ 0.0, 0.0, 0.0 ];      
var lightSpecular_3 = [ 0.0, 0.0, 1.0];        
var lightDiffused_3 = [ 0.0, 0.0, 1.0 ];

var materialDiffused = [ 1.0, 1.0, 1.0 ];   
var materialAmbient = [ 0.0, 0.0, 0.0 ];
var materialSpecular = [ 1.0, 1.0, 1.0 ];
var materialShininess = 128.0;

var perspectiveProjectionMatrix_sphere;

// FBO related variables
var FBO = null;
var RBO = null;
var texture_FBO;
var winWidth;
var winHeight;
var FBO_WIDTH = 512;
var FBO_HEIGHT = 512;
var FBO_Result = false;

var requestAnimationFrame = window.requestAnimationFrame ||
                            window.webkitRequestAnimationFrame ||
                            window.mozRequestAnimationFrame ||
                            window.oRequestAnimationFrame ||
                            window.msRequestAnimationFrame;

// Our main fuction
function main()
{
    // Get canvas
    canvas = document.getElementById("SSG");
    if(canvas == null)
    {
        console.log("Getting gl failed\n");
    }
    else
    {
        console.log("Getting gl succeeded\n");
    }

    // Set canvas width and height for future use
    canvas_original_width = canvas.width;
    canvas_original_height = canvas.height;

    // Register for keyboard events
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
    window.addEventListener("resize", resize_cube, false);

    initialize_cube();
    resize_cube();
    display_cube();
}

function keyDown(event)
{
    // Code
    switch(event.keyCode)
    {
        case 81 :   // Q/q
        case 113 :
            toggleFullscreen();
            break;
        
        case 70 :   // f/F
        case 120 :
            light_type = 'f';
            break;
            
        case 86 :   // V/v
        case 118 :
            light_type = 'v';
            break;
        
        case 69 :   // E/e
        case 101 :
            uninitialize_cube();
            window.close();
        
        case 128 :
        case 76 :
            if(bLightingEnabled == false)
            {
                bLightingEnabled = true;
            }
            else
            {
                bLightingEnabled = false;
            }
            break;
    }
}

function mouseDown()
{
    // Code   
}

function toggleFullscreen()
{
    var fullscreen_element = document.fullscreenElement ||
                             document.webkitFullscreenElement ||
                             document.mozFullScreenElement ||
                             document.msFullscreenElement ||
                             null;

    if(fullscreen_element == null)
    {
        if(canvas.requestFullscreen)
        {
            canvas.requestFullscreen();
        }
        else if(canvas.webkitRequestFullscreen)
        {
            canvas.webkitRequestFullscreen();
        }
        else if(canvas.mozRequestFullScreen)
        {
            canvas.mozRequestFullScreen();
        }
        else if(canvas.msRequestFullscreen)
        {
            canvas.msRequestFullscreen();
        }
        
        bFullscreen = true;
        
    }
    else
    {
        if(document.exitFullscreen)
        {
            document.exitFullscreen();
        }
        else if(document.webkitExitFullscreen)
        {
            document.webkitExitFullscreen();
        }
        else if(document.mozCancelFullScreen)
        {
            document.mozCancelFullScreen();
        }
        else if(document.msExitFullscreen)
        {
            document.msExitFullscreen();
        }

        bFullscreen = false;
    }
}

function initialize_cube() 
{
  
    // Code
    // Get context from above canvas
    gl = canvas.getContext("webgl2");
    if(gl == null)
    {
        console.log("Getting WebGL-2 context failed\n");
    }
    else
    {
        console.log("Getting WebGL-2 context succeeded\n");
    }

    // Set WebGL-2 context's view width and view height proprties
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    // Vertex shader
    var vertexShaderSourceCode = 
    
    "#version 300 es\n"+
    "\n"+
    "in vec4 aPosition;\n"+
    "in vec2 aTexcoord;\n"+
    "out vec2 oTexcoord;\n"+
    "uniform mat4 uMVPMatrix;\n"+
    "void main(void)\n"+
    "{\n"+
    "gl_Position=uMVPMatrix * aPosition;\n"+
    "oTexcoord = aTexcoord;\n"+
    "}\n";
    
    var vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject, vertexShaderSourceCode);
    gl.compileShader(vertexShaderObject);

    if(gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(vertexShaderObject);
        if(error.length > 0)
        {
            var log = "Vertex shader compilation error :" + error;
            alert(log);
            uninitialize_cube();
        }
    }
    else
    {
        console.log("Vertex shader compile successfully for cube\n");
    }

    // Fragment shader
    var fragmentShaderSourceCode = 
    
    "#version 300 es\n"+
    "\n"+
    "precision highp float;"+
    "in vec2 oTexcoord;\n"+
    "uniform sampler2D uTextureSampler;\n"+
    "out vec4 FragColor;\n"+
    "void main(void)\n"+
    "{\n"+
    "FragColor = texture(uTextureSampler, oTexcoord);\n"+
    "}\n";
    
    var fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject, fragmentShaderSourceCode);
    gl.compileShader(fragmentShaderObject);
                                
    if(gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject);
        if(error.length > 0)
        {
            var log = "Fragment shader compilation error :" + error;
            alert(log);
            uninitialize_cube();
        }
    }
    {
        console.log("Fragment shader compile successfully for cube\n");
    }

    // Shader program
    shaderProgramObject_cube = gl.createProgram();
    gl.attachShader(shaderProgramObject_cube, vertexShaderObject);
    gl.attachShader(shaderProgramObject_cube, fragmentShaderObject);
    gl.bindAttribLocation(shaderProgramObject_cube, vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, "aPosition");
    gl.bindAttribLocation(shaderProgramObject_cube, vertexAttributeEnum.SSG_ATTRIBUTE_TEXCOORD, "aTexcoord");
    gl.linkProgram(shaderProgramObject_cube);

    if(gl.getProgramParameter(shaderProgramObject_cube, gl.LINK_STATUS) == false)
    {
        var error = gl.getProgramInfoLog(shaderProgramObject_cube);
        if(error.length > 0)
        {
            var log = "Shader program linking error :" + error;
            alert(log);
            uninitialize_cube();
        }
    }
    else
    {
        console.log("Shader program linked successfully for cube\n");
    }

    // Get shader uniform location 
    mvpMatrixUniform_cube = gl.getUniformLocation(shaderProgramObject_cube, "uMVPMatrix");
    textureSamplerUniform_cube = gl.getUniformLocation(shaderProgramObject_cube, "uTextureSampler");

    // Geometry attribute declarations
    var cube_position = new Float32Array
    ([
        // front
        1.0,  1.0,  1.0, // top-right of front
        -1.0,  1.0,  1.0, // top-left of front
        -1.0, -1.0,  1.0, // bottom-left of front
        1.0, -1.0,  1.0, // bottom-right of front

        // right
        1.0,  1.0, -1.0, // top-right of right
        1.0,  1.0,  1.0, // top-left of right
        1.0, -1.0,  1.0, // bottom-left of right
        1.0, -1.0, -1.0, // bottom-right of right

        // back
        1.0,  1.0, -1.0, // top-right of back
        -1.0,  1.0, -1.0, // top-left of back
        -1.0, -1.0, -1.0, // bottom-left of back
        1.0, -1.0, -1.0, // bottom-right of back

        // left
        -1.0,  1.0,  1.0, // top-right of left
        -1.0,  1.0, -1.0, // top-left of left
        -1.0, -1.0, -1.0, // bottom-left of left
        -1.0, -1.0,  1.0, // bottom-right of left

        // top
        1.0,  1.0, -1.0, // top-right of top
        -1.0,  1.0, -1.0, // top-left of top
        -1.0,  1.0,  1.0, // bottom-left of top
        1.0,  1.0,  1.0, // bottom-right of top

        // bottom
        1.0, -1.0,  1.0, // top-right of bottom
        -1.0, -1.0,  1.0, // top-left of bottom
        -1.0, -1.0, -1.0, // bottom-left of bottom
        1.0, -1.0, -1.0, // bottom-right of bottom 
    ]);

    var texcoord = new Float32Array
    ([
        // front
        1.0, 1.0, // top-right of front
        0.0, 1.0, // top-left of front
        0.0, 0.0, // bottom-left of front
        1.0, 0.0, // bottom-right of front

        // right
        1.0, 1.0, // top-right of right
        0.0, 1.0, // top-left of right
        0.0, 0.0, // bottom-left of right
        1.0, 0.0, // bottom-right of right

        // back
        1.0, 1.0, // top-right of back
        0.0, 1.0, // top-left of back
        0.0, 0.0, // bottom-left of back
        1.0, 0.0, // bottom-right of back

        // left
        1.0, 1.0, // top-right of left
        0.0, 1.0, // top-left of left
        0.0, 0.0, // bottom-left of left
        1.0, 0.0, // bottom-right of left

        // top
        1.0, 1.0, // top-right of top
        0.0, 1.0, // top-left of top
        0.0, 0.0, // bottom-left of top
        1.0, 0.0, // bottom-right of top

        // bottom
        1.0, 1.0, // top-right of bottom
        0.0, 1.0, // top-left of bottom
        0.0, 0.0, // bottom-left of bottom
        1.0, 0.0, // bottom-right of bottom
    ]);
    
    // Vao
    vao_cube= gl.createVertexArray();
    gl.bindVertexArray(vao_cube);

    // Vbo 
    vbo_cube = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_cube);

    gl.bufferData(gl.ARRAY_BUFFER, cube_position, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    
    // Vbo texcoord
    vbo_texcoord = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texcoord);
    gl.bufferData(gl.ARRAY_BUFFER, texcoord, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_TEXCOORD, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_TEXCOORD);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindVertexArray(null);

    // Enable depth
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    // Set clear color
    gl.clearColor(1.0, 1.0, 1.0, 1.0);

    // Initialize projection matrix
    perspectiveProjectionMatrix_cube = mat4.create();
    
    // Call FBO Function
    if(create_FBO(FBO_WIDTH, FBO_HEIGHT) == true)
    {
        FBO_Result = initialize_sphere(FBO_WIDTH, FBO_HEIGHT);
    }

    return(0);
}

function initialize_sphere(textureWidth, textureHeight)
{
    // Code
    // Vertex shader of Per Vertex Light
    var vertexShaderSourceCode_PV = 
    
    "#version 300 es\n"+
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
    "}\n";
    
    var vertexShaderObject_PV = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject_PV, vertexShaderSourceCode_PV);
    gl.compileShader(vertexShaderObject_PV);

    if(gl.getShaderParameter(vertexShaderObject_PV, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(vertexShaderObject_PV);
        if(error.length > 0)
        {
            var log = "Per Vertex light Vertex shader compilation error :" + error;
            alert(log);
            uninitialize();
        }
    }
    else
    {
        console.log("Per vertex light Vertex shader compile successfully for sphere\n");
    }

    // Fragment shader of Per Vertext Light
    var fragmentShaderSourceCode_PV = 
    "#version 300 es\n"+
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
    "}\n";
    
    var fragmentShaderObject_PV = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject_PV, fragmentShaderSourceCode_PV);
    gl.compileShader(fragmentShaderObject_PV);
                                
    if(gl.getShaderParameter(fragmentShaderObject_PV, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject_PV);
        if(error.length > 0)
        {
            var log = "Per vertext light Fragment shader compilation error :" + error;
            alert(log);
            uninitialize();
        }
    }
    else
    {
        console.log("Per vertext light Fragment shader compile successfully for sphere\n");
    }

    // Shader program for Per Vertex Light
    shaderProgramObject_PV = gl.createProgram();
    gl.attachShader(shaderProgramObject_PV, vertexShaderObject_PV);
    gl.attachShader(shaderProgramObject_PV, fragmentShaderObject_PV);
    gl.bindAttribLocation(shaderProgramObject_PV, vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, "aPosition");
    gl.bindAttribLocation(shaderProgramObject_PV, vertexAttributeEnum.SSG_ATTRIBUTE_NORMAL, "aNormal");
    gl.linkProgram(shaderProgramObject_PV);

    if(gl.getProgramParameter(shaderProgramObject_PV, gl.LINK_STATUS) == false)
    {
        var error = gl.getProgramInfoLog(shaderProgramObject_PV);
        if(error.length > 0)
        {
            var log = "Per Vertex Shader program linking error :" + error;
            alert(log);
            uninitialize();
        }
    }
    else
    {
        console.log("Per Vertex Shader program linked successfully for sphere\n");
    }

    // Vertex shader of Per Fragment Light
    var vertexShaderSourceCode_PF = 
    "#version 300 es\n"+
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
    "}\n";
    
    var vertexShaderObject_PF = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject_PF, vertexShaderSourceCode_PF);
    gl.compileShader(vertexShaderObject_PF);

    if(gl.getShaderParameter(vertexShaderObject_PF, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(vertexShaderObject_PF);
        if(error.length > 0)
        {
            var log = " Per Fragment light Vertex shader compilation error :" + error;
            alert(log);
            uninitialize();
        }
    }
    else
    {
        console.log("Per Fragment light Vertex shader compile successfully  for sphere\n");
    }

    // Fragment shader of Per Fragment Light
    var fragmentShaderSourceCode_PF = 
        "#version 300 es\n"+
        "\n"+
        "precision highp float;"+
        "precision highp int;"+
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
        "}\n";
    
    var fragmentShaderObject_PF = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject_PF, fragmentShaderSourceCode_PF);
    gl.compileShader(fragmentShaderObject_PF);
                                
    if(gl.getShaderParameter(fragmentShaderObject_PF, gl.COMPILE_STATUS) == false)
    {
        var error = gl.getShaderInfoLog(fragmentShaderObject_PF);
        if(error.length > 0)
        {
            var log = "Per fragment light Fragment shader compilation error :" + error;
            alert(log);
            uninitialize();
        }
    }
    {
        console.log("Per Fragment light Fragment shader compile successfully for sphere\n");
    }

    // Shader program for Per Fragment Light
    shaderProgramObject_PF = gl.createProgram();
    gl.attachShader(shaderProgramObject_PF, vertexShaderObject_PF);
    gl.attachShader(shaderProgramObject_PF, fragmentShaderObject_PF);
    gl.bindAttribLocation(shaderProgramObject_PF, vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, "aPosition");
    gl.bindAttribLocation(shaderProgramObject_PF, vertexAttributeEnum.SSG_ATTRIBUTE_NORMAL, "aNormal");
    gl.linkProgram(shaderProgramObject_PF);

    if(gl.getProgramParameter(shaderProgramObject_PF, gl.LINK_STATUS) == false)
    {
        var error = gl.getProgramInfoLog(shaderProgramObject_PF);
        if(error.length > 0)
        {
            var log = "Per Fragment Shader program linking error :" + error;
            alert(log);
            uninitialize();
        }
    }
    else
    {
        console.log("Per Fragment Shader program linked successfully for sphere\n");
    }

    
    // Geometry attribute declarations
    sphere = new Mesh();
    makeSphere(sphere, 0.8, 30, 30);

    // Enable depth
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
  
    // Set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    // Initialize projection matrix
    perspectiveProjectionMatrix_sphere = mat4.create();

    resize_sphere(winWidth, winHeight);

    return(true);
}

function create_FBO(textureWidth,textureHeight)
{
    // Code
    var maxRenderBufferSize;
    var maxRenderBufferSize = gl.getParameter(gl.MAX_RENDERBUFFER_SIZE);
    if(maxRenderBufferSize < textureWidth || maxRenderBufferSize < textureHeight) 
    {
        console.log("Texture size exceeds MAX_RENDERBUFFER_SIZE");
        return(false);
    }

    // Create Framebuffer
    FBO = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, FBO);

    // Create Texture for FBO
    texture_FBO = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture_FBO);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, textureWidth, textureHeight, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    // Attach Texture to Framebuffer at Color attachment 0
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture_FBO, 0);

    RBO = gl.createRenderbuffer();
    gl.bindRenderbuffer(gl.RENDERBUFFER, RBO);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, textureWidth, textureHeight);

    // Attach Renderbuffer to Framebuffer for depth attachment
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, RBO);

    // Check Framebuffer status
    var status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
    if(status != gl.FRAMEBUFFER_COMPLETE)
    {
        console.log("Framebuffer status is not complete\n");
        return(false);
    }

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    return(true);
}

function resize_cube()
{
    // Code
    winWidth = canvas.width;
    winHeight = canvas.height;

    if(canvas.height <= 0)
    {
        canvas.height = 1;
    }

    if(bFullscreen == true)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }   
    else
    {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }

    // Set viewport
    gl.viewport(0, 0, canvas.width, canvas.height);

    // Set perspective projection
    mat4.perspective(perspectiveProjectionMatrix_cube, 45.0, parseFloat(canvas.width)/parseFloat(canvas.height), 0.1, 100.0)
}

function resize_sphere()
{
    // Code
    if(canvas.height <= 0)
        canvas.height = 1;
        
    if(bFullscreen == true)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else
    {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }

    // Set viewport
    gl.viewport(0, 0, FBO_WIDTH, FBO_HEIGHT);

    // Set perspective projection
    mat4.perspective(perspectiveProjectionMatrix_sphere, 45.0, parseFloat(FBO_WIDTH)/parseFloat(FBO_HEIGHT), 0.1, 100.0);
}

function display_cube()
{
    // Code
    if (FBO_Result === true) 
    {
        // Bind FBO
        gl.bindFramebuffer(gl.FRAMEBUFFER, FBO);

        // Clear the FBO's attachments (color and depth)
        gl.clearColor(0.0, 0.0, 0.0, 1.0); // Black background for FBO scene
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

        // Render FBO scene (e.g., sphere)
        display_sphere(FBO_WIDTH, FBO_HEIGHT);
        update_sphere();

        // Unbind FBO
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    }

    resize_cube(winWidth, winHeight);

    // Reset bg Color to White
    gl.clearColor(1.0, 1.0, 1.0, 1.0);

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(shaderProgramObject_cube);

    // Transformations
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -5.0]);
    mat4.rotateX(modelViewMatrix, modelViewMatrix, degToRad(cube_angle));
    mat4.rotateY(modelViewMatrix, modelViewMatrix, degToRad(cube_angle));
    mat4.rotateZ(modelViewMatrix, modelViewMatrix, degToRad(cube_angle));
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix_cube, modelViewMatrix);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture_FBO);
    gl.uniform1i(textureSamplerUniform_cube, null);

    gl.uniformMatrix4fv(mvpMatrixUniform_cube, false, modelViewProjectionMatrix);
    gl.bindVertexArray(vao_cube);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
    gl.bindVertexArray(null);
    gl.bindTexture(gl.TEXTURE_2D, null);

    gl.useProgram(null);

    // Update 
    update_cube();

    // Double buffering (swapbuffer)
    requestAnimationFrame(display_cube, canvas);
}

function display_sphere(textureWidth, textureHeight)
{
    // Code
    resize_sphere(textureWidth, textureHeight);

    if(light_type == 'v')
    {
        gl.useProgram(shaderProgramObject_PV);

        // Transformations
        var modelMatrix = mat4.create();
        var viewMatrix = mat4.create();
        var modelViewMatrix = mat4.create();

        // Get shader uniform location for Per Vertex Light
        modelMatrixUniform_sphere = gl.getUniformLocation(shaderProgramObject_PV, "uModelMatrix");
        viewMatrixUniform_sphere = gl.getUniformLocation(shaderProgramObject_PV, "uViewMatrix");
        projectionMatrixUniform_sphere = gl.getUniformLocation(shaderProgramObject_PV, "uProjectionMatrix");

        light_Diffused_Uniform[0] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Diffused[0]");
        light_Ambient_Uniform[0] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Ambient[0]");
        light_Specular_Uniform[0] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Specular[0]");
        light_Position_Uniform[0] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Position[0]");

        light_Diffused_Uniform[1] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Diffused[1]");
        light_Ambient_Uniform[1] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Ambient[1]");
        light_Specular_Uniform[1] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Specular[1]");
        light_Position_Uniform[1] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Position[1]");

        light_Diffused_Uniform[2] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Diffused[2]");
        light_Ambient_Uniform[2] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Ambient[2]");
        light_Specular_Uniform[2] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Specular[2]");
        light_Position_Uniform[2] = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Position[2]");

        material_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uMaterial_Diffused");
        material_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uMaterial_Ambient");
        material_Specular_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uMaterial_Specular");
        material_Shininess_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uMaterial_Shininess");
        
        keyPressedUniform_sphere = gl.getUniformLocation(shaderProgramObject_PV, "uKeyPressed");
        
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -3.0]);
        mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

        gl.uniformMatrix4fv(modelMatrixUniform_sphere, false, modelMatrix);
        gl.uniformMatrix4fv(viewMatrixUniform_sphere, false, viewMatrix);
        gl.uniformMatrix4fv(projectionMatrixUniform_sphere, false, perspectiveProjectionMatrix_sphere);


        if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform_sphere, 1);
            
            gl.uniform3fv(light_Diffused_Uniform[0], lightDiffused_1);
            gl.uniform3fv(light_Ambient_Uniform[0], lightAmbient_1);
            gl.uniform3fv(light_Specular_Uniform[0], lightSpecular_1);
            gl.uniform4fv(light_Position_Uniform[0], lightPosition_1);

            gl.uniform3fv(light_Diffused_Uniform[1], lightDiffused_2);
            gl.uniform3fv(light_Ambient_Uniform[1], lightAmbient_2);
            gl.uniform3fv(light_Specular_Uniform[1], lightSpecular_2);
            gl.uniform4fv(light_Position_Uniform[1], lightPosition_2);

            gl.uniform3fv(light_Diffused_Uniform[2], lightDiffused_3);
            gl.uniform3fv(light_Ambient_Uniform[2], lightAmbient_3);
            gl.uniform3fv(light_Specular_Uniform[2], lightSpecular_3);
            gl.uniform4fv(light_Position_Uniform[2], lightPosition_3);

            gl.uniform3fv(material_Diffused_Uniform, materialDiffused);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess);
        }
        else
        {
            gl.uniform1i(keyPressedUniform_sphere, 0);
        }

        sphere.draw();
        gl.useProgram(null);
    }
    
    if(light_type == 'f')
    {
        gl.useProgram(shaderProgramObject_PF);

        // Transformations
        var modelMatrix = mat4.create();
        var viewMatrix = mat4.create();
        var modelViewMatrix = mat4.create();

        // Get shader uniform location for Per Fragment Light
        modelMatrixUniform_sphere = gl.getUniformLocation(shaderProgramObject_PF, "uModelMatrix");
        viewMatrixUniform_sphere = gl.getUniformLocation(shaderProgramObject_PF, "uViewMatrix");
        projectionMatrixUniform_sphere = gl.getUniformLocation(shaderProgramObject_PF, "uProjectionMatrix");

        light_Diffused_Uniform[0] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Diffused[0]");
        light_Ambient_Uniform[0] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Ambient[0]");
        light_Specular_Uniform[0] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Specular[0]");
        light_Position_Uniform[0] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Position[0]");

        light_Diffused_Uniform[1] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Diffused[1]");
        light_Ambient_Uniform[1] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Ambient[1]");
        light_Specular_Uniform[1] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Specular[1]");
        light_Position_Uniform[1] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Position[1]");

        light_Diffused_Uniform[2] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Diffused[2]");
        light_Ambient_Uniform[2] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Ambient[2]");
        light_Specular_Uniform[2] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Specular[2]");
        light_Position_Uniform[2] = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Position[2]");

        material_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uMaterial_Diffused");
        material_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uMaterial_Ambient");
        material_Specular_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uMaterial_Specular");
        material_Shininess_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uMaterial_Shininess");

        keyPressedUniform_sphere = gl.getUniformLocation(shaderProgramObject_PF, "uKeyPressed");
        
        
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -3.0]);
        mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

        gl.uniformMatrix4fv(modelMatrixUniform_sphere, false, modelMatrix);
        gl.uniformMatrix4fv(viewMatrixUniform_sphere, false, viewMatrix);
        gl.uniformMatrix4fv(projectionMatrixUniform_sphere, false, perspectiveProjectionMatrix_sphere);

        if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform_sphere, 1);

            gl.uniform3fv(light_Diffused_Uniform[0], lightDiffused_1);
            gl.uniform3fv(light_Ambient_Uniform[0], lightAmbient_1);
            gl.uniform3fv(light_Specular_Uniform[0], lightSpecular_1);
            gl.uniform4fv(light_Position_Uniform[0], lightPosition_1);

            gl.uniform3fv(light_Diffused_Uniform[1], lightDiffused_2);
            gl.uniform3fv(light_Ambient_Uniform[1], lightAmbient_2);
            gl.uniform3fv(light_Specular_Uniform[1], lightSpecular_2);
            gl.uniform4fv(light_Position_Uniform[1], lightPosition_2);

            gl.uniform3fv(light_Diffused_Uniform[2], lightDiffused_3);
            gl.uniform3fv(light_Ambient_Uniform[2], lightAmbient_3);
            gl.uniform3fv(light_Specular_Uniform[2], lightSpecular_3);
            gl.uniform4fv(light_Position_Uniform[2], lightPosition_3);

            gl.uniform3fv(material_Diffused_Uniform, materialDiffused);
            gl.uniform3fv(material_Ambient_Uniform,  materialAmbient);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular);
            gl.uniform1f(material_Shininess_Uniform, materialShininess);
        }
        else
        {
            gl.uniform1i(keyPressedUniform_sphere, 0);
        }

        sphere.draw();
        gl.useProgram(null);
    }
}


function degToRad(degrees)
{
    return(degrees * Math.PI / 180.0);
}

function update_cube()
{
    // Code
    cube_angle = cube_angle - 0.1;
    if(cube_angle >= 360.0)
    {
        cube_angle = cube_angle + 360.0;
    }
}

function update_sphere()
{
    // Code
    if(bLightingEnabled == true && light_type == 'v' || light_type == 'f')
    {
        // Red light
        lightPosition_1[0]= 0.0;
        lightPosition_1[1]= 3.0 * Math.cos(light_angle);
        lightPosition_1[2]= 4.0 * Math.sin(light_angle);
        lightPosition_1[3]= 1.0;

        light_angle = light_angle + 0.005;
        if(light_angle > 360.0)
        {
            light_angle = light_angle - 360.0;
        }

        // Green light
        lightPosition_2[0]= 5.0 * Math.sin(light_angle);
        lightPosition_2[1]= 0.0;
        lightPosition_2[2]= 3.0 * Math.cos(light_angle);
        lightPosition_2[3]= 1.0;

        light_angle = light_angle + 0.005;
        if(light_angle > 360.0)
        {
            light_angle = light_angle - 360.0;
        }

        // Blue light
        lightPosition_3[0]= 3.0 * Math.cos(light_angle);
        lightPosition_3[1]= 6.0 * Math.sin(light_angle);
        lightPosition_3[2]= 0.0;
        lightPosition_3[3]= 1.0;

        light_angle = light_angle + 0.005;
        if(light_angle > 360.0)
        {
            light_angle = light_angle - 360.0;
        }
    }
}

function uninitialize_cube()
{
    // Code
    if(shaderProgramObject_cube)
    {
        gl.useProgram(shaderProgramObject_cube);

        var shaderObjects = gl.getAttachedShaders(shaderProgramObject_cube);
        if(shaderObjects && shaderObjects.length > 0)
        {
            for(let i = 0; i < shaderObjects.length; i++)
            {
                gl.detachShader(shaderProgramObject_cube, shaderObjects[i]);
                gl.deleteShader(shaderObjects[i]);
                shaderObjects[i] = null;
            }
        }

        gl.useProgram(null);
        gl.deleteProgram(shaderProgramObject_cube);
        shaderProgramObject_cube = null;
    }

    if(vbo_texcoord)
    {
        gl.deleteBuffer(vbo_texcoord);
        vbo_texcoord = null;
    }

    if(vbo_cube)
    {
        gl.deleteBuffer(vbo_cube);
        vbo_cube = null;
    }
        
    if(vao_cube)
    {
        gl.deleteVertexArray(vao_cube);
        vao_cube = null;
    }

    if(texture_FBO)
    {
        gl.deleteTexture(texture_FBO);
        texture_FBO = null;
    }

    if(RBO)
    {
        gl.deleteRenderbuffer(RBO);
        RBO = null;
    }

    if(FBO)
    {
        gl.deleteFramebuffer(FBO);
        FBO = null;
    }
}

function uninitialize_sphere()
{
    // Code
    if(light_type == 'v')
    {
        if(shaderProgramObject_PV)
        {
            gl.useProgram(shaderProgramObject_PV);
    
            var shaderObjects = gl.getAttachedShaders(shaderProgramObject_PV);
            if(shaderObjects && shaderObjects.length > 0)
            {
                for(let i = 0; i < shaderObjects.length; i++)
                {
                    gl.detachShader(shaderProgramObject_PV, shaderObjects[i]);
                    gl.deleteShader(shaderObjects[i]);
                    shaderObjects[i] = null;
                }
            }
    
            gl.useProgram(null);
            gl.deleteProgram(shaderProgramObject_PV);
            shaderProgramObject_PV = null;
        }
    }

    if(light_type == 'f')
    {
        if(shaderProgramObject_PF)
        {
            gl.useProgram(shaderProgramObject_PF);
    
            var shaderObjects = gl.getAttachedShaders(shaderProgramObject_PF);
            if(shaderObjects && shaderObjects.length > 0)
            {
                for(let i = 0; i < shaderObjects.length; i++)
                {
                    gl.detachShader(shaderProgramObject_PF, shaderObjects[i]);
                    gl.deleteShader(shaderObjects[i]);
                    shaderObjects[i] = null;
                }
            }
    
            gl.useProgram(null);
            gl.deleteProgram(shaderProgramObject_PF);
            shaderProgramObject_PF = null;
        }
    }

    if(sphere)
    {
        sphere.deallocate();
        sphere = null;
    }

    if(vbo_normal)
    {
        gl.deleteBuffer(vbo_normal);
        vbo_normal = null;
    }
}