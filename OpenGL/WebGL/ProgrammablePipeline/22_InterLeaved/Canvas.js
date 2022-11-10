var canvas=null;
var gl=null;  //webgl contex
var bFullscreen=false;
var canvas_original_width;
var canvas_original_height;

const WebGLMacros={
    AMC_ATTRIBUTE_VERTEX:0,
    AMC_ATTRIBUTE_COLOR:1,
    AMC_ATTRIBUTE_NORMAL:2,
    AMC_ATTRIBUTE_TEXTURE0:3,
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var  vao_Cube;
var  vbo_Position_Cube;
var  vbo_Texture_Cube;
var  vbo_Normal_Cube;
var mvUniform,pUniform ,LDUiniform ,KDUiniform ,LightPositionUiniform ,LKeyIsPressedUiniform;

var perspectiveProjectionMatrix=null;

var requestAnimationFrame=window.requestAnimationFrame||
window.webkitRequestAnimationFrame||
window.mozRequestAnimationFrame||
window.oRequestAnimationFrame||
window.msRequestAnimationFrame;

var cancelAnimationFrame=window.cancelAnimationFrame||
window.webkitCancelRequestAnimationFrame||window.webkitCancelAnimationFrame||
window.mozCancelRequestAnimationFrame||window.mozCancelAnimationFrame||
window.oCancelRequestAnimationFrame||window.oCancelAnimationFrame||
window.msCancelRequestAnimationFrame||window.msCancelRequestAnimationFrame;

var rotateCube = 360.0;
var textureKundali;
var samplerUniform;

var gLighting=0;

//onload function
function main()
{
    //get <canvas element>
    canvas=document.getElementById("AMC");
    if(!canvas)
        console.log("Obtaining canvas failed\n");
    else
        console.log("Obtaining canvas successfully");
    
    canvas_original_width=canvas.width;
    canvas_original_height=canvas.height;    
 
    //register keyboard's keydown event
    window.addEventListener("keydown",keyDown,false);
    window.addEventListener("click",mouseDown,false);
    window.addEventListener("resize",resize,false);
   
    //initialize webGL
    init();

    draw();
}

function init()
{
    gl=canvas.getContext("webgl2");
    if(gl==null)
    {
        console.log("Obtaining webGL context failed");
        return;
    }

    gl.viewportWidth=canvas.width;
    gl.viewportHeight=canvas.height;

    //vertex Shader
    var vertexShaderSourceCode=
    "#version 300 es"+
    "\n"+
 "precision highp float;"+
"precision mediump int;"+
   "in vec4 vPosition;" +
		"in vec3 vNormal;" +
		"in vec3 vColor;" +
		"in vec2 vTexCoord;" +
		"uniform mat4 u_mv_matrix;" +
		"uniform mat4 u_p_matrix;" +
		"uniform vec3 u_Ld;" +
		"uniform vec3 u_Kd;" +
		"uniform vec4 u_Light_Position;" +
		"uniform int u_LKeyIsPressed;" +
		"out vec3 out_DiffusedColor;" +
		"out vec4 out_vecColor;" +
		"out vec2 out_texCoord;" +
		"void main(void)" +
		"{" +
		"if(u_LKeyIsPressed==1)" +
		"{" +
		"vec4 eye_coordinates=u_mv_matrix*vPosition;"+
		"mat3 normalMatrix=mat3(transpose(inverse(u_mv_matrix)));" +
		"vec3 tNorm=normalize(normalMatrix*vNormal);" +
		"vec3 s=normalize(vec3(u_Light_Position-eye_coordinates));" +
		"out_DiffusedColor=u_Ld*u_Kd*max(dot(s,tNorm),0.0);" +
		"}" +
		"out_vecColor=vec4(vColor,1.0);" +
		"out_texCoord=vTexCoord;" +
		"gl_Position=u_p_matrix*u_mv_matrix*vPosition;" +
		"}";


    vertexShaderObject=gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject,vertexShaderSourceCode);
    gl.compileShader(vertexShaderObject);

    if(gl.getShaderParameter(vertexShaderObject,gl.COMPILE_STATUS)==false)
    {
        var error=gl.getShaderInfoLog(vertexShaderObject);
        if(error.length>0)
        {
            alert(error);
            uninitialize();
        }
    }

    //fragment shader
    var fragmentShaderSourceCode=
    "#version 300 es"+
    "\n"+
    "precision highp float;"+
"precision mediump int;"+
   "in vec3 out_DiffusedColor;" +
		"in vec2 out_texCoord;" +
		"in vec4 out_vecColor;" +
		"uniform int u_LKeyIsPressed;" +
		"uniform sampler2D u_sampler;" +
		"out vec4 FragColor;" +
		"void main(void)" +
		"{" +
		"if(u_LKeyIsPressed==1)" +
		"{" +
		"FragColor=texture(u_sampler,out_texCoord)*out_vecColor*vec4(out_DiffusedColor,1.0);" +
		"}" +
		"else" +
		"{" +
		"FragColor=texture(u_sampler,out_texCoord)*out_vecColor;" +
		"}" +
		"}";

    
    fragmentShaderObject=gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject,fragmentShaderSourceCode);
    gl.compileShader(fragmentShaderObject);

    if(gl.getShaderParameter(fragmentShaderObject,gl.COMPILE_STATUS)==false)
    {
        var error=gl.getShaderInfoLog(fragmentShaderObject);
        if(error.length>0)
        {
            alert(error);
            uninitialize();
        }
    }

    //shader program
    shaderProgramObject=gl.createProgram();
    gl.attachShader(shaderProgramObject,vertexShaderObject);
    gl.attachShader(shaderProgramObject,fragmentShaderObject);

    gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AMC_ATTRIBUTE_VERTEX,"vPostion");
    gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AMC_ATTRIBUTE_NORMAL,"vNormal");
    gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AMC_ATTRIBUTE_TEXTURE0,"vTexCoord");
    gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AMC_ATTRIBUTE_COLOR,"vColor");
    
    gl.linkProgram(shaderProgramObject);
    if(gl.getProgramParameter(shaderProgramObject,gl.LINK_STATUS))
    {
        var error=gl.getProgramInfoLog(shaderProgramObject);
        if(error.length>0)
        {
            alert(error);
            uninitialize();
        }
    }

   	mvUniform = gl.getUniformLocation(shaderProgramObject, "u_mv_matrix");
	pUniform = gl.getUniformLocation(shaderProgramObject, "u_p_matrix");
	LDUiniform  = gl.getUniformLocation(shaderProgramObject, "u_Ld");
	KDUiniform = gl.getUniformLocation(shaderProgramObject, "u_Kd");
	LightPositionUiniform = gl.getUniformLocation(shaderProgramObject, "u_Light_Position");
	LKeyIsPressedUiniform = gl.getUniformLocation(shaderProgramObject, "u_LKeyIsPressed");

	sampler_Uniform = gl.getUniformLocation(shaderProgramObject, "u_sampler");
	


    var  CubeVertices=new Float32Array([
      1.0, 1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0,
		-1.0, 1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
		-1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
		1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0,
		
		1.0, -1.0, -1.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0,
		-1.0, -1.0, -1.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
		-1.0, -1.0, 1.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
		1.0, -1.0, 1.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0,

		1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
		-1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0,
		-1.0, -1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0,
		1.0, -1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0,

		1.0, 1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 1.0, 1.0, 1.0, 0.0,
		-1.0, 1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 1.0, 1.0, 1.0, 1.0,
		-1.0, -1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 1.0, 1.0, 0.0, 1.0,
		1.0, -1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 1.0, 1.0, 0.0, 0.0,

		1.0, 1.0, -1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0,
		1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0,
		1.0, -1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
		1.0, -1.0, -1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0,

		-1.0, 1.0, 1.0, -1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0,
		-1.0, 1.0, -1.0, -1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0,
		-1.0, -1.0, -1.0, -1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0,
		-1.0, -1.0, 1.0, -1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0

    ]);

    
    vao_Cube=gl.createVertexArray();
    gl.bindVertexArray(vao_Cube);
    vbo_Position_Cube=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_Position_Cube);
    gl.bufferData(gl.ARRAY_BUFFER,CubeVertices,gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX,3,gl.FLOAT,false,11*4,0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
  	
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_NORMAL,3,gl.FLOAT,false,11*4,3*4);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_NORMAL);
  
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR,3,gl.FLOAT,false,11*4,6*4);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
  
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0,2,gl.FLOAT,false,11*4,9*4);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0);
    gl.bindBuffer(gl.ARRAY_BUFFER,null);

    gl.bindVertexArray(null);

    perspectiveProjectionMatrix=mat4.create();
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    textureKundali = loadGLTextures("marble.png");
    
    resize(window.innerWidth, window.innerHeight);
}

function loadGLTextures(resourcePath)
{
    let texture=gl.createTexture();

    let pixel=new Uint8Array([0,0,0,0]);
    gl.bindTexture(gl.TEXTURE_2D,texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, pixel);

    let image = new Image();
    image.src = resourcePath;
    image.onload = function() {
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, image.width, image.height, 0, gl.RGB, gl.UNSIGNED_BYTE, image);
        gl.generateMipmap(gl.TEXTURE_2D);
        gl.bindTexture(gl.TEXTURE_2D, null);
    };

    gl.bindTexture(gl.TEXTURE_2D, null);
    return texture;
}

function toggleFullScreen()
{
    //code
    var fullscreen_element=document.fullscreenElement||
                        document.webkitFullscreenElement||
                        document.mozFullScreenElement||
                        document.msFullscreenElement||
                        null;

    if(fullscreen_element==null)
    {
        if(canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if(canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if(canvas.webkitRequestFullscreen)
            canvas.webkitRequestFullscreen();
        else if(canvas.msRequestFullscreen)
            canvas.msRequestFullscreen();
    }   
    else
    {
        if(document.exitFullscreen)
            document.exitFullscreen();
        else if(document.mozCancelFullScreen) 
            document.mozCancelFullScreen();
        else if(document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if(document.msExitFullscreen)
            document.msExitFullscreen();
            
    }                 
}

function resize()
{
    if(bFullscreen==true)
    {
        canvas.width=window.innerWidth;
        canvas.height=window.innerHeight;
    }
    else{
        canvas.width=canvas_original_width;
        canvas.height=canvas_original_height;
    }

    gl.viewport(0,0,canvas.width,canvas.height);

    mat4.perspective(perspectiveProjectionMatrix, 45.0, canvas.width / canvas.height, 1.0, 100.0);
        
       
}

function draw()
{
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
    var modelViewMatrix=mat4.create();
    var translationMatrix=mat4.create();
    var rotationMatrix=mat4.create();
    mat4.translate(translationMatrix,translationMatrix,[0.0,0.0,-5.0]);
    mat4.multiply(modelViewMatrix,modelViewMatrix,translationMatrix);
   
    mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadian(rotateCube));
    mat4.multiply(modelViewMatrix,modelViewMatrix,rotationMatrix);

    
    gl.uniformMatrix4fv(mvUniform,false,modelViewMatrix);
	gl.uniformMatrix4fv(pUniform,false,perspectiveProjectionMatrix);
	if (gLighting == 1)
	{
		gl.uniform1i(LKeyIsPressedUiniform,
			1);
		
		gl.uniform3f(LDUiniform, 1.0, 1.0,1.0);
		gl.uniform3f(KDUiniform, 1.0,1.0,1.0);
		var lightPosition = [ 0.0, 0.0, 2.0, 1.0 ];
		gl.uniform4fv(LightPositionUiniform, lightPosition);
	}
	else
	{
		gl.uniform1i(LKeyIsPressedUiniform,0);
	}

   
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, textureKundali);
    gl.uniform1i(samplerUniform, 0);

    gl.bindVertexArray(vao_Cube);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
	gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
	gl.bindVertexArray(null);
    gl.useProgram(null);
    requestAnimationFrame(draw,canvas);

    //update

	rotateCube -= 0.1;
	if (rotateCube < 0)
	{
		rotateCube = 360.0;
	}
}

function degreeToRadian(degree) {
    return degree * Math.PI / 180.0;
}

function keyDown(event)
{
    switch(event.keyCode)
    {
        case 70:
            toggleFullScreen();
            break;
	case 76:
		if(gLighting==0)
			gLighting=1;
		else
			gLighting=0;
    }
}

function mouseDown()
{}

function uninitialize()
{
    if(vao_Cube)
    {
        gl.deleteVertexArray(vao_Cube);
        vao_Cube=null;
    }
    if(vbo_Position_Cube)
    {
        gl.deleteBuffer(vbo_Position_Cube);
        vbo_Position_Cube=null;
    }
    if(vbo_Texture_Cube)
    {
        gl.deleteBuffer(vbo_Texture_Cube);
        vbo_Texture_Cube=null;
    }

    if(vao_Pyramid)
    {
        gl.deleteVertexArray(vao_Pyramid);
        vao_Pyramid=null;
    }
    if(vbo_Position_Pyramid)
    {
        gl.deleteBuffer(vbo_Position_Pyramid);
        vbo_Position_Pyramid=null;
    }

    if(vbo_Texture_Pyramid)
    {
        gl.deleteBuffer(vbo_Texture_Pyramid);
        vbo_Texture_Pyramid=null;
    }
    if(shaderProgramObject)
    {
        if(fragmentShaderObject)
        {
            gl.detachShader(shaderProgramObject,fragmentShaderObject);
            gl.deleteShader(fragmentShaderObject);
            fragmentShaderObject=null;
        }
        
        if(vertexShaderObject)
        {
            gl.detachShader(shaderProgramObject,vertexShaderObject);
            gl.deleteShader(vertexShaderObject);
            vertexShaderObject=null;
        }
        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject=null;
    }
}