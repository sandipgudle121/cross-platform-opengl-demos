var canvas = null;
var gl = null;
var bFullscreen = false;

var canvas_original_width;
var canvas_original_height;

// WebGL related variables
const vertexAttributeEnum = 
                            {
                                SSG_ATTRIBUTE_POSITION:0,
                                SSG_ATTRIBUTE_COLOR:1
                            };

var shaderProgramObject = null;

var vao_triangle = null;
var vbo_triangle_position = null;
var vbo_triangle_color = null;

var vao_square = null;
var vbo_square_position = null;

var angle = 0.0;


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
        case 81 :
        case 113 :
            uninitialize();
            window.close();
        
        case 70 :
        case 120 :
            toggleFullscreen();
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
        if(canvas.requstFullscreen)
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
                                    "in vec4 aColor;\n"+
                                    "out vec4 oColor;\n"+
                                    "uniform mat4 uMVPMatrix;\n"+
                                    "void main(void)\n"+
                                    "{\n"+
                                    "gl_Position=uMVPMatrix * aPosition;\n"+
                                    "oColor=aColor;\n"+
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
                                    "precision highp float;\n"+
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
    var triangle_position = new Float32Array([
                                                0.0, 1.0, 0.0,
                                                -1.0, -1.0, 0.0,
                                                1.0, -1.0, 0.0
                                            ]);
    
    var triangle_color = new Float32Array([
                                                1.0, 0.0, 0.0,
                                                0.0, 1.0, 0.0,
                                                0.0, 0.0, 1.0
                                          ]);
                                                      
    var square_position = new Float32Array([
                                                1.0, 1.0, 0.0,
                                                -1.0, 1.0, 0.0,
                                                -1.0, -1.0, 0.0,
                                                1.0, -1.0, 0.0
                                        ]);
    
    // Vao triangle
    vao_triangle = gl.createVertexArray();
    gl.bindVertexArray(vao_triangle);

    // Vbo position
    vbo_triangle_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_triangle_position);
    gl.bufferData(gl.ARRAY_BUFFER, triangle_position, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    
    // Vbo color
    vbo_triangle_color = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_triangle_color);
    gl.bufferData(gl.ARRAY_BUFFER, triangle_color, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR);
    
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);


    // Vao square
    vao_square = gl.createVertexArray();
    gl.bindVertexArray(vao_square);

    // Vbo position
    vbo_square_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_square_position);

    gl.bufferData(gl.ARRAY_BUFFER, square_position, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    // vbo color
    gl.vertexAttrib3f(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);

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
    // Code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(shaderProgramObject);

    // Triangle
    // Transformations
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [-1.5, 0.0, -4.0]);
    mat4.rotateY(modelViewMatrix, modelViewMatrix, degToRad(angle));
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.bindVertexArray(vao_triangle);
    gl.drawArrays(gl.TRIANGLES, 0, 3);
    gl.bindVertexArray(null);

    // Square
    // Transformations
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [1.5, 0.0, -4.0]);
    mat4.rotateX(modelViewMatrix, modelViewMatrix, degToRad(angle));
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.bindVertexArray(vao_square);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.bindVertexArray(null);
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
    // Code
    angle = angle + 1.0;
    if(angle >= 360.0)
        angle = angle - 360.0;
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

    
    
    if(vbo_square_position)
    {
        gl.deleteBuffer(vbo_square_position);
        vbo_square_position = null;
    }
    
    if(vao_square)
    {
        gl.deleteVertexArray(vao_square);
        vao_square = null;
    }

    if(vbo_triangle_color)
    {
        gl.deleteBuffer(vbo_triangle_color);
        vbo_triangle_color = null;
    }

    if(vbo_triangle_position)
    {
        gl.deleteBuffer(vbo_triangle_position);
        vbo_triangle_position = null;
    }

    if(vao_triangle)
    {
        gl.deleteVertexArray(vao_triangle);
        vao_triangle = null;
    }
}

