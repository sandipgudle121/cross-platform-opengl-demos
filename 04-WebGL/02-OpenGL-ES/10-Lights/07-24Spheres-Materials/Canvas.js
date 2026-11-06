var canvas = null;
var gl = null;
var bFullscreen = false;

var canvas_original_width;
var canvas_original_height;

// WebGL related variables
const vertexAttributeEnum = 
                            {
                                SSG_ATTRIBUTE_POSITION:0,
                                SSG_ATTRIBUTE_NORMAL:1,
                                SSG_ATTRIBUTE_TEXCOORD:2
                            };
var shaderProgramObject_PV = null;
var shaderProgramObject = null;

var sphere = null;

var vbo_normal = null;

var modelMatrixUniform = null;
var viewMatrixUniform = null;
var projectionMatrixUniform = null;

var light_Diffused_Uniform = [];
var light_Ambient_Uniform = [];
var light_Position_Uniform = [];
var light_Specular_Uniform = [];

var material_Diffused_Uniform = null;
var material_Ambient_Uniform = null;
var material_Specular_Uniform = null;
var material_Shininess_Uniform = null;

var keyPressedUniform = null;

var bLightingEnabled = false;
var angle = 0.0;
var SPEED = 0.04;

var X_ROTATION = false;
var Y_ROTATION = false;
var Z_ROTATION = false;

var lightPosition = [ 0.0, 0.0, 0.0, 1.0]; 
var lightAmbient = [ 0.0, 0.0, 0.0 ];      
var lightSpecular = [ 1.0, 1.0, 1.0];        
var lightDiffused = [ 1.0, 1.0, 1.0 ];

var perspectiveProjectionMatrix;

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
    window.addEventListener("resize", resize, false);

    initialize();
    resize();
    display();
}

function keyDown(event)
{
    // Code
    switch(event.keyCode)
    {
        case 81 :
        case 113 :
            uninitialize();
            window.close();
        
        case 70 :
        case 120 :
            toggleFullscreen();
            break;

        case 108 :      // L
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
        
        case 120: // X
        case 88: // x
            if (!X_ROTATION) 
            {
                X_ROTATION = true;
                Y_ROTATION = false;
                Z_ROTATION = false;
            } 
            else 
            {
                X_ROTATION = false;
            }
            break;

        case 121: // Y
        case 89: // y
            if (!Y_ROTATION) 
            {
                Y_ROTATION = true;
                X_ROTATION = false;
                Z_ROTATION = false;
            } 
            else 
            {
                Y_ROTATION = false;
            }
            break;

        case 122: // Z
        case 90: // z
            if (!Z_ROTATION) 
            {
                Z_ROTATION = true;
                X_ROTATION = false;
                Y_ROTATION = false;
            } 
            else 
            {
                Z_ROTATION = false;
            }
            break;
    }
}

function mouseDown()
{
    
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

function initialize()
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
        "oTransformed_Normals = vec3(0.0, 0.0, 0.0);\n"+
        "oLight_Direction = vec3(0.0, 0.0, 0.0);\n"+
        "oViewer_Vector = vec3(0.0, 0.0, 0.0);\n"+
        "}\n"+
        "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;\n"+
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
            uninitialize();
        }
    }
    else
    {
        console.log("Vertex shader compile successfully\n");
    }

    // Fragment shader
    var fragmentShaderSourceCode = 
        "#version 300 es\n"+
        "\n"+
        "precision highp float;"+
        "precision highp int;"+
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
        "phong_ADS_Light = vec3(1.0, 1.0, 1.0);\n"+
        "}\n"+
        "FragColor = vec4(phong_ADS_Light, 1.0);\n"+
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
            uninitialize();
        }
    }
    {
        console.log("Fragment shader compile successfully\n");
    }

    // Shader program
    shaderProgramObject = gl.createProgram();
    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);
    gl.bindAttribLocation(shaderProgramObject, vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, "aPosition");
    gl.bindAttribLocation(shaderProgramObject, vertexAttributeEnum.SSG_ATTRIBUTE_NORMAL, "aNormal");
    gl.linkProgram(shaderProgramObject);

    if(gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS) == false)
    {
        var error = gl.getProgramInfoLog(shaderProgramObject);
        if(error.length > 0)
        {
            var log = "Shader program linking error :" + error;
            alert(log);
            uninitialize();
        }
    }
    else
    {
        console.log("Shader program linked successfully\n");
    }

    // Get shader uniform location for Per Fragment Light
    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uProjectionMatrix");

    light_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject, "uLight_Diffused");
    light_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject, "uLight_Ambient");
    light_Specular_Uniform = gl.getUniformLocation(shaderProgramObject, "uLight_Specular");
    light_Position_Uniform = gl.getUniformLocation(shaderProgramObject, "uLight_Position");

    material_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject, "uMaterial_Diffused");
    material_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject, "uMaterial_Ambient");
    material_Specular_Uniform = gl.getUniformLocation(shaderProgramObject, "uMaterial_Specular");
    material_Shininess_Uniform = gl.getUniformLocation(shaderProgramObject, "uMaterial_Shininess");

    keyPressedUniform = gl.getUniformLocation(shaderProgramObject, "uKeyPressed");
    
    // Geometry attribute declarations
    sphere = new Mesh();
    makeSphere(sphere, 0.8, 100, 100);

    // Enable depth
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    // Set clear color
    gl.clearColor(0.3, 0.3, 0.3, 1.0);

    // Initialize projection matrix
    perspectiveProjectionMatrix = mat4.create();
}

function resize()
{
    // Code
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
    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width)/parseFloat(canvas.height), 0.1, 100.0)
}

function display()
{
    // Local variables
    var materialDiffused_1 = [0.7568, 0.61424, 0.07568];
    var materialAmbient_1 = [0.0215, 0.1745, 0.0215];
    var materialSpecular_1 = [0.633, 0.727811, 0.633];
    var materialShininess_1 = 0.06 * 128;

    var materialDiffused_2 = [0.54, 0.89, 0.63];
    var materialAmbient_2 = [0.135, 0.2225, 0.1573];
    var materialSpecular_2 = [0.316228, 0.316228, 0.316228];
    var materialShininess_2 = 0.1 * 128;

    var materialDiffused_3 = [0.18275, 0.17, 0.22525];
    var materialAmbient_3 = [0.05375, 0.05, 0.06625];
    var materialSpecular_3 = [0.332741, 0.328634, 0.346435];
    var materialShininess_3 = 0.3 * 128;

    var materialDiffused_4 = [1.0, 0.829, 0.829];
    var materialAmbient_4 = [0.25, 0.20725, 0.20725];
    var materialSpecular_4 = [0.296648, 0.296648, 0.296648];
    var materialShininess_4 = 0.088 * 128;

    var materialDiffused_5 = [0.61424, 0.04136, 0.04136];
    var materialAmbient_5 = [0.1745, 0.01175, 0.01175];
    var materialSpecular_5 = [0.727811, 0.626959, 0.626959];
    var materialShininess_5 = 0.6 * 128;

    var materialDiffused_6 = [0.396, 0.74151, 0.69102];
    var materialAmbient_6 = [0.1, 0.18725, 0.1745];
    var materialSpecular_6 = [0.297554, 0.30829, 0.306668];
    var materialShininess_6 = 0.1 * 128;

    var materialDiffused_7 = [0.780392, 0.568627, 0.113725];
    var materialAmbient_7 = [0.329412, 0.223529, 0.027451];
    var materialSpecular_7 = [0.992157, 0.941176, 0.807843];
    var materialShininess_7 = 0.21794872 * 128;

    var materialDiffused_8 = [0.714, 0.4284, 0.18144];
    var materialAmbient_8 = [0.2125, 0.1275, 0.054];
    var materialSpecular_8 = [0.393548, 0.271906, 0.166721];
    var materialShininess_8 = 0.2 * 128;

    var materialDiffused_9 = [0.4, 0.4, 0.4];
    var materialAmbient_9 = [0.25, 0.25, 0.25];
    var materialSpecular_9 = [0.774597, 0.774597, 0.774597];
    var materialShininess_9 = 0.6 * 128;

    var materialDiffused_10 = [0.7038, 0.27048, 0.0828];
    var materialAmbient_10 = [0.19125, 0.0735, 0.0225];
    var materialSpecular_10 = [0.256777, 0.137622, 0.086014];
    var materialShininess_10 = 0.1 * 128;

    var materialDiffused_11 = [0.75164, 0.60648, 0.22648];
    var materialAmbient_11 = [0.24725, 0.1995, 0.0745];
    var materialSpecular_11 = [0.608281, 0.555802, 0.366065];
    var materialShininess_11 = 0.4 * 128;

    var materialDiffused_12 = [0.50754, 0.50754, 0.50754];
    var materialAmbient_12 = [0.19225, 0.19225, 0.19225];
    var materialSpecular_12 = [0.508273, 0.508273, 0.508273];
    var materialShininess_12 = 0.4 * 128;

    var materialDiffused_13 = [0.01, 0.01, 0.01];
    var materialAmbient_13 = [0.0, 0.0, 0.0];
    var materialSpecular_13 = [0.5, 0.5, 0.5];
    var materialShininess_13 = 0.25 * 128;

    var materialDiffused_14 = [0.0, 0.50980392, 0.50980392];
    var materialAmbient_14 = [0.0, 0.1, 0.06];
    var materialSpecular_14 = [0.50196078, 0.50196078, 0.50196078];
    var materialShininess_14 = 0.25 * 128;

    var materialDiffused_15 = [0.1, 0.35, 0.1];
    var materialAmbient_15 = [0.0, 0.0, 0.0];
    var materialSpecular_15 = [0.45, 0.45, 0.45];
    var materialShininess_15 = 0.25 * 128;

    var materialDiffused_16 = [0.5, 0.0, 0.0];
    var materialAmbient_16 = [0.0, 0.0, 0.0];
    var materialSpecular_16 = [0.7, 0.6, 0.6];
    var materialShininess_16 = 0.25 * 128;

    var materialDiffused_17 = [0.55, 0.55, 0.55];
    var materialAmbient_17 = [0.0, 0.0, 0.0];
    var materialSpecular_17 = [0.7, 0.7, 0.7];
    var materialShininess_17 = 0.25 * 128;

    var materialDiffused_18 = [0.5, 0.5, 0.0];
    var materialAmbient_18 = [0.0, 0.0, 0.0];
    var materialSpecular_18 = [0.6, 0.6, 0.5];
    var materialShininess_18 = 0.25 * 128;

    var materialDiffused_19 = [0.01, 0.01, 0.01];
    var materialAmbient_19 = [0.02, 0.02, 0.02];
    var materialSpecular_19 = [0.4, 0.4, 0.4];
    var materialShininess_19 = 0.078125 * 128;

    var materialDiffused_20 = [0.4, 0.5, 0.5];
    var materialAmbient_20 = [0.0, 0.05, 0.05];
    var materialSpecular_20 = [0.04, 0.7, 0.7];
    var materialShininess_20 = 0.078125 * 128;

    var materialDiffused_21 = [0.4, 0.5, 0.4];
    var materialAmbient_21 = [0.0, 0.05, 0.0];
    var materialSpecular_21 = [0.04, 0.7, 0.04];
    var materialShininess_21 = 0.078125 * 128;

    var materialDiffused_22 = [0.5, 0.4, 0.4];
    var materialAmbient_22 = [0.02, 0.0, 0.0];
    var materialSpecular_22 = [0.7, 0.04, 0.04];
    var materialShininess_22 = 0.078125 * 128;

    var materialDiffused_23 = [0.5, 0.5, 0.5];
    var materialAmbient_23 = [0.05, 0.05, 0.05];
    var materialSpecular_23 = [0.7, 0.7, 0.7];
    var materialShininess_23 = 0.078125 * 128;

    var materialDiffused_24 = [0.5, 0.5, 0.4];
    var materialAmbient_24 = [0.05, 0.05, 0.0];
    var materialSpecular_24 = [0.7, 0.7, 0.04];
    var materialShininess_24 = 0.078125 * 128;
  
    // Code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    // Calculate the width and height for each viewport
    var viewportWidth = canvas.width / 4;
    var viewportHeight = canvas.height / 6;
    
    // Sphere 1
    gl.useProgram(shaderProgramObject);
    gl.viewport( 60.0 + 0, 5 * viewportHeight, viewportWidth/1.5, viewportHeight);
    

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
    {
        gl.uniform1i(keyPressedUniform, 1);
        
        gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
        gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
        gl.uniform3fv(light_Specular_Uniform, lightSpecular);
        gl.uniform4fv(light_Position_Uniform, lightPosition);

        gl.uniform3fv(material_Diffused_Uniform, materialDiffused_1);
        gl.uniform3fv(material_Ambient_Uniform, materialAmbient_1);
        gl.uniform3fv(material_Specular_Uniform, materialSpecular_1);
        gl.uniform1f(material_Shininess_Uniform,  materialShininess_1);
    }
    else
    {
        gl.uniform1i(keyPressedUniform, 0);
    }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 2
    gl.useProgram(shaderProgramObject);
    gl.viewport(60.0 + viewportWidth, 5 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_2);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_2);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_2);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_2);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 3
    gl.useProgram(shaderProgramObject);
    gl.viewport(60.0 + 2 * viewportWidth, 5 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_3);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_3);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_3);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_3);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);


    // Sphere 4
    gl.useProgram(shaderProgramObject);
    gl.viewport(60.0 + 3 * viewportWidth, 5 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_4);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_4);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_4);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_4);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Sphere 5
    gl.useProgram(shaderProgramObject);

    // Viewport 5 (row 2, col 1)
    gl.viewport(60.0 + 0, 4 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_5);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_5);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_5);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_5);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 6
    gl.useProgram(shaderProgramObject);

    // Viewport 6 (row 2, col 2)
    gl.viewport(60.0 + viewportWidth, 4 * viewportHeight, viewportWidth/1.5, viewportHeight);;

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_6);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_6);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_6);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_6);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 7
    gl.useProgram(shaderProgramObject);

    // Viewport 7 (row 2, col 3)
    gl.viewport(60.0 + 2 * viewportWidth, 4 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_7);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_7);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_7);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_7);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);


    // Sphere 8
    gl.useProgram(shaderProgramObject);

    // Viewport 8 (row 2, col 4)
    gl.viewport(60.0 + 3 * viewportWidth, 4 * viewportHeight, viewportWidth/1.5, viewportHeight);;

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_8);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_8);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_8);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_8);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Sphere 9
    gl.useProgram(shaderProgramObject);

    // Viewport 9 (row 3, col 1)
    gl.viewport(60.0 + 0, 3 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_9);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_9);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_9);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_9);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 10
    gl.useProgram(shaderProgramObject);

    // Viewport 10 (row 3, col 2)
    gl.viewport(60.0 + viewportWidth, 3 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_10);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_10);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_10);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_10);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 11
    gl.useProgram(shaderProgramObject);

    // Viewport 11 (row 3, col 3)
    gl.viewport(60.0 + 2 * viewportWidth, 3 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_11);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_11);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_11);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_11);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);


    // Sphere 12
    gl.useProgram(shaderProgramObject);

    // Viewport 12 (row 3, col 4)
    gl.viewport(60.0 + 3 * viewportWidth, 3 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_12);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_12);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_12);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_12);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Sphere 13
    gl.useProgram(shaderProgramObject);

    // Viewport 13 (row 4, col 1)
    gl.viewport(60.0 + 0, 2 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_13);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_13);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_13);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_13);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 14
    gl.useProgram(shaderProgramObject);

    // Viewport 14 (row 4, col 2)
    gl.viewport(60.0 + viewportWidth, 2 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_14);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_14);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_14);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_14);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 15
    gl.useProgram(shaderProgramObject);

    // Viewport 15 (row 4, col 3)
    gl.viewport(60.0 + 2 * viewportWidth, 2 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_15);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_15);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_15);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_15);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);


    // Sphere 16
    gl.useProgram(shaderProgramObject);

    // Viewport 16 (row 4, col 4)
    gl.viewport(60.0 + 3 * viewportWidth, 2 * viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_16);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_16);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_16);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_16);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Sphere 17
    gl.useProgram(shaderProgramObject);

    // Viewport 17 (row 5, col 1)
    gl.viewport(60.0 + 0, viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_17);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_17);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_17);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_17);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 18
    gl.useProgram(shaderProgramObject);

    // Viewport 18 (row 5, col 2)
    gl.viewport(60.0 + viewportWidth, viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_18);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_18);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_18);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_18);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 19
    gl.useProgram(shaderProgramObject);

    // Viewport 19 (row 5, col 3)
    gl.viewport(60.0 + 2 * viewportWidth, viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_19);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_19);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_19);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_19);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);


    // Sphere 20
    gl.useProgram(shaderProgramObject);

    // Viewport 20 (row 5, col 4)
    gl.viewport(60.0 + 3 * viewportWidth, viewportHeight, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_20);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_20);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_20);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_20);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Sphere 21
    gl.useProgram(shaderProgramObject);

    // Viewport 21 (row 6, col 1)
    gl.viewport(60.0 + 0, 0, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_21);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_21);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_21);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_21);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 22
    gl.useProgram(shaderProgramObject);

    // Viewport 22 (row 6, col 2)
    gl.viewport(60.0 + viewportWidth, 0, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_22);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_22);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_22);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_22);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Sphere 23
    gl.useProgram(shaderProgramObject);

    // Viewport 23 (row 6, col 3)
    gl.viewport(60.0 + 2 * viewportWidth, 0, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_23);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_23);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_23);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_23);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);


    // Sphere 24
    gl.useProgram(shaderProgramObject);

    // Viewport 24 (row 6, col 4)
    gl.viewport(60.0 + 3 * viewportWidth, 0, viewportWidth/1.5, viewportHeight);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
  
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);

    gl.uniformMatrix4fv(modelMatrixUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewMatrixUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
        {
            gl.uniform1i(keyPressedUniform, 1);
            
            gl.uniform3fv(light_Diffused_Uniform, lightDiffused);
            gl.uniform3fv(light_Ambient_Uniform, lightAmbient);
            gl.uniform3fv(light_Specular_Uniform, lightSpecular);
            gl.uniform4fv(light_Position_Uniform, lightPosition);
    
            gl.uniform3fv(material_Diffused_Uniform, materialDiffused_24);
            gl.uniform3fv(material_Ambient_Uniform, materialAmbient_24);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular_24);
            gl.uniform1f(material_Shininess_Uniform,  materialShininess_24);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

    sphere.draw();
    gl.useProgram(null);

    // Update 
    update();

    // Double buffering (swapbuffer)
    requestAnimationFrame(display, canvas);
}

function degToRad(degrees)
{
    return(degrees * Math.PI / 180.0);
}

function update()
{
    // Variables

    // Code
    if(X_ROTATION == true)
    {
        // X rotation
        lightPosition[0]= 0.0;
        lightPosition[1]= 12.0 * Math.cos(angle);
        lightPosition[2]= 15.0 * Math.sin(angle);
        lightPosition[3]= 1.0;

        angle = angle + SPEED;
        if(angle > 360.0)
        {
            angle = angle - 360.0;
        }
    }

    if(Y_ROTATION == true)
    {
        // Y rotation
        lightPosition[0]= 20.0 * Math.sin(angle);
        lightPosition[1]= 0.0;
        lightPosition[2]= 20.0 * Math.cos(angle);
        lightPosition[3]= 1.0;

        angle = angle + SPEED;
        if(angle > 360.0)
        {
            angle = angle - 360.0;
        }
    }

    if(Z_ROTATION == true)
    {
        // Z rotation
        lightPosition[0]= 20.0 * Math.cos(angle);
        lightPosition[1]= 20.0 * Math.sin(angle);
        lightPosition[2]= 0.0;
        lightPosition[3]= 1.0;

        angle = angle + SPEED;
        if(angle > 360.0)
        {
            angle = angle - 360.0;
        }
    }
}

function uninitialize()
{
    // Code
    if(shaderProgramObject)
    {
        gl.useProgram(shaderProgramObject);

        var shaderObjects = gl.getAttachedShaders(shaderProgramObject);
        if(shaderObjects && shaderObjects.length > 0)
        {
            for(let i = 0; i < shaderObjects.length; i++)
            {
                gl.detachShader(shaderProgramObject, shaderObjects[i]);
                gl.deleteShader(shaderObjects[i]);
                shaderObjects[i] = null;
            }
        }

        gl.useProgram(null);
        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }

    if(sphere)
	{
		sphere.deallocate();
		sphere = null;
	}
}

