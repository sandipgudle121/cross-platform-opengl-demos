var canvas = null;
var gl = null;
var bFullscreen = false;

var canvas_original_width;
var canvas_original_height;

// WebGL related variables
const vertexAttributeEnum = 
                            {
                                SSG_ATTRIBUTE_POSITION:0,
                                SSG_ATTRIBUTE_NORMAL:1
                            };

var shaderProgramObject = null;

var vao_pyramid = null;
var vbo_pyramid = null;

var angle = 0.0;

// Light uniforms
var ldUniform = null;
var kdUniform = null;
var lightPositionUniform = null;
var keyPressedUniform = null;

var bLightEnabled = false;
var bAnimationEnabled = false;

var modelViewMatrixUniform = null;
var projectionMatrixUniform = null;

var materialDiffused = [ 0.5, 0.5, 0.5,];
var lightDiffused = [ 1.0, 1.0, 1.0 ];
var lightPosition = [ 0.0, 0.0, 2.0, 1.0];


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
            break;
        
        case 70 :
        case 120 :
            toggleFullscreen();
            break;
        
        case 65 :
        case 97 :
            if(bAnimationEnabled == false)
            {
                bAnimationEnabled = true;
            }
            else
            {
                bAnimationEnabled = false;
            }
            break;

        case 76 :
        case 108 :
            if(bLightEnabled == false)
            {
                bLightEnabled = true;
            }
            else
            {
                bLightEnabled = false;
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
                                    /*"oDiffusedLight = vec3(0.0, 0.0, 0.0);\n"+*/
                                    "oDiffusedLight = uLd * uKd * dot(s,n);\n"+
                                    /*"oDiffusedLight = vec3(1.0, 0.0, 0.0);\n"+*/
                                    "}\n"+
                                    "else\n"+
                                    "{\n"+
                                    "oDiffusedLight = vec3(0.0, 0.0, 0.0);\n"+
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
    lightPositionUniform = gl.getUniformLocation(shaderProgramObject, "uLightPosition");
    keyPressedUniform = gl.getUniformLocation(shaderProgramObject, "uKeyPressed");
        

    // Geometry attribute declarations                                   
    // Geometry attribute declarations
    var pyramid_position = new Float32Array([
                                                // front
                                                0.0,  1.0,  0.0, // front-top
                                                -1.0, -1.0,  1.0, // front-left
                                                1.0, -1.0,  1.0, // front-right

                                                // right   
                                                0.0,  1.0,  0.0, // right-top
                                                1.0, -1.0,  1.0, // right-left
                                                1.0, -1.0, -1.0, // right-right

                                                // back
                                                0.0,  1.0,  0.0, // back-top
                                                1.0, -1.0, -1.0, // back-left
                                                -1.0, -1.0, -1.0, // back-right

                                                // left
                                                0.0,  1.0,  0.0, // left-top
                                                -1.0, -1.0, -1.0, // left-left
                                                -1.0, -1.0,  1.0, // left-right
                                           ]); 
    
    var pyramid_normal = new Float32Array ([
                                            // front
	                                        0.000000, 0.447214,  0.894427, // front-top
	                                        0.000000, 0.447214,  0.894427, // front-left
	                                        0.000000, 0.447214,  0.894427, // front-right

	                                        // right			    
	                                        0.894427, 0.447214,  0.000000, // right-top
	                                        0.894427, 0.447214,  0.000000, // right-left
	                                        0.894427, 0.447214,  0.000000, // right-right

	                                        // back
	                                        0.000000, 0.447214, -0.894427, // back-top
	                                        0.000000, 0.447214, -0.894427, // back-left
	                                        0.000000, 0.447214, -0.894427, // back-right

	                                        // left
	                                        -0.894427, 0.447214,  0.000000, // left-top
	                                        -0.894427, 0.447214,  0.000000, // left-left
	                                        -0.894427, 0.447214,  0.000000, // left-right
                                        ]);
    
    // vao square
    vao_pyramid= gl.createVertexArray();
    gl.bindVertexArray(vao_pyramid);
    // Vbo normal
    vbo_normal = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_normal);
    gl.bufferData(gl.ARRAY_BUFFER, pyramid_normal, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_NORMAL);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    
    
    // Vbo poition
    vbo_pyramid = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_pyramid);
    gl.bufferData(gl.ARRAY_BUFFER, pyramid_position, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // Un-bind VAO
    gl.bindVertexArray(null);

    // Enable 
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

    // Cube
    // Transformations
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -5.0]);
    mat4.rotateY(modelViewMatrix, modelViewMatrix, degToRad(angle));
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    // Push above mvp into vertex shaders's mvp uniform
    gl.uniformMatrix4fv(modelViewMatrixUniform, false, modelViewMatrix);
    gl.uniformMatrix4fv(projectionMatrixUniform , false, perspectiveProjectionMatrix);

    if(bLightEnabled == true)
    {
        gl.uniform1i(keyPressedUniform, 1);
        gl.uniform3fv(ldUniform, lightDiffused);
        gl.uniform3fv(kdUniform, materialDiffused);
        gl.uniform4fv(lightPositionUniform, lightPosition);
    }
    else
    {
        gl.uniform1i(keyPressedUniform, 0);
    }

    gl.bindVertexArray(vao_pyramid);
    gl.drawArrays(gl.TRIANGLES, 0, 12);
    gl.bindVertexArray(null);
    gl.useProgram(null);

    // Update 
    if(bAnimationEnabled == true)
    {
        update();
    }

    // Double buffering (swapbuffer)
    requestAnimationFrame(display, canvas);
}

function degToRad(degrees)
{
    return(degrees * Math.PI / 180.0);
}

function update()
{
    // Code
    angle = angle - 1.5;
    if(angle >= 360.0)
    {
        angle = angle + 360.0;
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

    if(vbo_normal)
    {
        gl.deleteBuffer(vbo_normal);
        vbo_normal = null;
    }

    if(vbo_pyramid)
    {
        gl.deleteBuffer(vbo_pyramid);
        vbo_pyramid = null;
    }
    
    if(vao_pyramid)
    {
        gl.deleteVertexArray(vao_pyramid);
        vao_pyramid = null;
    }
}

