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

var shaderProgramObject = null;

var sphere = null;
var vbo_normal = null;

var modelMatrixUniform = null;
var viewMatrixUniform = null;
var projectionMatrixUniform = null;

var light_Diffused_Uniform = null;
var light_Ambient_Uniform = null;
var light_Position_Uniform = null;
var light_Specular_Uniform = null;

var material_Diffused_Uniform = null;
var material_Ambient_Uniform = null;
var material_Specular_Uniform = null;
var material_Shininess_Uniform = null;

var keyPressedUniform = null;

var bLightingEnabled = false;

var lightPosition = [ 100.0, 100.0, 100.0, 1.0];
var lightAmbient = [ 0.1, 0.1, 0.1 ];
var lightSpecular = [ 1.0, 1.0, 1.0 ];
var lightDiffused = [ 1.0, 1.0, 1.0];

var materialDiffuse = [ 0.5, 0.2, 0.7 ];
var materialAmbient = [ 0.0, 0.0, 0.0 ];
var materialSpecular = [ 0.7, 0.7, 0.7 ];
var materialShininess = 50.0;

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
                                "precision highp float;"+
                                "precision highp int;"+
                                "in vec4 aPosition;\n"+
                                "in vec3 aNormal;\n"+
                                "\n"+
                                "uniform mat4 uViewMatrix;\n"+
                                "uniform mat4 uModelMatrix;\n"+
                                "uniform mat4 uProjectionMatrix;\n"+
                                "\n"+
                                "uniform vec3 uLight_Ambient;\n"+
                                "uniform vec3 uLight_Specular;\n"+
                                "uniform vec3 uLight_Diffused;\n"+
                                "uniform vec4 uLight_Position;\n"+
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
                                "vec3 light_Direction = normalize(vec3(uLight_Position - eye_Coordinates));\n"+
                                "vec3 reflection_Vector = reflect(-light_Direction, transformed_Normals);\n"+
                                "vec3 viewer_Vector = normalize(-eye_Coordinates.xyz);\n"+
                                "vec3 lightAmbient = uLight_Ambient * uMaterial_Ambient;\n"+
                                "vec3 lightDiffused = uLight_Diffused * uMaterial_Diffused * max(dot(light_Direction, transformed_Normals), 0.0);\n"+
                                "vec3 lightSpecular = uLight_Specular * uMaterial_Specular * pow(max(dot(reflection_Vector, viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
                                "oPhong_ADS_Light = lightAmbient + lightDiffused + lightSpecular;\n"+
                                "}\n"+
                                "else\n"+
                                "{\n"+
                                "oPhong_ADS_Light = vec3(0.0f, 0.0f, 0.0f);\n"+
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

    // Get shader uniform location 
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
    makeSphere(sphere, 0.8, 30, 30);

    // Enable depth
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
  
    // Set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

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
    // Code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(shaderProgramObject);

    // Transformations
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
    
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -3.0]);
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

        gl.uniform3fv(material_Diffused_Uniform, materialDiffuse);
        gl.uniform3fv(material_Ambient_Uniform, materialAmbient);
        gl.uniform3fv(material_Specular_Uniform, materialSpecular);
        gl.uniform1f(material_Shininess_Uniform,  materialShininess);
    }
    else
    {
        gl.uniform1i(keyPressedUniform, 0);
    }

    sphere.draw();
    gl.useProgram(null);

    // Double buffering (swapbuffer)
    requestAnimationFrame(display, canvas);
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

