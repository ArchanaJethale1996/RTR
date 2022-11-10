var canvas=null;
var gl=null;  //webgl contex
var bFullscreen=false;
var canvas_original_width;
var canvas_original_height;
var gWidth=1.0,gHeight=1.0;
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


var lightAmbient=[0.0,0.0,0.0];
var lightDiffused=[1.0,1.0,1.0];
var lightSpecular=[1.0,1.0,1.0];
var lightPosition=[0.0,3.0,3.0,1.0];

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

var angleOfXRotation = 0.0;
var angleOfYRotation = 0.0;
var angleOfZRotation = 0.0;
var KeyPressed = 1;

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
      gWidth=  canvas.width ;
	gHeight=canvas.height;
       
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
		case 99:
			KeyPressed = 1;
			lightPosition[0] = 0.0;
			lightPosition[1] = 0.0;
			lightPosition[2] = 100.0;
			angleOfXRotation = 0.0;
			break;
		case 89:
			KeyPressed = 2;
			lightPosition[1] = 0.0;
			lightPosition[2] = 0.0;
			lightPosition[0] = 100.0;
			angleOfYRotation = 0.0;
			break;
		case 90:
			KeyPressed = 3;
			lightPosition[1] = 100.0;
			lightPosition[0] = 0.0;
			lightPosition[2] = 0.0;
			angleOfZRotation = 0.0;
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


function draw24SpherePerFragment()
{
	var materialAmbient=[];
	var materialDiffused=[];
	var materialSpecular=[];
	var materialShininess=[];

	var modelMatrix;
	var viewMatrix;
	var translationMatrix;
	var scaleMatrix;
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	scaleMatrix= mat4.create();
	
	//1st sphere 1st col emrald
	materialAmbient[0] = 0.0215;
	materialAmbient[1] = 0.1745;
	materialAmbient[2] = 0.0215;


	materialDiffused[0] = 0.07568;
	materialDiffused[1] = 0.61424;
	materialDiffused[2] = 0.07568;

	materialSpecular[0] = 0.633;
	materialSpecular[1] = 0.727811;
	materialSpecular[2] = 0.633;

	materialShininess = 0.6 * 128;
	gl.uniform3fv(kaUniform_PerFragment,  materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 0, (canvas.height / 4) * 3,canvas.width/6,canvas.height/6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  




	////2nd sphere on  1st col,jade

	materialAmbient[0] = 0.135;
	materialAmbient[1] = 0.2225;
	materialAmbient[2] = 0.1575;
	materialAmbient[3] = 1.0;
	
	materialDiffused[0] = 0.54;
	materialDiffused[1] = 0.89;
	materialDiffused[2] = 0.63;
	
	
	materialSpecular[0] = 0.316228;
	materialSpecular[1] = 0.316228;
	materialSpecular[2] = 0.316228;
	

	materialShininess = 0.1 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 1, (canvas.height / 4) * 3, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd sphere on  1st col,obsidian
	materialAmbient[0] = 0.05375;
	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.06625;
	
	
	materialDiffused[0] = 0.18275;
	materialDiffused[1] = 0.17;
	materialDiffused[2] = 0.22525;
	
	
	materialSpecular[0] = 0.332741;
	materialSpecular[1] = 0.328634;
	materialSpecular[2] = 0.346435;
	
	
	materialShininess = 0.3 * 128;
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width/ 6) * 2, (canvas.height / 4) * 3,  canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////4th sphere on  1st col,pearl

	materialAmbient[0] = 0.25;
	materialAmbient[1] = 0.20725;
	materialAmbient[2] = 0.20725;
	

	materialDiffused[0] = 1.0;
	materialDiffused[1] = 0.829;
	materialDiffused[2] = 0.829;
	

	materialSpecular[0] = 0.296648;
	materialSpecular[1] = 0.296648;
	materialSpecular[2] = 0.296648;
	

	materialShininess = 0.88 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 3, (canvas.height / 4) * 3, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////5th sphere on  1st col,ruby

	materialAmbient[0] = 0.1745;
	materialAmbient[1] = 0.01175;
	materialAmbient[2] = 0.01175;
	
	
	materialDiffused[0] = 0.61424;
	materialDiffused[1] = 0.04136;
	materialDiffused[2] = 0.04136;
	
	
	materialSpecular[0] = 0.727811;
	materialSpecular[1] = 0.626959;
	materialSpecular[2] = 0.626959;
	
	
	materialShininess = 0.6 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 4, (canvas.height / 4) * 3,canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	////6th sphere on  1st col,turquoise

	materialAmbient[0] = 0.1;
	materialAmbient[1] = 0.18725;
	materialAmbient[2] = 0.1745;
	

	materialDiffused[0] = 0.396;
	materialDiffused[1] = 0.074151;
	materialDiffused[2] = 0.69102;
	

	materialSpecular[0] = 0.297254;
	materialSpecular[1] = 0.30829;
	materialSpecular[2] = 0.306678;
	

	materialShininess = 0.1 * 128;
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 5, (canvas.height / 4) * 3, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  



	////2nd Col

	//1st sphere 2nd col brass
	materialAmbient[0] = 0.329412;
	materialAmbient[1] = 0.223529;
	materialAmbient[2] = 0.027451;
	
	
	materialDiffused[0] = 0.780392;
	materialDiffused[1] = 0.568627;
	materialDiffused[2] = 0.113725;
	
	
	materialSpecular[0] = 0.992157;
	materialSpecular[1] = 0.941176;
	materialSpecular[2] = 0.807843;
	
	
	materialShininess = 0.21794872 * 128;
	
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 0, (canvas.height / 4) * 2, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////2nd sphere on  2nd col,bronze

	materialAmbient[0] = 0.2125;
	materialAmbient[1] = 0.1275;
	materialAmbient[2] = 0.054;
	
	
	materialDiffused[0] = 0.714;
	materialDiffused[1] = 0.4284;
	materialDiffused[2] = 0.18144;
	
	
	materialSpecular[0] = 0.393548;
	materialSpecular[1] = 0.271906;
	materialSpecular[2] = 0.166721;
	
	
	materialShininess = 0.2 * 128;
	
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((gWidth / 6) * 1, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd sphere on  2nd col,chrome

	materialAmbient[0] = 0.25;
	materialAmbient[1] = 0.25;
	materialAmbient[2] = 0.25;
	
	
	materialDiffused[0] = 0.4;
	materialDiffused[1] = 0.4;
	materialDiffused[2] = 0.4;
	
	
	materialSpecular[0] = 0.774597;
	materialSpecular[1] = 0.774597;
	materialSpecular[2] = 0.774597;
	
	
	materialShininess = 0.6 * 128;
	

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 2, (canvas.height / 4) * 2, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////4th sphere on  2nd col,copper

	materialAmbient[0] = 0.19125;
	materialAmbient[1] = 0.0735;
	materialAmbient[2] = 0.0225;
	
	
	materialDiffused[0] = 0.7038;
	materialDiffused[1] = 0.27048;
	materialDiffused[2] = 0.0828;
	
	
	materialSpecular[0] = 0.256777;
	materialSpecular[1] = 0.137622;
	materialSpecular[2] = 0.086014;
	
	
	materialShininess = 0.1 * 128;
	

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 3, (canvas.height / 4) * 2, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////5th sphere on  2nd col,gold

	materialAmbient[0] = 0.2472;
	materialAmbient[1] = 0.1995;
	materialAmbient[2] = 0.0745;
	

	materialDiffused[0] = 0.75164;
	materialDiffused[1] = 0.60648;
	materialDiffused[2] = 0.22648;
	
	
	materialSpecular[0] = 0.628281;
	materialSpecular[1] = 0.555802;
	materialSpecular[2] = 0.366065;
	

	materialShininess = 0.4 * 128;


	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((gWidth / 6) * 4, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////6th sphere on  2nd col,silver

	materialAmbient[0] = 0.19225;
	materialAmbient[1] = 0.19225;
	materialAmbient[2] = 0.19225;
	
	
	materialDiffused[0] = 0.5074;
	materialDiffused[1] = 0.5074;
	materialDiffused[2] = 0.5074;
	
	
	materialSpecular[0] = 0.508273;
	materialSpecular[1] = 0.508273;
	materialSpecular[2] = 0.508273;
	
	
	materialShininess = 0.4 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 5, (canvas.height / 4) * 2, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd col
	//1st sphere 3rd col black
	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	
	
	materialDiffused[0] = 0.0;
	materialDiffused[1] = 0.0;
	materialDiffused[2] = 0.0;
	
	
	materialSpecular[0] = 0.5;
	materialSpecular[1] = 0.5;
	materialSpecular[2] = 0.5;
	
	
	materialShininess = 0.25 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 0, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	////2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.1;
	materialAmbient[2] = 0.06;
	
	
	materialDiffused[0] = 0.0;
	materialDiffused[1] = 0.50980392;
	materialDiffused[2] = 0.5098392;
	
	
	materialSpecular[0] = 0.50196078;
	materialSpecular[1] = 0.50196078;
	materialSpecular[2] = 0.50196078;
	
	
	materialShininess = 0.25 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 1, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd sphere on  3rd col,green

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	
	
	materialDiffused[0] = 0.1;
	materialDiffused[1] = 0.35;
	materialDiffused[2] = 0.1;
	
	
	materialSpecular[0] = 0.45;
	materialSpecular[1] = 0.55;
	materialSpecular[2] = 0.45;
	
	
	materialShininess = 0.25 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 2, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	//
	////4th sphere on  3rd col,red
	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	
	
	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.0;
	materialDiffused[2] = 0.0;
	
	
	materialSpecular[0] = 0.7;
	materialSpecular[1] = 0.6;
	materialSpecular[2] = 0.6;
	
	
	materialShininess = 0.25 * 128;

	
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 3, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	////5th sphere on  3rd col,white

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	
	
	materialDiffused[0] = 0.55;
	materialDiffused[1] = 0.55;
	materialDiffused[2] = 0.55;
	
	
	materialSpecular[0] = 0.70;
	materialSpecular[1] = 0.70;
	materialSpecular[2] = 0.70;
	
	
	materialShininess = 0.25 * 128;
	

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 4, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	
	
	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.0;
	
	
	materialSpecular[0] = 0.60;
	materialSpecular[1] = 0.60;
	materialSpecular[2] = 0.50;
	
	
	materialShininess = 0.25 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 5, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	////4th col
	////1st sphere 3rd col black
	materialAmbient[0] = 0.02;
	materialAmbient[1] = 0.02;
	materialAmbient[2] = 0.02;
	
	
	materialDiffused[0] = 0.01;
	materialDiffused[1] = 0.01;
	materialDiffused[2] = 0.01;
	
	
	materialSpecular[0] = 0.4;
	materialSpecular[1] = 0.4;
	materialSpecular[2] = 0.4;
	
	
	materialShininess = 0.78125 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 0, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	



	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.05;
	
	
	materialDiffused[0] = 0.4;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.5;
	
	
	materialSpecular[0] = 0.04;
	materialSpecular[1] = 0.7;
	materialSpecular[2] = 0.7;
	
	
	materialShininess = 0.078125 * 128;
	
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 1, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd sphere on  3rd col,green
	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.0;
	
	
	materialDiffused[0] = 0.4;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.4;
	
	
	materialSpecular[0] = 0.04;
	materialSpecular[1] = 0.7;
	materialSpecular[2] = 0.04;
	
	
	materialShininess = 0.078125 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 2, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	//////4th sphere on  3rd col,red

	materialAmbient[0] = 0.05;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	

	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.4;
	materialDiffused[2] = 0.4;
	

	materialSpecular[0] = 0.7;
	materialSpecular[1] = 0.04;
	materialSpecular[2] = 0.04;
	

	materialShininess = 0.078125 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 3, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	



	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	//////5th sphere on  3rd col,white

	materialAmbient[0] = 0.05;
	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.05;
	
	
	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.5;
	
	
	materialSpecular[0] = 0.70;
	materialSpecular[1] = 0.70;
	materialSpecular[2] = 0.70;
	
	
	materialShininess = 0.078125 * 128;
	
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);


	 gl.viewport((canvas.width / 6) * 4, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		
false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.05;
	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.0;
	
	
	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.4;
	
	
	materialSpecular[0] = 0.70;
	materialSpecular[1] = 0.70;
	materialSpecular[2] = 0.04;
	
	
	materialShininess = 0.078125 * 128;
	
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerFragment, materialAmbient);
	gl.uniform3fv(kdUniform_PerFragment,  materialDiffused);
	gl.uniform3fv(ksUniform_PerFragment,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerFragment, materialShininess);

	 gl.viewport((canvas.width / 6) * 5, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);

	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerFragment,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerFragment,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerFragment,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

}


function draw24SpherePerVertex()
{
	var materialAmbient=[4];
	var materialDiffused=[4];
	var materialSpecular=[4];
	var materialShininess;

	var modelMatrix;
	var viewMatrix;
	var translationMatrix;
	var scaleMatrix;
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();
	//1st sphere 1st col emrald
	materialAmbient[0] = 0.0215;
	materialAmbient[1] = 0.1745;
	materialAmbient[2] = 0.0215;
	

	materialDiffused[0] = 0.07568;
	materialDiffused[1] = 0.61424;
	materialDiffused[2] = 0.07568;
	

	materialSpecular[0] = 0.633;
	materialSpecular[1] = 0.727811;
	materialSpecular[2] = 0.633;
	

	materialShininess = 0.6 * 128;
	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 0, (canvas.height / 4) * 3, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  




	////2nd sphere on  1st col,jade

	materialAmbient[0] = 0.135;
	materialAmbient[1] = 0.2225;
	materialAmbient[2] = 0.1575;
	

	materialDiffused[0] = 0.54;
	materialDiffused[1] = 0.89;
	materialDiffused[2] = 0.63;
	

	materialSpecular[0] = 0.316228;
	materialSpecular[1] = 0.316228;
	materialSpecular[2] = 0.316228;
	

	materialShininess = 0.1 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 1, (canvas.height / 4) * 3, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd sphere on  1st col,obsidian
	materialAmbient[0] = 0.05375;
	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.06625;
	

	materialDiffused[0] = 0.18275;
	materialDiffused[1] = 0.17;
	materialDiffused[2] = 0.22525;
	

	materialSpecular[0] = 0.332741;
	materialSpecular[1] = 0.328634;
	materialSpecular[2] = 0.346435;
	

	materialShininess = 0.3 * 128;
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 2, (canvas.height / 4) * 3, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////4th sphere on  1st col,pearl

	materialAmbient[0] = 0.25;
	materialAmbient[1] = 0.20725;
	materialAmbient[2] = 0.20725;
	

	materialDiffused[0] = 1.0;
	materialDiffused[1] = 0.829;
	materialDiffused[2] = 0.829;
	

	materialSpecular[0] = 0.296648;
	materialSpecular[1] = 0.296648;
	materialSpecular[2] = 0.296648;
	

	materialShininess = 0.88 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 3, (canvas.height / 4) * 3, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////5th sphere on  1st col,ruby

	materialAmbient[0] = 0.1745;
	materialAmbient[1] = 0.01175;
	materialAmbient[2] = 0.01175;
	

	materialDiffused[0] = 0.61424;
	materialDiffused[1] = 0.04136;
	materialDiffused[2] = 0.04136;
	

	materialSpecular[0] = 0.727811;
	materialSpecular[1] = 0.626959;
	materialSpecular[2] = 0.626959;
	

	materialShininess = 0.6 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 4, (canvas.height / 4) * 3, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	////6th sphere on  1st col,turquoise

	materialAmbient[0] = 0.1;
	materialAmbient[1] = 0.18725;
	materialAmbient[2] = 0.1745;
	

	materialDiffused[0] = 0.396;
	materialDiffused[1] = 0.074151;
	materialDiffused[2] = 0.69102;
	

	materialSpecular[0] = 0.297254;
	materialSpecular[1] = 0.30829;
	materialSpecular[2] = 0.306678;
	

	materialShininess = 0.1 * 128;
	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 5, (canvas.height / 4) * 3, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	



	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  



	////2nd Col

	//1st sphere 2nd col brass
	materialAmbient[0] = 0.329412;
	materialAmbient[1] = 0.223529;
	materialAmbient[2] = 0.027451;
	

	materialDiffused[0] = 0.780392;
	materialDiffused[1] = 0.568627;
	materialDiffused[2] = 0.113725;
	

	materialSpecular[0] = 0.992157;
	materialSpecular[1] = 0.941176;
	materialSpecular[2] = 0.807843;
	

	materialShininess = 0.21794872 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 0, (canvas.height / 4) * 2, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////2nd sphere on  2nd col,bronze

	materialAmbient[0] = 0.2125;
	materialAmbient[1] = 0.1275;
	materialAmbient[2] = 0.054;
	

	materialDiffused[0] = 0.714;
	materialDiffused[1] = 0.4284;
	materialDiffused[2] = 0.18144;
	

	materialSpecular[0] = 0.393548;
	materialSpecular[1] = 0.271906;
	materialSpecular[2] = 0.166721;
	

	materialShininess = 0.2 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((gWidth / 6) * 1, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd sphere on  2nd col,chrome

	materialAmbient[0] = 0.25;
	materialAmbient[1] = 0.25;
	materialAmbient[2] = 0.25;
	

	materialDiffused[0] = 0.4;
	materialDiffused[1] = 0.4;
	materialDiffused[2] = 0.4;
	

	materialSpecular[0] = 0.774597;
	materialSpecular[1] = 0.774597;
	materialSpecular[2] = 0.774597;
	

	materialShininess = 0.6 * 128;


	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 2, (canvas.height / 4) * 2, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////4th sphere on  2nd col,copper

	materialAmbient[0] = 0.19125;
	materialAmbient[1] = 0.0735;
	materialAmbient[2] = 0.0225;
	

	materialDiffused[0] = 0.7038;
	materialDiffused[1] = 0.27048;
	materialDiffused[2] = 0.0828;
	

	materialSpecular[0] = 0.256777;
	materialSpecular[1] = 0.137622;
	materialSpecular[2] = 0.086014;
	

	materialShininess = 0.1 * 128;


	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 3, (canvas.height / 4) * 2, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////5th sphere on  2nd col,gold

	materialAmbient[0] = 0.2472;
	materialAmbient[1] = 0.1995;
	materialAmbient[2] = 0.0745;
	

	materialDiffused[0] = 0.75164;
	materialDiffused[1] = 0.60648;
	materialDiffused[2] = 0.22648;
	

	materialSpecular[0] = 0.628281;
	materialSpecular[1] = 0.555802;
	materialSpecular[2] = 0.366065;
	

	materialShininess = 0.4 * 128;


	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((gWidth / 6) * 4, (gHeight / 4) * 2, gWidth / 6, gHeight / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	



	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////6th sphere on  2nd col,silver

	materialAmbient[0] = 0.19225;
	materialAmbient[1] = 0.19225;
	materialAmbient[2] = 0.19225;
	

	materialDiffused[0] = 0.5074;
	materialDiffused[1] = 0.5074;
	materialDiffused[2] = 0.5074;
	

	materialSpecular[0] = 0.508273;
	materialSpecular[1] = 0.508273;
	materialSpecular[2] = 0.508273;
	

	materialShininess = 0.4 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 5, (canvas.height / 4) * 2, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	



	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd col
	//1st sphere 3rd col black
	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	

	materialDiffused[0] = 0.0;
	materialDiffused[1] = 0.0;
	materialDiffused[2] = 0.0;
	

	materialSpecular[0] = 0.5;
	materialSpecular[1] = 0.5;
	materialSpecular[2] = 0.5;
	

	materialShininess = 0.25 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 0, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
		mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	



	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	////2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.1;
	materialAmbient[2] = 0.06;
	

	materialDiffused[0] = 0.0;
	materialDiffused[1] = 0.50980392;
	materialDiffused[2] = 0.5098392;
	

	materialSpecular[0] = 0.50196078;
	materialSpecular[1] = 0.50196078;
	materialSpecular[2] = 0.50196078;
	

	materialShininess = 0.25 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 1, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	


	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd sphere on  3rd col,green

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	

	materialDiffused[0] = 0.1;
	materialDiffused[1] = 0.35;
	materialDiffused[2] = 0.1;
	

	materialSpecular[0] = 0.45;
	materialSpecular[1] = 0.55;
	materialSpecular[2] = 0.45;
	

	materialShininess = 0.25 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 2, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	//
	////4th sphere on  3rd col,red
	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	

	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.0;
	materialDiffused[2] = 0.0;
	

	materialSpecular[0] = 0.7;
	materialSpecular[1] = 0.6;
	materialSpecular[2] = 0.6;
	

	materialShininess = 0.25 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 3, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);	

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	////5th sphere on  3rd col,white

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	

	materialDiffused[0] = 0.55;
	materialDiffused[1] = 0.55;
	materialDiffused[2] = 0.55;
	

	materialSpecular[0] = 0.70;
	materialSpecular[1] = 0.70;
	materialSpecular[2] = 0.70;
	

	materialShininess = 0.25 * 128;


	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 4, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);
	 

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	

	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.0;
	

	materialSpecular[0] = 0.60;
	materialSpecular[1] = 0.60;
	materialSpecular[2] = 0.5;
	

	materialShininess = 0.25 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 5, (canvas.height / 4) * 1, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);
	 
	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	////4th col
	////1st sphere 3rd col black
	materialAmbient[0] = 0.02;
	materialAmbient[1] = 0.02;
	materialAmbient[2] = 0.02;
	

	materialDiffused[0] = 0.01;
	materialDiffused[1] = 0.01;
	materialDiffused[2] = 0.01;
	

	materialSpecular[0] = 0.4;
	materialSpecular[1] = 0.4;
	materialSpecular[2] = 0.4;
	

	materialShininess = 0.78125 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	 scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 0, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);
	 

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////2nd sphere on  3rd col,cyan

	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.05;
	

	materialDiffused[0] = 0.4;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.5;
	

	materialSpecular[0] = 0.04
	materialSpecular[1] = 0.7;
	materialSpecular[2] = 0.7;
	

	materialShininess = 0.078125 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 1, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);
	 

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////3rd sphere on  3rd col,green
	materialAmbient[0] = 0.0;
	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.0;
	

	materialDiffused[0] = 0.4;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.4;
	

	materialSpecular[0] = 0.04;
	materialSpecular[1] = 0.7;
	materialSpecular[2] = 0.04;
	

	materialShininess = 0.078125 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 2, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);
	 

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  

	//////4th sphere on  3rd col,red

	materialAmbient[0] = 0.05;
	materialAmbient[1] = 0.0;
	materialAmbient[2] = 0.0;
	

	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.4;
	materialDiffused[2] = 0.4;
	

	materialSpecular[0] = 0.7;
	materialSpecular[1] = 0.04;
	materialSpecular[2] = 0.04;
	

	materialShininess = 0.078125 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 3, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);
	 

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	//////5th sphere on  3rd col,white

	materialAmbient[0] = 0.05;
	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.05;
	

	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.5;
	

	materialSpecular[0] = 0.70;
	materialSpecular[1] = 0.70;
	materialSpecular[2] = 0.70;
	

	materialShininess = 0.078125 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);


	 gl.viewport((canvas.width / 6) * 4, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
////	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);

	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);

	 
	 
	sphere.draw();
	  


	////6th sphere on  3rd col,yellow plastic

	materialAmbient[0] = 0.05;

	materialAmbient[1] = 0.05;
	materialAmbient[2] = 0.0;
	

	materialDiffused[0] = 0.5;
	materialDiffused[1] = 0.5;
	materialDiffused[2] = 0.4;
	

	materialSpecular[0] = 0.70;
	materialSpecular[1] = 0.70;
	materialSpecular[2] = 0.04;
	

	materialShininess = 0.078125 * 128;

	modelMatrix = mat4.create();
	viewMatrix = mat4.create();
	translationMatrix = mat4.create();
	scaleMatrix = mat4.create();

	gl.uniform3fv(kaUniform_PerVertex,  materialAmbient);
	gl.uniform3fv(kdUniform_PerVertex,  materialDiffused);
	gl.uniform3fv(ksUniform_PerVertex,  materialSpecular);
	gl.uniform1f(materialShininessUniform_PerVertex, materialShininess);

	 gl.viewport((canvas.width / 6) * 5, (canvas.height / 4) * 0, canvas.width / 6, canvas.height / 6);
	mat4.translate(modelMatrix,translationMatrix,[0.0, 0.0, -5.0]);
//	 mat4.scale(modelMatrix, scaleMatrix ,0.7, 0.7, 0.7);
	 
	gl.uniformMatrix4fv(mUniform_PerVertex,
		false,
		modelMatrix);
	gl.uniformMatrix4fv(vUniform_PerVertex,
		false,
		viewMatrix);
	gl.uniformMatrix4fv(pUniform_PerVertex,
		false,
		perspectiveProjectionMatrix);
	sphere.draw();
	  

}


function draw()
{
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    if (gPerFragment == true) {
        gl.useProgram(shaderProgramObject_PerFragment);
		if (gLighting == 1)
		{
			gl.uniform1i(LKeyIsPressedUniform_PerFragment,
				1);

			if (KeyPressed == 1)
			{
				lightPosition[0] = 0.0;
				lightPosition[1] = Math.sin(rotateSphere) * 100.0 - 3.0;
				lightPosition[2] = Math.cos(rotateSphere) * 100.0 - 3.0;
			}

			if (KeyPressed == 2)
			{
				lightPosition[0] = Math.sin(rotateSphere) * 100.0 - 3.0;
				lightPosition[1] = 0.0;
				lightPosition[2] = Math.cos(rotateSphere) * 100.0 - 3.0;
			}

			if (KeyPressed == 3)
			{
				lightPosition[0] = Math.sin(rotateSphere) * 100.0 - 3.0;
				lightPosition[1] = Math.cos(rotateSphere) * 100.0 - 3.0;
				lightPosition[2] = 0.0;
			}
			gl.uniform4fv(LightPositionUniform_PerFragment,
lightPosition);
			gl.uniform3fv(laUniform_PerFragment, lightAmbient);
			gl.uniform3fv(ldUniform_PerFragment, lightDiffused);
			gl.uniform3fv(lsUniform_PerFragment, lightSpecular);
			//sphere.draw();
			draw24SpherePerFragment();
		}
		else
		{

			var modelMatrix;
			var viewMatrix;
			var translationMatrix;

			modelMatrix = mat4.create();
			viewMatrix = mat4.create();
			translationMatrix = mat4.create();

			mat4.translate(modelMatrix,translationMatrix ,[0.0, 0.0, -6.0]);
			
			gl.uniformMatrix4fv(mUniform_PerFragment,
				false,
				modelMatrix);
			gl.uniformMatrix4fv(vUniform_PerFragment,
				false,
				viewMatrix);
			gl.uniformMatrix4fv(pUniform_PerFragment,
				false,
				perspectiveProjectionMatrix);

			gl.uniform1i(LightPositionUniform_PerFragment, 0);
			draw24SpherePerFragment();
		}
		gl.useProgram(null);
    }
    else
    {
        gl.useProgram(shaderProgramObject_PerVertex);
		if (gLighting == 1)
		{
			gl.uniform1i(LKeyIsPressedUniform_PerVertex,
				1);

			if (KeyPressed == 1)
			{
				lightPosition[0] = 0.0;
				lightPosition[1] = Math.sin(rotateSphere) * 100.0 - 3.0;
				lightPosition[2] = Math.cos(rotateSphere) * 100.0 - 3.0;
			}

			if (KeyPressed == 2)
			{
				lightPosition[0] = Math.sin(rotateSphere) * 100.0 - 3.0;
				lightPosition[1] = 0.0;
				lightPosition[2] = Math.cos(rotateSphere) * 100.0 - 3.0;
			}

			if (KeyPressed == 3)
			{
				lightPosition[0] = Math.sin(rotateSphere) * 100.0 - 3.0;
				lightPosition[1] = Math.cos(rotateSphere) * 100.0 - 3.0;
				lightPosition[2] = 0.0;
			}
			gl.uniform4fv(LightPositionUniform_PerVertex,  lightPosition);
			gl.uniform3fv(laUniform_PerVertex,  lightAmbient);
			gl.uniform3fv(ldUniform_PerVertex,  lightDiffused);
			gl.uniform3fv(lsUniform_PerVertex,  lightSpecular);
			//sphere.draw();
			draw24SpherePerVertex();
		}
		else
		{

			var modelMatrix;
			var viewMatrix;
			var translationMatrix;

			modelMatrix = mat4.create();
			viewMatrix = mat4.create();
			translationMatrix = mat4.create();

			mat4.translate(modelMatrix,translationMatrix ,[0.0, 0.0, -6.0]);
			
			gl.uniformMatrix4fv(mUniform_PerVertex,
				false,
				modelMatrix);
			gl.uniformMatrix4fv(vUniform_PerVertex,
				false,
				viewMatrix);
			gl.uniformMatrix4fv(pUniform_PerVertex,
				false,
				perspectiveProjectionMatrix);

			gl.uniform1i(LKeyIsPressedUniform_PerVertex, 0);
//sphere.draw();
			draw24SpherePerVertex();
		}
		gl.useProgram(null);
    }
    requestAnimationFrame(draw,canvas);

	rotateSphere = rotateSphere+0.1;
	if(rotateSphere>=360.0)
	{
		rotateSphere=0.0;
	}
}
