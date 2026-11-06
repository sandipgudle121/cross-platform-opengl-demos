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
                                SSG_ATTRIBUTE_COLOR:2,
                                SSG_ATTRIBUTE_NORMAL:3
                            };

var shaderProgramObject = null;

var VAO = null;
var VBO = null;
var vbo_texcoord = null;
var marble_texture;
var angle = 0.0;

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
var bAnimationEnabled = false;


var lightPosition = [ 0.0, 0.0, 2.0, 1.0]; 
var lightAmbient = [ 0.1, 0.1, 0.1 ];      
var lightSpecular = [ 1.0, 1.0, 1.0];        
var lightDiffused = [ 1.0, 1.0, 1.0 ]; 

var materialDiffused = [ 1.0, 1.0, 1.0 ];   
var materialAmbient = [ 0.0, 0.0, 0.0 ];
var materialSpecular = [ 1.0, 1.0, 1.0 ];
var materialShininess = 128.0;

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
        
        case 97 :
        case 65 :
            if(bAnimationEnabled == false)
            {
                bAnimationEnabled = true;
            }
            else
            {
                bAnimationEnabled = false;
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
    "vec3 lightSpecular = uLight_Specular * uMaterial_Specular * pow(max(dot(reflection_Vector, normalized_Viewer_Vector), 0.0), uMaterial_Shininess );\n"+
    "phong_ADS_Light = lightAmbient + lightDiffused + lightSpecular;\n"+
    "}\n"+
    "else\n"+
    "{\n"+
    "phong_ADS_Light = vec3(1.0, 1.0, 1.0);\n"+
    "}\n"+
    "vec3 texColor = vec3(texture(uTextureSampler, oTexCoord));\n"+
    "FragColor = vec4(texColor * vec3(oColor) * phong_ADS_Light, 1.0);\n"+
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
    gl.bindAttribLocation(shaderProgramObject, vertexAttributeEnum.SSG_ATTRIBUTE_NORMAL, "aNormal");
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
    modelMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject, "uProjectionMatrix");
    keyPressedUniform = gl.getUniformLocation(shaderProgramObject, "uKeyPressed");
    textureSamplerUniform = gl.getUniformLocation(shaderProgramObject, "uTextureSampler");

    light_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject, "uLight_Diffused");
    light_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject, "uLight_Ambient");
    light_Specular_Uniform = gl.getUniformLocation(shaderProgramObject, "uLight_Specular");
    light_Position_Uniform = gl.getUniformLocation(shaderProgramObject, "uLight_Position");

    material_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject, "uMaterial_Diffused");
    material_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject, "uMaterial_Ambient");
    material_Specular_Uniform = gl.getUniformLocation(shaderProgramObject, "uMaterial_Specular");
    material_Shininess_Uniform = gl.getUniformLocation(shaderProgramObject, "uMaterial_Shininess");

    // Geometry attribute declarations
    var cube_PCNT = new Float32Array
    ([
        // front
        // position				// color		 // normals			// texcoords
        1.0,  1.0,  1.0,	    1.0, 0.0, 0.0,	 0.0,  0.0,  1.0,	1.0, 1.0,
        -1.0,  1.0,  1.0,	    1.0, 0.0, 0.0,	 0.0,  0.0,  1.0,	0.0, 1.0,
        -1.0, -1.0,  1.0,	    1.0, 0.0, 0.0,	 0.0,  0.0,  1.0,	0.0, 0.0,
        1.0, -1.0,  1.0, 	    1.0, 0.0, 0.0,	 0.0,  0.0,  1.0,	1.0, 0.0,
                        
        // right			 
        // position			// color		 // normals			// texcoords
        1.0,  1.0, -1.0,	0.0, 0.0, 1.0,	 1.0,  0.0,  0.0,	1.0, 1.0,
        1.0,  1.0,  1.0,	0.0, 0.0, 1.0,	 1.0,  0.0,  0.0,	0.0, 1.0,
        1.0, -1.0,  1.0,	0.0, 0.0, 1.0,	 1.0,  0.0,  0.0,	0.0, 0.0,
        1.0, -1.0, -1.0,	0.0, 0.0, 1.0,	 1.0,  0.0,  0.0,	1.0, 0.0,
                        
        // back				 
        // position				// color		 // normals		    // texcoords
        1.0,  1.0, -1.0,	    1.0, 1.0, 0.0,	 0.0,  0.0, -1.0,	1.0, 1.0,
        -1.0,  1.0, -1.0,	    1.0, 1.0, 0.0,	 0.0,  0.0, -1.0,	0.0, 1.0,
        -1.0, -1.0, -1.0,	    1.0, 1.0, 0.0,	 0.0,  0.0, -1.0,	0.0, 0.0,
        1.0, -1.0, -1.0,	    1.0, 1.0, 0.0,	 0.0,  0.0, -1.0,	1.0, 0.0,
                        
        // left				 
        // position			// color		 // normals			// texcoords
        -1.0,  1.0,  1.0,	1.0, 0.0, 1.0,	-1.0,  0.0,  0.0,	1.0, 1.0,
        -1.0,  1.0, -1.0,	1.0, 0.0, 1.0,	-1.0,  0.0,  0.0,	0.0, 1.0,
        -1.0, -1.0, -1.0,	1.0, 0.0, 1.0,	-1.0,  0.0,  0.0,	0.0, 0.0,
        -1.0, -1.0,  1.0,	1.0, 0.0, 1.0,	-1.0,  0.0,  0.0,	1.0, 0.0,
                        
        // top				 
        // position				// color		 // normals		    // texcoords
        1.0,  1.0, -1.0,	    0.0, 1.0, 0.0,	 0.0,  1.0,  0.0,	1.0, 1.0,
        -1.0,  1.0, -1.0,	    0.0, 1.0, 0.0,	 0.0,  1.0,  0.0,	0.0, 1.0,
        -1.0,  1.0,  1.0,	    0.0, 1.0, 0.0,	 0.0,  1.0,  0.0,	0.0, 0.0,
        1.0,  1.0,  1.0,	    0.0, 1.0, 0.0,	 0.0,  1.0,  0.0,	1.0, 0.0,
                        
        // bottom			 
        // position				// color		 // normals			// texcoords
        1.0, -1.0,  1.0,	    1.0, 0.5, 0.0,	 0.0, -1.0,  0.0,	1.0, 1.0,
        -1.0, -1.0,  1.0,	    1.0, 0.5, 0.0,	 0.0, -1.0,  0.0,	0.0, 1.0,
        -1.0, -1.0, -1.0,	    1.0, 0.5, 0.0,	 0.0, -1.0,  0.0,	0.0, 0.0,
        1.0, -1.0, -1.0,	    1.0, 0.5, 0.0,	 0.0, -1.0,  0.0,	1.0, 0.0, 
    ]);
    
    // Vao
    VAO = gl.createVertexArray();
    gl.bindVertexArray(VAO);

    // Create VBO for positions
    var VBO = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, VBO);
    gl.bufferData(gl.ARRAY_BUFFER, cube_PCNT, gl.STATIC_DRAW); // Use cube_PCNT directly if it's a Float32Array

    // Set position attribute
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 11 * Float32Array.BYTES_PER_ELEMENT, 0);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_POSITION);

    // Set color attribute
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 11 * Float32Array.BYTES_PER_ELEMENT, 3 * Float32Array.BYTES_PER_ELEMENT);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_COLOR);

    // Set normal attribute
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 11 * Float32Array.BYTES_PER_ELEMENT, 6 * Float32Array.BYTES_PER_ELEMENT);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_NORMAL);

    // Set texcoord attribute
    gl.vertexAttribPointer(vertexAttributeEnum.SSG_ATTRIBUTE_TEXCOORD, 2, gl.FLOAT, false, 11 * Float32Array.BYTES_PER_ELEMENT, 9 * Float32Array.BYTES_PER_ELEMENT);
    gl.enableVertexAttribArray(vertexAttributeEnum.SSG_ATTRIBUTE_TEXCOORD);

    // Unbind the VBO
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    // Enable depth
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    // Set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    
    // Create texture
    loadGLTexture();

    // Initialize projection matrix
    perspectiveProjectionMatrix = mat4.create();
}

function loadGLTexture()
{
    marble_texture = gl.createTexture();
    marble_texture.image = new Image();
    marble_texture.image.src = "marble.png";
    marble_texture.image.onload = function()
    {
        gl.bindTexture(gl.TEXTURE_2D, marble_texture);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, marble_texture.image);
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
    // Code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(shaderProgramObject);
    
    var viewMatrix = mat4.create(); 
    var modelMatrix = mat4.create(); 
    var projectionMatrix = mat4.create(); 
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();  

    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -5.0]);
    mat4.rotateX(modelMatrix, modelMatrix, degToRad(angle));
    mat4.rotateY(modelMatrix, modelMatrix, degToRad(angle));
    mat4.rotateZ(modelMatrix, modelMatrix, degToRad(angle));
    mat4.multiply(modelViewMatrix, viewMatrix, modelMatrix);
    mat4.perspective(projectionMatrix, degToRad(45), canvas.width / canvas.height, 0.1, 100.0);
    mat4.multiply(modelViewProjectionMatrix, projectionMatrix, modelViewMatrix);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, marble_texture);
    gl.uniform1i(textureSamplerUniform, null);

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

        gl.uniform3fv(material_Diffused_Uniform, materialDiffused);
        gl.uniform3fv(material_Ambient_Uniform,  materialAmbient);
        gl.uniform3fv(material_Specular_Uniform, materialSpecular);
        gl.uniform1f(material_Shininess_Uniform, materialShininess);
    }
    else
    {
        gl.uniform1i(keyPressedUniform, 0);
    }
    
    gl.bindVertexArray(VAO);
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
    angle = angle - 1.0;
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

    if(vbo_texcoord)
    {
        gl.deleteBuffer(vbo_texcoord);
        vbo_texcoord = null;
    }

    if(VBO)
    {
        gl.deleteBuffer(VBO);
        VBO = null;
    }
        
    if(VAO)
    {
        gl.deleteVertexArray(VAO);
        VAO = null;
    }
}