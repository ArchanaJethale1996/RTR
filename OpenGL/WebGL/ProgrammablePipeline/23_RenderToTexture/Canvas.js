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

var vao_Cube;
var vbo_Position_Cube;
var vbo_Texture_Cube;;
var mvpUniform;

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
var textureStone;
var samplerUniform;

//frame buffer changes
var gFrameBuffer = 0;
var gColorTexture = 0;
var gDepthTexture = 0;

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
    "in vec4 vPosition;"+
    "in vec2 vTexCoord;" +
    "out vec2 out_TexCoord;"+
    "uniform mat4 u_mvp_matrix;"+
    "void main(void)"+
    "{"+
    "gl_Position=u_mvp_matrix*vPosition;"+
    "out_TexCoord=vTexCoord;"+
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
    "in vec2 out_TexCoord;" +
    "uniform sampler2D u_sampler;"+
    "out vec4 FragColor;"+
    "void main(void)"+
    "{"+
    "FragColor=texture(u_sampler,out_TexCoord);"+
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
    gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AMC_ATTRIBUTE_TEXTURE0,"vTexCoord");
    
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

    mvpUniform=gl.getUniformLocation(shaderProgramObject,"u_mvp_matrix");
    samplerUniform= gl.getUniformLocation(shaderProgramObject, "u_sampler");

    var  CubeVertices=new Float32Array([
        1.0, 1.0, -1.0,
		-1.0, 1.0, -1.0,
		-1.0, 1.0, 1.0,
		1.0, 1.0 , 1.0 ,

		//bottom  face
		1.0 , -1.0 , -1.0 ,
		-1.0 , -1.0 , -1.0 ,
		-1.0 , -1.0 , 1.0 ,
		1.0 , -1.0 , 1.0 ,

		// front  face
		1.0 , 1.0 , 1.0 ,
		-1.0 , 1.0 , 1.0 ,
		-1.0 , -1.0 , 1.0 ,
		1.0 , -1.0 , 1.0 ,

		//back  face
		1.0 , 1.0 , -1.0 ,
		-1.0 , 1.0 , -1.0 ,
		-1.0 , -1.0 , -1.0 ,
		1.0 , -1.0 , -1.0 ,

		//right  face
		1.0 , 1.0 , -1.0 ,
		1.0 , 1.0 , 1.0 ,
		1.0 , -1.0 , 1.0 ,
		1.0 , -1.0 , -1.0 ,

		//left  face
		-1.0 , 1.0 , -1.0 ,
		-1.0 , 1.0 , 1.0 ,
		-1.0 , -1.0 , 1.0 ,
		-1.0 , -1.0 , -1.0 
    ]);

    var  CubeTexture=new Float32Array([
        0.0, 1.0,
		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0,

		1.0, 1.0, 
		0.0, 1.0, 
		0.0, 0.0, 
		1.0, 0.0, 

		1.0, 1.0,
		0.0, 1.0,
		0.0, 0.0, 
		1.0, 0.0, 

		1.0, 1.0, 
		0.0, 1.0, 
		0.0, 0.0,
		1.0, 0.0, 

		1.0, 0.0,
		1.0, 1.0,
		0.0, 1.0,
		0.0, 0.0,

		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0

    ]);

    vao_Cube=gl.createVertexArray();
    gl.bindVertexArray(vao_Cube);
    vbo_Position_Cube=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_Position_Cube);
    gl.bufferData(gl.ARRAY_BUFFER,CubeVertices,gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX,3,gl.FLOAT,false,0,0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER,null);

    vbo_Texture_Cube=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_Texture_Cube);
    gl.bufferData(gl.ARRAY_BUFFER,CubeTexture,gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXTURE0,2,gl.FLOAT,false,0,0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXTURE0);
    gl.bindBuffer(gl.ARRAY_BUFFER,null);
    gl.bindVertexArray(null);

    
    gFrameBuffer=gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, gFrameBuffer);
    //color
    gColorTexture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, gColorTexture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 800, 600, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    gl.bindTexture(gl.TEXTURE_2D, null);

    //depth
    gDepthTexture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, gDepthTexture);
    gl.texStorage2D(gl.TEXTURE_2D, 1, gl.DEPTH_COMPONENT32F, 800, 600);
    gl.bindTexture(gl.TEXTURE_2D, null);

    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, gColorTexture, 0);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, gDepthTexture, 0);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);



    perspectiveProjectionMatrix=mat4.create();
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    textureKundali = loadGLTextures("Kundali.png");
    
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
 
    
    if (gFrameBuffer) {
        gl.bindFramebuffer(gl.FRAMEBUFFER, gFrameBuffer);
        //color
        gColorTexture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, gColorTexture);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 800, 600, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
        gl.bindTexture(gl.TEXTURE_2D, null);

        //depth
        gDepthTexture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, gDepthTexture);
        gl.texStorage2D(gl.TEXTURE_2D, 1, gl.DEPTH_COMPONENT32F, 800, 600);
        gl.bindTexture(gl.TEXTURE_2D, null);

        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, gColorTexture, 0);
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, gDepthTexture, 0);
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    }
       
}

function draw()
{
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    var modelViewMatrix=mat4.create();
    var modelViewProjectionMatrix=mat4.create();
    var translationMatrix=mat4.create();
    var rotationMatrix=mat4.create();

    gl.bindFramebuffer(gl.FRAMEBUFFER, gFrameBuffer);
    gl.clearBufferfv(gl.COLOR, 0, [0.0, 1.0, 0.0, 1.0]);
    gl.clearBufferfv(gl.DEPTH, 0, [1.0, 1.0, 1.0, 1.0]);

    gl.useProgram(shaderProgramObject);
    mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -5.0]);
    mat4.multiply(modelViewMatrix,modelViewMatrix,translationMatrix);
    mat4.rotateX(rotationMatrix, rotationMatrix, degreeToRadian(rotateCube));
    mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadian(rotateCube));
    mat4.rotateZ(rotationMatrix, rotationMatrix, degreeToRadian(rotateCube));
  
    mat4.multiply(modelViewMatrix,modelViewMatrix,rotationMatrix);
    mat4.multiply(modelViewProjectionMatrix,perspectiveProjectionMatrix,modelViewMatrix);
    
    gl.uniformMatrix4fv(mvpUniform,false,modelViewProjectionMatrix);

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

	gl.bindFramebuffer(gl.FRAMEBUFFER, null);
	gl.bindTexture(gl.TEXTURE_2D, null);

	modelViewMatrix = mat4.create();
	modelViewProjectionMatrix = mat4.create();
	translationMatrix = mat4.create();
	rotationMatrix = mat4.create();
	gl.useProgram(shaderProgramObject);
	mat4.translate(translationMatrix, translationMatrix, [0.0, 0.0, -5.0]);
	mat4.multiply(modelViewMatrix, modelViewMatrix, translationMatrix);
	mat4.rotateX(rotationMatrix, rotationMatrix, degreeToRadian(rotateCube));
	mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadian(rotateCube));
	mat4.rotateZ(rotationMatrix, rotationMatrix, degreeToRadian(rotateCube));

	mat4.multiply(modelViewMatrix, modelViewMatrix, rotationMatrix);
	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

	gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, gColorTexture);;
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
    
    if (gFrameBuffer)
    {
        gl.deleteFramebuffer(gFrameBuffer);
        gFrameBuffer = 0;
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