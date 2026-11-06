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
var shaderProgramObject_PF = null;

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
var light_type = 'v';


var lightPosition = [ 100.0, 100.0, 100.0, 1.0]; 
var lightAmbient = [ 0.1, 0.1, 0.1 ];      
var lightSpecular = [ 1.0, 1.0, 1.0];        
var lightDiffused = [ 1.0, 1.0, 1.0 ]; 

var materialDiffused = [ 0.5, 0.2, 0.7 ];   
var materialAmbient = [ 0.0, 0.0, 0.0 ];
var materialSpecular = [ 0.7, 0.7, 0.7 ];
var materialShininess = 128.0;

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
            uninitialize();
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
        console.log("Per vertex light Vertex shader compile successfully\n");
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
        console.log("Per vertext light Fragment shader compile successfully\n");
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
        console.log("Per Vertex Shader program linked successfully\n");
    }

    


    // Vertex shader of Per Fragment Light
    var vertexShaderSourceCode_PF = 
                                    "#version 300 es\n"+
                                    "\n"+
                                    "in vec4 aPosition;\n"+
                                    "in vec3 aNormal;\n"+
                                    "in vec4 aColor;\n"+
                                    "out vec4 oColor;\n"+
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
        console.log("Per Fragment light Vertex shader compile successfully\n");
    }

    // Fragment shader of Per Fragment Light
    var fragmentShaderSourceCode_PF = 
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
                                    "vec3 normalized_Transformed_Normals = normalize(oTransformed_Normals);\n"+
                                    "vec3 normalized_Light_Direction = normalize(oLight_Direction);\n"+
                                    "vec3 normalized_Viewer_Vector = normalize(oViewer_Vector);\n"+
                                    "\n"+
                                    "vec3 lightAmbient = uLight_Ambient * uMaterial_Ambient;\n"+
                                    "vec3 lightDiffused = uLight_Diffused * uMaterial_Diffused * max(dot(normalized_Light_Direction, normalized_Transformed_Normals), 0.0);\n"+
                                    "vec3 reflection_Vector = reflect(-normalized_Light_Direction, normalized_Transformed_Normals);\n"+
                                    "vec3 lightSpecular = uLight_Specular * uMaterial_Specular * pow(max(dot(reflection_Vector, normalized_Viewer_Vector), 0.0f), uMaterial_Shininess );\n"+
                                    "phong_ADS_Light = lightAmbient + lightDiffused + lightSpecular;\n"+
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
        console.log("Per Fragment light Fragment shader compile successfully\n");
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
        console.log("Per Fragment Shader program linked successfully\n");
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
    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(canvas.width)/parseFloat(canvas.height), 0.1, 100.0);
}

function display()
{
    // Code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    if(light_type == 'v')
    {
        gl.useProgram(shaderProgramObject_PV);

        // Transformations
        var modelMatrix = mat4.create();
        var viewMatrix = mat4.create();
        var modelViewMatrix = mat4.create();

        // Get shader uniform location for Per Vertex Light
        modelMatrixUniform = gl.getUniformLocation(shaderProgramObject_PV, "uModelMatrix");
        viewMatrixUniform = gl.getUniformLocation(shaderProgramObject_PV, "uViewMatrix");
        projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject_PV, "uProjectionMatrix");

        light_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Diffused");
        light_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Ambient");
        light_Specular_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Specular");
        light_Position_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uLight_Position");

        material_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uMaterial_Diffused");
        material_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uMaterial_Ambient");
        material_Specular_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uMaterial_Specular");
        material_Shininess_Uniform = gl.getUniformLocation(shaderProgramObject_PV, "uMaterial_Shininess");
        
        keyPressedUniform = gl.getUniformLocation(shaderProgramObject_PV, "uKeyPressed");
        
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

            gl.uniform3fv(material_Diffused_Uniform, materialDiffused);
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
    }
    
    if(light_type == 'f')
    {
        gl.useProgram(shaderProgramObject_PF);

        // Transformations
        var modelMatrix = mat4.create();
        var viewMatrix = mat4.create();
        var modelViewMatrix = mat4.create();

        // Get shader uniform location for Per Fragment Light
        modelMatrixUniform = gl.getUniformLocation(shaderProgramObject_PF, "uModelMatrix");
        viewMatrixUniform = gl.getUniformLocation(shaderProgramObject_PF, "uViewMatrix");
        projectionMatrixUniform = gl.getUniformLocation(shaderProgramObject_PF, "uProjectionMatrix");

        light_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Diffused");
        light_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Ambient");
        light_Specular_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Specular");
        light_Position_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uLight_Position");

        material_Diffused_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uMaterial_Diffused");
        material_Ambient_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uMaterial_Ambient");
        material_Specular_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uMaterial_Specular");
        material_Shininess_Uniform = gl.getUniformLocation(shaderProgramObject_PF, "uMaterial_Shininess");

        keyPressedUniform = gl.getUniformLocation(shaderProgramObject_PF, "uKeyPressed");
        
        
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

            gl.uniform3fv(material_Diffused_Uniform, materialDiffused);
            gl.uniform3fv(material_Ambient_Uniform,  materialAmbient);
            gl.uniform3fv(material_Specular_Uniform, materialSpecular);
            gl.uniform1f(material_Shininess_Uniform, materialShininess);
        }
        else
        {
            gl.uniform1i(keyPressedUniform, 0);
        }

        sphere.draw();
        gl.useProgram(null);
    }
    

    // Double buffering (swapbuffer)
    requestAnimationFrame(display, canvas);
}

function uninitialize()
{
    //Code
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
}

