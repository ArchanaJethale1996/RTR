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

var vertexShaderObject_PerFragment;
var fragmentShaderObject_PerFragment;
var shaderProgramObject_PerFragment;

var vertexShaderObject_PerVetex;
var fragmentShaderObject_PerVetex;
var shaderProgramObject_PerVetex;


var light_ambient=[0.0,0.0,0.0];
var light_diffuse=[1.0,1.0,1.0];
var light_specular=[1.0,1.0,1.0];
var light_position=[100.0,100.0,100.0,1.0];

var material_ambient=[0.0,0.0,0.0];
var material_diffuse=[1.0,1.0,1.0];
var material_specular=[1.0,1.0,1.0];
var material_shininess=50.0;
var rotateSphere=0.0;

var sphere=null;

var  mUniform_PerFragment,vUniform_PerFragment,pUniform_PerFragment;
var laUniform_PerFragment,ldUniform_PerFragment ,lsUniform_PerFragment;
var kaUniform_PerFragment,kdUniform_PerFragment,ksUniform_PerFragment,materialShininessUniform_PerFragment;
var LightPositionUniform_PerFragment, LKeyIsPressedUniform_PerFragment;


var mUniform_PerVertex, vUniform_PerVertex, pUniform_PerVertex;
var laUniform_PerVertex, ldUniform_PerVertex, lsUniform_PerVertex;
var kaUniform_PerVertex, kdUniform_PerVertex, ksUniform_PerVertex, materialShininessUniform_PerVertex;
var LightPositionUniform_PerVertex, LKeyIsPressedUniform_PerVertex;

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

var gLighting = 0;
var gAnimate = false;
var gPerFragment = false;
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
    var vertexShaderSourceCode_PerVertex =
    "#version 300 es" +
    "\n" +
    "precision mediump int;" +
    "in vec4 vPosition;" +
		"in vec3 vNormal;" +
		"uniform mat4 u_m_matrix;" +
		"uniform mat4 u_v_matrix;" +
		"uniform mat4 u_p_matrix;" +
		"uniform vec3 u_Ld;" +
		"uniform vec3 u_Kd;" +
		"uniform vec3 u_Ls;" +
		"uniform vec3 u_Ks;" +
		"uniform vec3 u_La;" +
		"uniform vec3 u_Ka;" +
		"uniform vec4 u_Light_Position;" +
		"uniform float u_MaterialShininess;" +
		"uniform int u_LKeyIsPressed;" +
		"out vec3 phong_ads_light;" +
		"void main(void)" +
		"{" +
		"if(u_LKeyIsPressed==1)" +
		"{" +
		"vec4 eye_coordinates=u_v_matrix*u_m_matrix*vPosition;" +
		"vec3 tNorm=normalize(mat3(u_v_matrix*u_m_matrix)*vNormal);" +
		"vec3 lightDirection=normalize(vec3(u_Light_Position-eye_coordinates));" +
		"float tndotld=max(dot(lightDirection,tNorm),0.0);" +
		"vec3 ReflectionVector=reflect(-lightDirection,tNorm);" +
		"vec3 viewerVector=normalize(vec3(-eye_coordinates.xyz));" +
		"vec3 ambient=u_La*u_Ka;" +
		"vec3 diffused=u_Ld*u_Kd*tndotld;" +
		"vec3 specular=u_Ls*u_Ks*pow(max(dot(ReflectionVector,viewerVector),0.0),u_MaterialShininess);" +
		"phong_ads_light = ambient+diffused+specular;" +
		"}" +
		"else" +
		"{" +
		"phong_ads_light=vec3(1.0,1.0,1.0);" +
		"}" +
		"gl_Position=u_p_matrix*u_v_matrix*u_m_matrix*vPosition;" +
		"}";

    vertexShaderObject_PerVertex = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject_PerVertex, vertexShaderSourceCode_PerVertex);
    gl.compileShader(vertexShaderObject_PerVertex);

    if (gl.getShaderParameter(vertexShaderObject_PerVertex, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(vertexShaderObject_PerVertex);
        if (error.length > 0) {
            alert(error);
            uninitialize();
        }
    }

    //fragment shader
    var fragmentShaderSourceCode_PerVertex =
    "#version 300 es" +
    "\n" +
    "precision highp float;" +
    "precision mediump int;" +
	"in vec3 phong_ads_light;" +
	"uniform int u_LKeyIsPressed;" +
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
	"	FragColor=vec4(phong_ads_light,1.0);" +
    "}";

    fragmentShaderObject_PerVertex = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject_PerVertex, fragmentShaderSourceCode_PerVertex);
    gl.compileShader(fragmentShaderObject_PerVertex);

    if (gl.getShaderParameter(fragmentShaderObject_PerVertex, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(fragmentShaderObject_PerVertex);
        if (error.length > 0) {
            alert(error);
            uninitialize();
        }
    }

    //shader program
    shaderProgramObject_PerVertex = gl.createProgram();
    gl.attachShader(shaderProgramObject_PerVertex, vertexShaderObject_PerVertex);
    gl.attachShader(shaderProgramObject_PerVertex, fragmentShaderObject_PerVertex);

    gl.bindAttribLocation(shaderProgramObject_PerVertex, WebGLMacros.AMC_ATTRIBUTE_VERTEX, "vPostion");
    gl.bindAttribLocation(shaderProgramObject_PerVertex, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

    gl.linkProgram(shaderProgramObject_PerVertex);
    if (gl.getProgramParameter(shaderProgramObject_PerVertex, gl.LINK_STATUS)) {
        var error = gl.getProgramInfoLog(shaderProgramObject_PerVertex);
        if (error.length > 0) {
            alert(error);
            uninitialize();
        }
    }

    mUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_m_matrix");
    vUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_v_matrix");
    pUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_p_matrix");
    ldUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Ld");

    lsUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Ls");

    laUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_La");

    kdUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Kd");

    kaUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Ka");

    ksUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Ks");

    materialShininessUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_MaterialShininess");
    LightPositionUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Light_Position");
    LKeyIsPressedUniform_PerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_LKeyIsPressed");


    //////////////////////////////////////////////////////////////////
    //Per Fragment
    //vertex Shader
    var vertexShaderSourceCode=
    "#version 300 es"+
    "\n" +
    "precision mediump int;" +
    "in vec4 vPosition;" +
		"in vec3 vNormal;" +
		"uniform mat4 u_m_matrix;" +
		"uniform mat4 u_v_matrix;" +
		"uniform mat4 u_p_matrix;" +
		"uniform int u_LKeyIsPressed;" +
		"out vec3 tNormVertexShader;" +
		"out vec4 eye_coordinatesVertexShader;" +
		"void main(void)" +
		"{" +
		"if(u_LKeyIsPressed==1)" +
		"{" +
		"eye_coordinatesVertexShader=u_v_matrix*u_m_matrix*vPosition;" +
		"tNormVertexShader=mat3(u_v_matrix*u_m_matrix)*vNormal;" +
		"}" +
		"gl_Position=u_p_matrix*u_v_matrix*u_m_matrix*vPosition;" +
		"}";
        
    vertexShaderObject_PerFragment=gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject_PerFragment,vertexShaderSourceCode);
    gl.compileShader(vertexShaderObject_PerFragment);

    if(gl.getShaderParameter(vertexShaderObject_PerFragment,gl.COMPILE_STATUS)==false)
    {
        var error=gl.getShaderInfoLog(vertexShaderObject_PerFragment);
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
	"in vec3 tNormVertexShader;" +
		"in vec4 eye_coordinatesVertexShader;" +
		"uniform vec3 u_Ld;" +
		"uniform vec3 u_Kd;" +
		"uniform vec3 u_Ls;" +
		"uniform vec3 u_Ks;" +
		"uniform vec3 u_La;" +
		"uniform vec3 u_Ka;" +
		"uniform vec4 u_Light_Position;" +
		"uniform float u_MaterialShininess;" +
		"uniform int u_LKeyIsPressed;" +
		"out vec4 FragColor;" +
		"void main(void)" +
		"{" +
		"if(u_LKeyIsPressed==1)" +
		"{" +
		"vec3 tNorm=normalize(tNormVertexShader);" +
		"vec3 lightDirection=normalize(vec3(u_Light_Position-eye_coordinatesVertexShader));" +
		"float tndotld=max(dot(lightDirection,tNorm),0.0);" +
		"vec3 ReflectionVector=reflect(-lightDirection,tNorm);" +
		"vec3 viewerVector=normalize(vec3(-eye_coordinatesVertexShader.xyz));" +
		"vec3 ambient=u_La*u_Ka;" +
		"vec3 diffused=u_Ld*u_Kd*tndotld;" +
		"vec3 specular=u_Ls*u_Ks*pow(max(dot(ReflectionVector,viewerVector),0.0),u_MaterialShininess);" +
		"vec3 phong_ads_light = ambient+diffused+specular;" +
		"FragColor=vec4(phong_ads_light,1.0);" +
		"}" +
		"else" +
		"{" +
		"FragColor=vec4(1.0,1.0,1.0,1.0);" +
		"}" +
		"}";
    
    fragmentShaderObject_PerFragment=gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject_PerFragment,fragmentShaderSourceCode);
    gl.compileShader(fragmentShaderObject_PerFragment);

    if(gl.getShaderParameter(fragmentShaderObject_PerFragment,gl.COMPILE_STATUS)==false)
    {
        var error=gl.getShaderInfoLog(fragmentShaderObject_PerFragment);
        if(error.length>0)
        {
            alert(error);
            uninitialize();
        }
    }

    //shader program
    shaderProgramObject_PerFragment=gl.createProgram();
    gl.attachShader(shaderProgramObject_PerFragment,vertexShaderObject_PerFragment);
    gl.attachShader(shaderProgramObject_PerFragment,fragmentShaderObject_PerFragment);

    gl.bindAttribLocation(shaderProgramObject_PerFragment,WebGLMacros.AMC_ATTRIBUTE_VERTEX,"vPostion");
    gl.bindAttribLocation(shaderProgramObject_PerFragment,WebGLMacros.AMC_ATTRIBUTE_NORMAL,"vNormal");
    
    gl.linkProgram(shaderProgramObject_PerFragment);
    if(gl.getProgramParameter(shaderProgramObject_PerFragment,gl.LINK_STATUS))
    {
        var error=gl.getProgramInfoLog(shaderProgramObject_PerFragment);
        if(error.length>0)
        {
            alert(error);
            uninitialize();
        }
    }

    mUniform_PerFragment              = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_m_matrix");
	vUniform_PerFragment              = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_v_matrix");
	pUniform_PerFragment               = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_p_matrix");
	ldUniform_PerFragment              = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Ld");

	lsUniform_PerFragment              = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Ls");

	laUniform_PerFragment              = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_La");

	kdUniform_PerFragment              = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Kd");

	kaUniform_PerFragment              = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Ka");

	ksUniform_PerFragment              = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Ks");

	materialShininessUniform_PerFragment= gl.getUniformLocation(shaderProgramObject_PerFragment, "u_MaterialShininess");
	LightPositionUniform_PerFragment   = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Light_Position");
	LKeyIsPressedUniform_PerFragment   = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_LKeyIsPressed");

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
    if (gPerFragment == true) {
        gl.useProgram(shaderProgramObject_PerFragment);
        var modelMatrix = mat4.create();
        var viewMatrix = mat4.create();
        var translationMatrix = mat4.create();
        var rotationMatrix = mat4.create();
        mat4.translate(modelMatrix, translationMatrix, [0.0, 0.0, -5.0]);

        mat4.rotateX(rotationMatrix, rotationMatrix, degreeToRadian(rotateSphere));

        mat4.multiply(modelMatrix, modelMatrix, rotationMatrix);
        gl.uniformMatrix4fv(mUniform_PerFragment,
               false,
               modelMatrix);
        gl.uniformMatrix4fv(vUniform_PerFragment,
                false,
                viewMatrix);
        gl.uniformMatrix4fv(pUniform_PerFragment,
            false,
            perspectiveProjectionMatrix);
        if (gLighting == 1) {
            gl.uniform1i(LKeyIsPressedUniform_PerFragment,
                gLighting);

            gl.uniform3fv(ldUniform_PerFragment, light_diffuse);
            gl.uniform3fv(lsUniform_PerFragment, light_specular);
            gl.uniform3fv(laUniform_PerFragment, light_ambient);

            gl.uniform3fv(kdUniform_PerFragment, material_diffuse);
            gl.uniform3fv(ksUniform_PerFragment, material_specular);
            gl.uniform3fv(kaUniform_PerFragment, material_ambient);

            gl.uniform1f(materialShininessUniform_PerFragment,
                material_shininess);

            gl.uniform4fv(LightPositionUniform_PerFragment, light_position);
        }
        else {
            gl.uniform1i(LKeyIsPressedUniform_PerFragment, gLighting);
        }
        sphere.draw();
        gl.useProgram(null);
    }
    else
    {
        gl.useProgram(shaderProgramObject_PerVertex);
        var modelMatrix = mat4.create();
        var viewMatrix = mat4.create();
        var translationMatrix = mat4.create();
        var rotationMatrix = mat4.create();
        mat4.translate(modelMatrix, translationMatrix, [0.0, 0.0, -5.0]);

        mat4.rotateX(rotationMatrix, rotationMatrix, degreeToRadian(rotateSphere));

        mat4.multiply(modelMatrix, modelMatrix, rotationMatrix);
        gl.uniformMatrix4fv(mUniform_PerVertex,
               false,
               modelMatrix);
        gl.uniformMatrix4fv(vUniform_PerVertex,
                false,
                viewMatrix);
        gl.uniformMatrix4fv(pUniform_PerVertex,
            false,
            perspectiveProjectionMatrix);
        if (gLighting == 1) {
            gl.uniform1i(LKeyIsPressedUniform_PerVertex,
                gLighting);

            gl.uniform3fv(ldUniform_PerVertex, light_diffuse);
            gl.uniform3fv(lsUniform_PerVertex, light_specular);
            gl.uniform3fv(laUniform_PerVertex, light_ambient);

            gl.uniform3fv(kdUniform_PerVertex, material_diffuse);
            gl.uniform3fv(ksUniform_PerVertex, material_specular);
            gl.uniform3fv(kaUniform_PerVertex, material_ambient);

            gl.uniform1f(materialShininessUniform_PerVertex,
                material_shininess);

            gl.uniform4fv(LightPositionUniform_PerVertex, light_position);
        }
        else {
            gl.uniform1i(LKeyIsPressedUniform_PerVertex, gLighting);
        }
        sphere.draw();
        gl.useProgram(null);
    }
    requestAnimationFrame(draw,canvas);

	rotateSphere = rotateSphere+1.0;
	if(rotateSphere>=360.0)
	{
		rotateSphere=0.0;
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
	    case 65:
            if (gAnimate == true)
				gAnimate = false;
			else
				gAnimate = true;
            break;
        case 77:
            if (gPerFragment == true)
                gPerFragment = false;
            else
                gPerFragment = true;
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
    if(shaderProgramObject_PerFragment)
    {
        if(fragmentShaderObject_PerFragment)
        {
            gl.detachShader(shaderProgramObject_PerFragment,fragmentShaderObject_PerFragment);
            gl.deleteShader(fragmentShaderObject_PerFragment);
            fragmentShaderObject_PerFragment=null;
        }
        
        if(vertexShaderObject_PerFragment)
        {
            gl.detachShader(shaderProgramObject_PerFragment,vertexShaderObject_PerFragment);
            gl.deleteShader(vertexShaderObject_PerFragment);
            vertexShaderObject_PerFragment=null;
        }
        gl.deleteProgram(shaderProgramObject_PerFragment);
        shaderProgramObject_PerFragment=null;
    }
}