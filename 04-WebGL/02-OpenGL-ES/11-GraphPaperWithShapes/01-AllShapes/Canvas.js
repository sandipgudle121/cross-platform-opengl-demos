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

// Green Line
var VAO_green_line = null;
var VBO_green_line_color = null;
var VBO_green_line = null;

// Red Line
var VAO_red_line = null;
var VBO_red_line = null;
var VBO_red_line_color = null;

// Horizontal Lines
var VAO_horizontal_blue_lines = null;
var VBO_horizontal_blue_lines = null;
var VBO_horizontal_blue_lines_color = null;

// Vertical Lines
var VAO_vertical_blue_lines = null;
var VBO_vertical_blue_lines = null;
var VBO_vertical_blue_lines_color = null;

// Triangle
var VAO_triangle = null;
var VBO_triangle = null;
var VBO_triangle_color = null;

// Square
var VAO_square = null;
var VBO_square = null;
var VBO_square_color = null;

// Circle
var VAO_circle = null;
var VBO_circle = null;
const  circle_points = 360;


var TRIANGLE = false;
var SQUARE = false;
var CIRCLE = false;
var GRAPH = false;

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

        case 71 :
        case 103 :
            if(GRAPH == true)
            {
                GRAPH = false;
            }
            else
            {
                GRAPH = true;
            }
            break;
        
        case 115 :
        case 83 :
            if(SQUARE == true)
            {
                SQUARE = false;
            }
            else
            {
                SQUARE = true;
            }
            break;
        
        case 116 :
        case 84 :
            if(TRIANGLE == true)
            {
                TRIANGLE = false;
            }
            else
            {
                TRIANGLE = true;
            }
            break;

        case 99 :
        case 67 :
            if(CIRCLE == true)
            {
                CIRCLE = false;
            }
            else
            {
                CIRCLE = true;
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
    var red_line_position = new Float32Array([
                                                -2.0, 0.0, 0.0,
                                                2.0, 0.0, 0.0
                                            ]);
    
    var green_line_position = new Float32Array([
                                                    0.0, 2.0, 0.0,
                                                    0.0, -2.0, 0.0
                                               ]);                                            

    var green_line_color = new Float32Array([
                                                0.0, 1.0, 0.0,
                                                0.0, 1.0, 0.0,
                                            ]);  
                     
    var red_line_color = new Float32Array([
                                                1.0, 0.0, 0.0,
                                                1.0, 0.0, 0.0,
                                          ]); 
    
    var horizontal_blue_lines = new Float32Array([ 
                                                    -2.0, 2.0, 0.0,
                                                    2.0, 2.0, 0.0    
                                                ]);
                                            
    var horizontal_blue_lines_color = new Float32Array([ 
                                                            0.0, 0.0, 1.0,
                                                            0.0, 0.0, 1.0
                                                      ]);

    var vertical_blue_lines = new Float32Array([ 
                                                    2.0, 2.0, 0.0,
                                                    2.0, -2.0, 0.0    
                                              ]);
                                                
    var vertical_blue_lines_color = new Float32Array([ 
                                                            0.0, 0.0, 1.0,
                                                            0.0, 0.0, 1.0
                                                      ]);

    var triangle = new Float32Array([ 
                                        0.0, 1.0, 0.0,
                                        -1.0, -1.0, 0.0,
                                        1.0, -1.0, 0.0    
                                    ]);
                                                    
    var triangle_color = new Float32Array([ 
                                                1.0, 1.0, 0.0,
                                                1.0, 1.0, 0.0,
                                                1.0, 1.0, 0.0
                                          ]);

    var square = new Float32Array([ 
                                        1.0, 1.0, 0.0,
                                        -1.0, 1.0, 0.0,
                                        -1.0, -1.0, 0.0,
                                        1.0, -1.0, 0.0    
                                  ]);
                                                        
    var square_color = new Float32Array([ 
                                            1.0, 1.0, 0.0,
                                            1.0, 1.0, 0.0,
                                            1.0, 1.0, 0.0,
                                            1.0, 1.0, 0.0
                                        ]);
                                
    const circle_vertices = new Float32Array(circle_points * 3);

    for (let i = 0; i < circle_points; i++) 
    {
        const angle = 2 * Math.PI * i / circle_points;
        circle_vertices[i * 3] = Math.cos(angle);           // x-coordinate
        circle_vertices[i * 3 + 1] = Math.sin(angle);       // y-coordinate
        circle_vertices[i * 3 + 2] = 0.0;                   // z-coordinate 
    }

    // VAO for RED line
    VAO_red_line = gl.createVertexArray();
    gl.bindVertexArray(VAO_red_line);
    // VBO Position
    VBO_red_line = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_red_line);
    gl.bufferData(gl.ARRAY_BUFFER, red_line_position, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // VBO Color
    VBO_red_line_color = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_red_line_color);
    gl.bufferData(gl.ARRAY_BUFFER, green_line_color, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // Un-bind VAO
    gl.bindVertexArray(null);


    // VAO for GREEN line
    VAO_green_line = gl.createVertexArray();
    gl.bindVertexArray(VAO_green_line);
    // VBO Position
    VBO_green_line = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_green_line);
    gl.bufferData(gl.ARRAY_BUFFER, green_line_position, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // VBO Color
    VBO_green_line_color = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_green_line_color);
    gl.bufferData(gl.ARRAY_BUFFER, red_line_color, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // Un-bind VAO
    gl.bindVertexArray(null);


    // VAO for Horizontal BLUE lines
    VAO_horizontal_blue_lines = gl.createVertexArray();
    gl.bindVertexArray(VAO_horizontal_blue_lines);
    // VBO Position
    VBO_horizontal_blue_lines = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_horizontal_blue_lines);
    gl.bufferData(gl.ARRAY_BUFFER, horizontal_blue_lines, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // VBO Color
    VBO_horizontal_blue_lines_color = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_horizontal_blue_lines_color);
    gl.bufferData(gl.ARRAY_BUFFER, horizontal_blue_lines_color, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // Un-bind VAO
    gl.bindVertexArray(null);


    // VAO for Vertical BLUE lines
    VAO_vertical_blue_lines = gl.createVertexArray();
    gl.bindVertexArray(VAO_vertical_blue_lines);
    // VBO Position
    VBO_vertical_blue_lines = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_vertical_blue_lines);
    gl.bufferData(gl.ARRAY_BUFFER, vertical_blue_lines, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // VBO Color
    VBO_vertical_blue_lines_color = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_vertical_blue_lines_color);
    gl.bufferData(gl.ARRAY_BUFFER, vertical_blue_lines_color, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // Un-bind VAO
    gl.bindVertexArray(null);


    // VAO for Triangle
    VAO_triangle = gl.createVertexArray();
    gl.bindVertexArray(VAO_triangle);
    // VBO Position
    VBO_triangle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_triangle);
    gl.bufferData(gl.ARRAY_BUFFER, triangle, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // VBO Color
    VBO_triangle_color = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_triangle_color);
    gl.bufferData(gl.ARRAY_BUFFER, triangle_color, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // Un-bind VAO
    gl.bindVertexArray(null);


    // VAO for Square
    VAO_square = gl.createVertexArray();
    gl.bindVertexArray(VAO_square);
    // VBO Position
    VBO_square = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_square);
    gl.bufferData(gl.ARRAY_BUFFER, square, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // VBO Color
    VBO_square_color = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_square_color);
    gl.bufferData(gl.ARRAY_BUFFER, square_color, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // Un-bind VAO
    gl.bindVertexArray(null);
    

    // VAO for Circle
    VAO_circle = gl.createVertexArray();
    gl.bindVertexArray(VAO_circle);
    // VBO Position
    VBO_circle = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO_circle);
    gl.bufferData(gl.ARRAY_BUFFER, circle_vertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    // VBO Color
    gl.vertexAttrib3f(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
    // Un-bind VAO
    gl.bindVertexArray(null);

    // Enable depth
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.enable(gl.CULL_FACE);

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

    if(GRAPH == true)
    {

        // Horizontal BLUE Lines
        for(let i = 0; i < 81; i++)
        {
            gl.useProgram(shaderProgramObject);

            // Transformations
            var modelViewMatrix = mat4.create();
            var modelViewProjectionMatrix = mat4.create();

            mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 2.0 * - i * 0.025, -4.0]);
            mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

            gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
            gl.bindVertexArray(VAO_horizontal_blue_lines);
            gl.drawArrays(gl.LINES, 0, 2);
            gl.bindVertexArray(null);
            gl.useProgram(null);
        }

        // Vertical BLUE Lines
        for(let i = 0; i < 81; i++)
        {
            gl.useProgram(shaderProgramObject);

            // Transformations
            var modelViewMatrix = mat4.create();
            var modelViewProjectionMatrix = mat4.create();

            mat4.translate(modelViewMatrix, modelViewMatrix, [2.0 * - i * 0.025, 0.0, -4.0]);
            mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

            gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
            gl.bindVertexArray(VAO_vertical_blue_lines);
            gl.drawArrays(gl.LINES, 0, 2);
            gl.bindVertexArray(null);
            gl.useProgram(null);
        }

        // RED Line
        gl.useProgram(shaderProgramObject);

        // Transformations
        var modelViewMatrix = mat4.create();
        var modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -4.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
        gl.bindVertexArray(VAO_red_line);
        gl.lineWidth(2);
        gl.drawArrays(gl.LINES, 0, 2);
        gl.bindVertexArray(null);
        gl.useProgram(null);


        // GREEN Line
        gl.useProgram(shaderProgramObject);

        // Transformations
        var modelViewMatrix = mat4.create();
        var modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -4.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
        gl.bindVertexArray(VAO_green_line);
        gl.lineWidth(5);
        gl.drawArrays(gl.LINES, 0, 2);
        gl.bindVertexArray(null);
        gl.useProgram(null);
    }

    // Triangle
    if(TRIANGLE == true)
    {
        gl.useProgram(shaderProgramObject);

        // Transformations
        var modelViewMatrix = mat4.create();
        var modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -4.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
        gl.bindVertexArray(VAO_triangle);
        gl.lineWidth(2);
        gl.drawArrays(gl.LINE_LOOP, 0, 3);
        gl.bindVertexArray(null);
        gl.useProgram(null);
    }

    // Square
    if(SQUARE == true)
    {
        gl.useProgram(shaderProgramObject);

        // Transformations
        var modelViewMatrix = mat4.create();
        var modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -4.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
        gl.bindVertexArray(VAO_square);
        gl.lineWidth(2);
        gl.drawArrays(gl.LINE_LOOP, 0, 4);
        gl.bindVertexArray(null);
        gl.useProgram(null);
    }    

    // Circle
    if(CIRCLE == true)
    {
        gl.useProgram(shaderProgramObject);

        // Transformations
        var modelViewMatrix = mat4.create();
        var modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -4.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
        gl.bindVertexArray(VAO_circle);
        gl.lineWidth(2);
        gl.drawArrays(gl.LINE_LOOP, 0, circle_points);
        gl.bindVertexArray(null);
        gl.useProgram(null);
    }

    // Update 
    update();

    // Double buffering (swapbuffer)
    requestAnimationFrame(display, canvas);
}

function update()
{
    // Code
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

    // Circle
    if(VBO_circle)
    {
        gl.deleteBuffer(VBO_circle);
        VBO_circle = null;
    }
    if(VAO_square)
    {
        gl.deleteVertexArray(VAO_square);
        VAO_square = null;
    }
    
    // Triangle
    if(VBO_square_color)
    {
        gl.deleteBuffer(VBO_square_color);
        VBO_square_color = null;
    }
    if(VBO_square)
    {
        gl.deleteBuffer(VBO_square);
        VBO_square = null;
    }
    if(VAO_square)
    {
        gl.deleteVertexArray(VAO_square);
        VAO_square = null;
    }
    
    // Triangle
    if(VBO_triangle_color)
    {
        gl.deleteBuffer(VBO_triangle_color);
        VBO_triangle_color = null;
    }
    if(VBO_triangle)
    {
        gl.deleteBuffer(VBO_triangle);
        VBO_triangle = null;
    }
    if(VAO_triangle)
    {
        gl.deleteVertexArray(VAO_triangle);
        VAO_triangle = null;
    }

    // Vertical Line
    if(VBO_vertical_blue_lines)
    {
        gl.deleteBuffer(VBO_vertical_blue_lines);
        VBO_vertical_blue_lines = null;
    }
    if(VBO_vertical_blue_lines_color)
    {
        gl.deleteBuffer(VBO_vertical_blue_lines_color);
        VBO_vertical_blue_lines_color = null;
    }
    if(VAO_vertical_blue_lines)
    {
        gl.deleteVertexArray(VAO_vertical_blue_lines);
        VAO_vertical_blue_lines = null;
    }
    
    // Horizontal Line
    if(VBO_horizontal_blue_lines)
    {
        gl.deleteBuffer(VBO_horizontal_blue_lines);
        VBO_horizontal_blue_lines = null;
    }
    if(VBO_horizontal_blue_lines_color)
    {
        gl.deleteBuffer(VBO_horizontal_blue_lines_color);
        VBO_horizontal_blue_lines_color = null;
    }
    if(VAO_horizontal_blue_lines)
    {
        gl.deleteVertexArray(VAO_horizontal_blue_lines);
        VAO_horizontal_blue_lines = null;
    }
    
    // RED Line
    if(VBO_red_line_color)
    {
        gl.deleteBuffer(VBO_red_line_color);
        VBO_red_line_color = null;
    }
    if(VBO_red_line)
    {
        gl.deleteBuffer(VBO_red_line);
        VBO_red_line = null;
    }
    if(VAO_red_line)
    {
        gl.deleteVertexArray(VAO_red_line);
        VAO_red_line = null;
    }

    // GREEN Line
    if(VBO_green_line_color)
    {
        gl.deleteBuffer(VBO_green_line_color);
        VBO_green_line_color = null;
    }
    if(VBO_green_line)
    {
        gl.deleteBuffer(VBO_green_line);
        VAO_green_line = null;
    }
    if(VAO_green_line)
    {
        gl.deleteVertexArray(VAO_green_line);
        VAO_green_line = null;
    }
}

