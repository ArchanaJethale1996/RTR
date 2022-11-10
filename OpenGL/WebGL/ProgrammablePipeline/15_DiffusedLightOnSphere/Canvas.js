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
var sphere=null;
var  mvUniform,pUniform ,LDUniform ,KDUniform ,LightPositionUniform ,LKeyIsPressedUniform;

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

var rotateSphere = 360.0;
var gLighting = 0;
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
    "\n" +
    "precision mediump int;" +
    "in vec4 vPosition;" +
    "in vec3 vNormal;" +
    "uniform mat4 u_mv_matrix;" +
    "uniform mat4 u_p_matrix;" +
    "uniform vec3 u_Ld;" +
    "uniform vec3 u_Kd;" +
    "uniform vec4 u_Light_Position;" +
    "uniform int u_LKeyIsPressed;" +
    "out vec3 out_DiffusedColor;" +
    "void main(void)" +
    "{" +
    "   if(u_LKeyIsPressed==1)" +
    "   {" +
    "      vec4 eye_coordinates=u_mv_matrix*vPosition;" +
    "      mat3 normalMatrix=mat3(transpose(inverse(u_mv_matrix)));" +
    "      vec3 tNorm=normalize(normalMatrix*vNormal);" +
    "      vec3 s=normalize(vec3(u_Light_Position-eye_coordinates));" +
    "      out_DiffusedColor=u_Ld*u_Kd*max(dot(s,tNorm),0.0);" +
    "   }" +
    "   gl_Position=u_p_matrix*u_mv_matrix*vPosition;" +
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
    "\n" +
    "precision highp float;" +
    "precision mediump int;" +
    "in vec3 out_DiffusedColor;" +
    "uniform int u_LKeyIsPressed;" +
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
    "   if(u_LKeyIsPressed==1)" +
    "   {" +
    "      FragColor=vec4(out_DiffusedColor,1.0);" +
    "   }" +
    "   else" +
    "   {" +
    "      FragColor=vec4(1.0,1.0,1.0,1.0);" +
    "   }" +
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

    mvUniform              = gl.getUniformLocation(shaderProgramObject, "u_mv_matrix");
	pUniform               = gl.getUniformLocation(shaderProgramObject, "u_p_matrix");
	LDUniform              = gl.getUniformLocation(shaderProgramObject, "u_Ld");
	KDUniform              = gl.getUniformLocation(shaderProgramObject, "u_Kd");
	LightPositionUniform   = gl.getUniformLocation(shaderProgramObject, "u_Light_Position");
	LKeyIsPressedUniform   = gl.getUniformLocation(shaderProgramObject, "u_LKeyIsPressed");
	
	sphere=new Mesh();
	makeSphere(sphere,2.0,30,30);

    perspectiveProjectionMatrix=mat4.create();
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    
    resize(window.innerWidth, window.innerHeight);
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
    
    mat4.translate(modelViewMatrix,translationMatrix,[0.0,0.0,-5.0]);
    mat4.multiply(modelViewMatrix,modelViewMatrix,translationMatrix);
    mat4.rotateX(rotationMatrix, rotationMatrix, degreeToRadian(rotateSphere));
    mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadian(rotateSphere));
    mat4.rotateZ(rotationMatrix, rotationMatrix, degreeToRadian(rotateSphere));
  
    mat4.multiply(modelViewMatrix,modelViewMatrix,rotationMatrix);
    
    gl.uniformMatrix4fv(mvUniform,
		false,
		modelViewMatrix);
	gl.uniformMatrix4fv(pUniform,
		false,
		perspectiveProjectionMatrix);
	if (gLighting == 1)
	{
		gl.uniform1i(LKeyIsPressedUniform,
			gLighting);
		
		gl.uniform3f(LDUniform, 1.0, 1.0,1.0);
		gl.uniform3f(KDUniform, 0.5, 0.5,0.5);
		var lightPosition = new Float32Array([ 0.0, 0.0, 2.0, 1.0]);
		gl.uniform4fv(LightPositionUniform, lightPosition);
	}
	else
	{
		gl.uniform1i(LKeyIsPressedUniform,gLighting);
	}
	sphere.draw();
    gl.useProgram(null);
    requestAnimationFrame(draw,canvas);

    //update
    rotateSphere -= 0.1;
	if (rotateSphere < 0)
	{
		rotateSphere = 360.0;
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
            if (gLighting == 0)
				gLighting = 1;
			else
				gLighting = 0;
            break;
    }
}

function mouseDown()
{}

function uninitialize()
{
	if(sphere)
	{
		sphere.deallocate();
		sphere=null;
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