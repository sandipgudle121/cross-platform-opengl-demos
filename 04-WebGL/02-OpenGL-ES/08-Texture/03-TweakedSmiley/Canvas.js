var canvas = null;
var gl = null;
var bFullscreen = false;

var canvas_original_width;
var canvas_original_height;

// WebGL related variables
const vertexAttributeEnum = 
                            {
                                SSG_ATTRIBUTE_POSITION:0,
                                SSG_ATTRIBUTE_TEXCOORD:1
                            };

var shaderProgramObject = null;

var vao = null;
var vbo = null;
var vbo_texcoord = null;

var smiley_texture;
var keyPressed = 0;

var uKeyPressedUniform;
var textureSamplerUniform = 0;
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
        
        case 49 :
        //case  :
            keyPressed = 1;
            break;
        
        case 50 :
        //case  :
            keyPressed = 2;
            break;
        
        case 51 :
        //case  :
            keyPressed = 3;
            break;
        
        case 52 :
        //case  :
            keyPressed = 4;
            break;
        
        default :
            keyPressed = 0;
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
                                    "in vec4 aPosition;\n"+
                                    "in vec2 aTexcoord;\n"+
                                    "out vec2 oTexCoord;\n"+
                                    "uniform mat4 uMVPMatrix;\n"+
                                    "void main(void)\n"+
                                    "{\n"+
                                    "gl_Position=uMVPMatrix * aPosition;\n"+
                                    "oTexCoord = aTexcoord;\n"+
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
                                    "in vec2 oTexCoord;\n"+
                                    "uniform sampler2D uTextureSampler;\n"+
                                    "uniform int uKeyPressed;\n"+
                                    "out vec4 FragColor;\n"+
                                    "void main(void)\n"+
                                    "{\n"+
                                    "if(uKeyPressed == 0)\n"+
                                    "{\n"+
                                    "FragColor = vec4(1.0,1.0,1.0,1.0);\n"+
                                    "}\n"+
                                    "else\n"+
                                    "{\n"+
                                    "FragColor = texture(uTextureSampler, oTexCoord);\n"+
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
    gl.bindAttribLocation(shaderProgramObject, vertexAttributeEnum.SSG_ATTRIBUTE_TEXCOORD, "aTexcoord");
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
    textureSamplerUniform = gl.getUniformLocation(shaderProgramObject, "uTextureSampler");
    uKeyPressedUniform = gl.getUniformLocation(shaderProgramObject, "uKeyPressed");

    // Geometry attribute declarations
    var triangle_position = new Float32Array([
                                                1.0, 1.0, 0.0,
                                                -1.0, 1.0, 0.0,
                                                -1.0, -1.0, 0.0,
                                                1.0, -1.0, 0.0
                                            ]);


    // Vao
    vao = gl.createVertexArray();
    gl.bindVertexArray(vao);

    // Vbo 
    vbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);

    gl.bufferData(gl.ARRAY_BUFFER, triangle_position, gl.STATIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    
    // Vbo texcoord
    vbo_texcoord = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texcoord);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(4 * 2), gl.DYNAMIC_DRAW);
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_TEXCOORD, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_TEXCOORD);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    // Enable depth
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    //gl.enable(gl.CULL_FACE);

    // Set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    
    // Create texture
    loadGLTexture();

    // Initialize projection matrix
    perspectiveProjectionMatrix = mat4.create();
}

function loadGLTexture()
{
    smiley_texture = gl.createTexture();
    smiley_texture.image = new Image();
    smiley_texture.image.src = "smiley.png";
    smiley_texture.image.onload = function()
    {
        gl.bindTexture(gl.TEXTURE_2D, smiley_texture);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, smiley_texture.image);
        gl.generateMipmap(gl.TEXTURE_2D);
        gl.bindTexture(gl.TEXTURE_2D, null);
    }
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
    // Variable declarations
    var square_texcoord = new Float32Array(8);

    // Code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(shaderProgramObject);

    // Transformations
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, smiley_texture);
    gl.uniform1i(textureSamplerUniform, 0);

    gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
    gl.bindVertexArray(vao);

    // Texture Coordinates
    if(keyPressed == 1)
        {
            square_texcoord[0] = 1.0;
            square_texcoord[1] = 1.0;
            square_texcoord[2] = 0.0;
            square_texcoord[3] = 1.0;
            square_texcoord[4] = 0.0;
            square_texcoord[5] = 0.0;
            square_texcoord[6] = 1.0;
            square_texcoord[7] = 0.0;
    
            gl.uniform1i(uKeyPressedUniform,1);
        } 
        else if(keyPressed == 2)
        {
            square_texcoord[0] = 0.5;
            square_texcoord[1] = 0.5;
            square_texcoord[2] = 0.0;
            square_texcoord[3] = 0.5;
            square_texcoord[4] = 0.0;
            square_texcoord[5] = 0.0;
            square_texcoord[6] = 0.5;
            square_texcoord[7] = 0.0;
    
            gl.uniform1i(uKeyPressedUniform,1);
        }
        else if(keyPressed == 3)
        {
            square_texcoord[0] = 2.0;
            square_texcoord[1] = 2.0;
            square_texcoord[2] = 0.0;
            square_texcoord[3] = 2.0;
            square_texcoord[4] = 0.0;
            square_texcoord[5] = 0.0;
            square_texcoord[6] = 2.0;
            square_texcoord[7] = 0.0;
    
            gl.uniform1i(uKeyPressedUniform,1);
        }
        else if(keyPressed == 4)
        {
            square_texcoord[0] = 0.5;
            square_texcoord[1] = 0.5;
            square_texcoord[2] = 0.5;
            square_texcoord[3] = 0.5;
            square_texcoord[4] = 0.5;
            square_texcoord[5] = 0.5;
            square_texcoord[6] = 0.5;
            square_texcoord[7] = 0.5;
    
            gl.uniform1i(uKeyPressedUniform,1);
        }
        else
        {
            gl.uniform1i(uKeyPressedUniform, 0);         
        }
    
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texcoord);
    gl.bufferData(gl.ARRAY_BUFFER, square_texcoord, gl.DYNAMIC_DRAW);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.bindVertexArray(null);
    gl.bindTexture(gl.TEXTURE_2D, null);

    gl.useProgram(null);

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

    if(vbo_texcoord)
    {
        gl.deleteBuffer(vbo_texcoord);
        vbo_texcoord = null;
    }

    if(vbo)
    {
        gl.deleteBuffer(vbo);
        vbo = null;
    }

    if(vao)
    {
        gl.deleteVertexArray(vao);
        vao = null;
    }
}

