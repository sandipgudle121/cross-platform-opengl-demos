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

var modelViewMatrixUniform = null;
var projectionMatrixUniform = null;
var ldUniform = null;
var kdUniform = null;
var lightPositionUniform = null;
var keyPressedUniform = null;

var bLightingEnabled = false;


// Material properties
var materialDiffuse = [0.5, 0.5, 0.5];
var lightDiffuse = [1.0, 1.0, 1.0 ];
var lightPosition = [0.0, 0.0, 2.0, 1.0];

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
    modelViewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uModelViewMatrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uProjectionMatrix");
    ldUniform = gl.getUniformLocation(shaderProgramObject, "uLd");
    kdUniform = gl.getUniformLocation(shaderProgramObject, "uKd");
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "uLightposition");
    keyPressedUniform = gl.getUniformLocation(shaderProgramObject, "uKeyPressed");
    
    // Geometry attribute declarations
    sphere = new Mesh();
    makeSphere(sphere, 0.8, 100, 100);

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
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(modelViewMatrixUniform, false, modelViewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform, false, perspectiveProjectionMatrix);

    if(bLightingEnabled == true)
    {
        gl.uniform1i(keyPressedUniform, 1);
        gl.uniform3fv(ldUniform, lightDiffuse);
        gl.uniform3fv(kdUniform, materialDiffuse);
        gl.uniform4fv(lightPositionUniform, lightPosition);
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

