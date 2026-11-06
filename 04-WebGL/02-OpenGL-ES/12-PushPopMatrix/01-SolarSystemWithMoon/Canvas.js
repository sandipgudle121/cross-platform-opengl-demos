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
                                SSG_ATTRIBUTE_TEXCOORD:2,
                                SSG_ATTRIBUTE_COLOR: 3
                            };

var shaderProgramObject = null;

var sphere = null;

let matrixStack = [];

var year = 0;
var day = 0;
var hour = 0;

var mvpMatrixUniform;
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
        case 81 :   // Esc
        case 113 :
            uninitialize();
            window.close();
        
        case 70 :   // F
        case 120 :  // f
            toggleFullscreen();
            break;

        case 68: 
            if(event.shiftKey) // Uppercase 'D'
            { 
                day = (day + 6) % 360;
                hour = (hour + 3) % 360;
            } 
            else // Lowercase 'd'
            { 
                day = (day - 6) % 360;
                hour = (hour - 3) % 360;
            }
            break;
    
        case 89:
            if(event.shiftKey) // Uppercase 'Y'
            { 
                year = (year + 3) % 360;
                day = (day + 6) % 360;
            } 
            else // Lowercase 'y'
            { 
                year = (year - 3) % 360;
                day = (day - 6) % 360;
            }
            break;
    
        case 77:
            if(event.shiftKey) // Uppercase 'M'
            { 
                hour = (hour - 3) % 360;
            } 
            else // Lowercase 'm'
            { 
                hour = (hour + 3) % 360;
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
    "in vec4 aPosition;\n"+
    "in vec4 aColor;\n"+
    "out vec4 oColor;\n"+
    "uniform mat4 uMVPMatrix;\n"+
    "void main(void)\n"+
    "{\n"+
    "gl_Position=uMVPMatrix * aPosition;\n"+
    "oColor = aColor;\n"+
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
    "precision highp  float;"+
    "in vec4 oColor;\n"+
    "out vec4 FragColor;\n"+
    "void main(void)\n"+
    "{\n"+
    "FragColor = oColor;\n"+
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
    gl.bindAttribLocation(shaderProgramObject, vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, "aColor");
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
    mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uMVPMatrix");

    // Geometry attribute declarations
    sphere = new Mesh();
    makeSphere(sphere, 0.8, 50, 50);


    // Enable depth
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    //gl.enable(gl.CULL_FACE);

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
    // Clear buffers
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(shaderProgramObject);

    // Transformation
    let modelMatrix = mat4.create();
    let viewMatrix = mat4.create();
    let scaleMatrix = mat4.create(); 

    // --------------------- SUN ---------------------------------------------
    pushMatrix(modelMatrix);  // Save the current matrix state for the Sun

    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);
    mat4.scale(scaleMatrix, mat4.create(), [1.0, 1.0, 1.0]);
    mat4.multiply(modelMatrix, modelMatrix, scaleMatrix);

    // Multiply with perspective matrix
    mat4.multiply(viewMatrix, perspectiveProjectionMatrix, modelMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, viewMatrix);
    gl.vertexAttrib3f(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 1.0, 0.5, 0.0);
    sphere.draw();

    popMatrix(); // Restore the matrix state

    // ----------------------- EARTH ------------------------------------------
    pushMatrix(modelMatrix);  // Save the current matrix state for Earth

    // Earth orbits around the Sun
    mat4.rotate(modelMatrix, modelMatrix, degToRad(year), [0.0, 1.0, 0.0]); 
    mat4.translate(modelMatrix, modelMatrix, [2.0, 0.0, 0.0]);

    // Rotate Earth around its own axis (self-rotation)
    mat4.rotate(modelMatrix, modelMatrix, degToRad(day), [0.0, 1.0, 0.0]);
    mat4.scale(scaleMatrix, mat4.create(), [0.6, 0.6, 0.6]);
    mat4.multiply(modelMatrix, modelMatrix, scaleMatrix);

    // Multiply with perspective matrix
    mat4.multiply(viewMatrix, perspectiveProjectionMatrix, modelMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, viewMatrix);
    gl.vertexAttrib3f(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 0.4, 0.9, 1.0);
    sphere.draw_earth();

    popMatrix(); // Restore the matrix state

    // --------------------------- MOON --------------------------------------
    pushMatrix(modelMatrix);  // Save the current matrix state for Moon

    // Moon orbits around the Earth
    mat4.rotate(modelMatrix, modelMatrix, degToRad(hour), [0.0, 1.0, 0.0]);
    mat4.translate(modelMatrix, modelMatrix, [1.5, 0.0, 0.0]);
    mat4.scale(scaleMatrix, mat4.create(), [0.4, 0.4, 0.4]);
    mat4.multiply(modelMatrix, modelMatrix, scaleMatrix);

    // Multiply with perspective matrix
    mat4.multiply(viewMatrix, perspectiveProjectionMatrix, modelMatrix);
    gl.uniformMatrix4fv(mvpMatrixUniform, false, viewMatrix);
    gl.vertexAttrib3f(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 1.0, 1.0, 1.0);
    sphere.draw();

    popMatrix(); // Restore the matrix state

    gl.useProgram(null);

    update();

    requestAnimationFrame(display, canvas);     // Swapbuffer equivalent
}

function degToRad(degrees)
{
    return(degrees * Math.PI / 180.0);
}

function update()
{
    // Code
}

function pushMatrix(matrix) {
    // Push the current matrix onto the stack
    matrixStack.push(mat4.clone(matrix)); // Clone to prevent reference issues
}

function popMatrix() {
    // Pop the last matrix from the stack and return it
    return matrixStack.pop();
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
