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

var lightAmbientZero=[1.0,0.0,0.0];
var lightDiffusedZero=[1.0,0.0,0.0];
var lightSpecularZero=[1.0,0.0,0.0];
var lightPositionZero=[-2.0,0.0,0.0,1.0];
var lightAngleZero = 0.0;

var lightAmbientOne=[0.0,0.0,1.0];
var lightDiffusedOne=[0.0,0.0,1.0];
var lightSpecularOne=[0.0,0.0,1.0];
var lightPositionOne=[2.0,0.0,0.0,1.0];
var lightAngleOne = 0.0;

var materialAmbient=[0.0,0.0,0.0];
var materialDiffused=[1.0,1.0,1.0];
var materialSpecular=[1.0,1.0,1.0];
var materialShininess=128.0;
var rotatePyramid=0.0;


var  vao_Pyramid;
var vbo_Position_Pyramid;
var vbo_Normal_Pyramid;
var mUniformPerVertex, vUniformPerVertex, pUniformPerVertex,KDUniformPerVertex,  KAUniformPerVertex, KSUniformPerVertex, MaterialShininessUniformPerVertex, LKeyIsPressedUniformPerVertex;
var 	LDUniformOnePerVertex, LAUniformOnePerVertex, LSUniformOnePerVertex, LightPositionUniformOnePerVertex, LDUniformTwoPerVertex, LAUniformTwoPerVertex, LSUniformTwoPerVertex, LightPositionUniformTwoPerVertex;

var mUniformPerFragment, vUniformPerFragment, pUniformPerFragment, KDUniformPerFragment, KAUniformPerFragment, KSUniformPerFragment, MaterialShininessUniformPerFragment, LKeyIsPressedUniformPerFragment;
var 	LDUniformOnePerFragment, LAUniformOnePerFragment, LSUniformOnePerFragment, LightPositionUniformOnePerFragment, LDUniformTwoPerFragment, LAUniformTwoPerFragment, LSUniformTwoPerFragment, LightPositionUniformTwoPerFragment;

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
		"uniform vec3 u_Kd;" +
		"uniform vec3 u_Ks;" +
		"uniform vec3 u_Ka;" +
		"uniform vec3 u_LdOne;" +
		"uniform vec3 u_LsOne;" +
		"uniform vec3 u_LaOne;" +
		"uniform vec4 u_Light_PositionOne;" +
		"uniform vec3 u_LdTwo;" +
		"uniform vec3 u_LsTwo;" +
		"uniform vec3 u_LaTwo;" +
		"uniform vec4 u_Light_PositionTwo;" +
		"uniform float u_MaterialShininess;" +
		"uniform int u_LKeyIsPressed;" +
		"out vec3 phong_ads_light;" +
		"void main(void)" +
		"{" +
		"if(u_LKeyIsPressed==1)" +
		"{" +
		"vec4 eye_coordinates=u_v_matrix*u_m_matrix*vPosition;" +
		"vec3 tNorm=normalize(mat3(u_v_matrix*u_m_matrix)*vNormal);" +
		"vec3 lightDirectionOne=normalize(vec3(u_Light_PositionOne-eye_coordinates));" +
		"vec3 lightDirectionTwo=normalize(vec3(u_Light_PositionTwo-eye_coordinates));" +
		"float tndotldOne=max(dot(lightDirectionOne,tNorm),0.0);" +
		"float tndotldTwo=max(dot(lightDirectionTwo,tNorm),0.0);" +
		"vec3 ReflectionVectorOne=reflect(-lightDirectionOne,tNorm);" +
		"vec3 ReflectionVectorTwo=reflect(lightDirectionTwo,tNorm);" +
		"vec3 viewerVector=normalize(vec3(-eye_coordinates.xyz));" +
		"vec3 ambientOne=u_LaOne*u_Ka;" +
		"vec3 diffusedOne=u_LdOne*u_Kd*tndotldOne;" +
		"vec3 specularOne=u_LsOne*u_Ks*pow(max(dot(ReflectionVectorOne,viewerVector),0.0),u_MaterialShininess);" +
		"vec3 ambientTwo=u_LaTwo*u_Ka;" +
		"vec3 diffusedTwo=u_LdTwo*u_Kd*tndotldTwo;" +
		"vec3 specularTwo=u_LsTwo*u_Ks*pow(max(dot(ReflectionVectorTwo,viewerVector),0.0),u_MaterialShininess);" +
		"phong_ads_light = ambientOne+diffusedOne+specularOne+ambientTwo+diffusedTwo+specularTwo;" +
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
		"if(u_LKeyIsPressed==1)" +
		"{" +
		"FragColor=vec4(phong_ads_light,1.0);" +
		"}" +
		"else" +
		"{" +
		"FragColor=vec4(1.0,1.0,1.0,1.0);" +
		"}" +
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

   mUniformPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_m_matrix");
	vUniformPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_v_matrix");
	pUniformPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_p_matrix");
	LDUniformOnePerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_LdOne");
	LAUniformOnePerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_LaOne");
	LSUniformOnePerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_LsOne");
	LightPositionUniformOnePerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Light_PositionOne");

	LDUniformTwoPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_LdTwo");
	LAUniformTwoPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_LaTwo");
	LSUniformTwoPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_LsTwo");
	LightPositionUniformTwoPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Light_PositionTwo");


	KDUniformPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Kd");
	KAUniformPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Ka");
	KSUniformPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_Ks");
	MaterialShininessUniformPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_MaterialShininess");

	LKeyIsPressedUniformPerVertex = gl.getUniformLocation(shaderProgramObject_PerVertex, "u_LKeyIsPressed");


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
		"uniform vec3 u_LdOne;" +
		"uniform vec3 u_LdTwo;" +
		"uniform vec3 u_Kd;" +
		"uniform vec3 u_LsOne;" +
		"uniform vec3 u_LsTwo;" +
		"uniform vec3 u_Ks;" +
		"uniform vec3 u_LaOne;" +
		"uniform vec3 u_LaTwo;" +
		"uniform vec3 u_Ka;" +
		"uniform vec4 u_Light_PositionOne;" +
		"uniform vec4 u_Light_PositionTwo;" +
		"uniform float u_MaterialShininess;" +
		"uniform int u_LKeyIsPressed;" +
		"out vec4 FragColor;" +
		"void main(void)" +
		"{" +
		"if(u_LKeyIsPressed==1)" +
		"{" +
		"vec3 tNorm=normalize(tNormVertexShader);"+
		"vec3 lightDirectionOne=normalize(vec3(u_Light_PositionOne-eye_coordinatesVertexShader));" +
		"vec3 lightDirectionTwo=normalize(vec3(u_Light_PositionTwo-eye_coordinatesVertexShader));" +
		"float tndotldOne=max(dot(lightDirectionOne,tNorm),0.0);" +
		"float tndotldTwo=max(dot(lightDirectionTwo,tNorm),0.0);" +
		"vec3 ReflectionVectorOne=reflect(-lightDirectionOne,tNorm);" +
		"vec3 ReflectionVectorTwo=reflect(-lightDirectionTwo,tNorm);" +
		"vec3 viewerVector=normalize(vec3(-eye_coordinatesVertexShader.xyz));" +
		"vec3 ambientOne=u_LaOne*u_Ka;" +
		"vec3 ambientTwo=u_LaTwo*u_Ka;" +
		"vec3 diffusedOne=u_LdOne*u_Kd*tndotldOne;" +
		"vec3 diffusedTwo=u_LdTwo*u_Kd*tndotldTwo;" +
		"vec3 specularOne=u_LsOne*u_Ks*pow(max(dot(ReflectionVectorOne,viewerVector),0.0),u_MaterialShininess);" +
		"vec3 specularTwo=u_LsTwo*u_Ks*pow(max(dot(ReflectionVectorTwo,viewerVector),0.0),u_MaterialShininess);" +
		"vec3 phong_ads_light = ambientOne+diffusedOne+specularOne+ambientTwo+diffusedTwo+specularTwo;" +
		"FragColor=vec4(phong_ads_light,1.0);" +
		"}" +
		"else" +
		"{" +
		"FragColor=vec4(1.0,1.0,1.0,1.0);"+
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
	mUniformPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_m_matrix");
	vUniformPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_v_matrix");
	pUniformPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_p_matrix");
	LDUniformOnePerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_LdOne");
	LAUniformOnePerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_LaOne");
	LSUniformOnePerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_LsOne");
	LightPositionUniformOnePerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Light_PositionOne");

	LDUniformTwoPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_LdTwo");
	LAUniformTwoPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_LaTwo");
	LSUniformTwoPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_LsTwo");
	LightPositionUniformTwoPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Light_PositionTwo");


	KDUniformPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Kd");
	KAUniformPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Ka");
	KSUniformPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_Ks");
	MaterialShininessUniformPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_MaterialShininess");
	
	LKeyIsPressedUniformPerFragment = gl.getUniformLocation(shaderProgramObject_PerFragment, "u_LKeyIsPressed");

	
    var PyramidVertices=new Float32Array([
        0.0, 1.0, 0.0,
		-1.0, -1.0, 1.0,
		1.0, -1.0, 1.0,

		0.0, 1.0, 0.0,
		-1.0, -1.0, -1.0,
		-1.0, -1.0, 1.0,

		0.0, 1.0, 0.0,
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,

		0.0, 1.0, 0.0,
		1.0, -1.0, 1.0,
		1.0, -1.0, -1.0
    ]);

    var PyramidNormal=new Float32Array([
		// Front face
		0.0, 0.447214, 0.894427,
		0.0, 0.447214, 0.894427,
		0.0, 0.447214, 0.894427,

		// Right face
		0.894427, 0.447214, 0.0,
		0.894427, 0.447214, 0.0,
		0.894427, 0.447214, 0.0,

		// Back face
		0.0, 0.447214, -0.894427,
		0.0, 0.447214, -0.894427,
		0.0, 0.447214, -0.894427,

		// Left face
		-0.894427, 0.447214, 0.0,
		-0.894427, 0.447214, 0.0,
		-0.894427, 0.447214, 0.0
    ]);

    vao_Pyramid=gl.createVertexArray();
    gl.bindVertexArray(vao_Pyramid);
    vbo_Position_Pyramid=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_Position_Pyramid);
    gl.bufferData(gl.ARRAY_BUFFER,PyramidVertices,gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX,3,gl.FLOAT,false,0,0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
    gl.bindBuffer(gl.ARRAY_BUFFER,null);

    vbo_Normal_Pyramid=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,vbo_Normal_Pyramid);
    gl.bufferData(gl.ARRAY_BUFFER,PyramidNormal,gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_NORMAL,3,gl.FLOAT,false,0,0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_NORMAL);
    gl.bindBuffer(gl.ARRAY_BUFFER,null);

    gl.bindVertexArray(null);


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

        mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadian(rotatePyramid));

        mat4.multiply(modelMatrix, modelMatrix, rotationMatrix);
        gl.uniformMatrix4fv(mUniformPerFragment,
               false,
               modelMatrix);
        gl.uniformMatrix4fv(vUniformPerFragment,
                false,
                viewMatrix);
        gl.uniformMatrix4fv(pUniformPerFragment,
            false,
            perspectiveProjectionMatrix);
        if (gLighting == 1) {
        		gl.uniform1i(LKeyIsPressedUniformPerFragment,
				gLighting);

			gl.uniform4fv(LightPositionUniformOnePerFragment, lightPositionZero);
			gl.uniform3fv(LAUniformOnePerFragment, lightAmbientZero);
			gl.uniform3fv(LDUniformOnePerFragment, lightDiffusedZero);
			gl.uniform3fv(LSUniformOnePerFragment, lightSpecularZero);

			gl.uniform4fv(LightPositionUniformTwoPerFragment, lightPositionOne);
			gl.uniform3fv(LAUniformTwoPerFragment, lightAmbientOne);
			gl.uniform3fv(LDUniformTwoPerFragment,lightDiffusedOne);
			gl.uniform3fv(LSUniformTwoPerFragment, lightSpecularOne);

			gl.uniform3fv(KAUniformPerFragment, materialAmbient);
			gl.uniform3fv(KDUniformPerFragment, materialDiffused);
			gl.uniform3fv(KSUniformPerFragment, materialSpecular);
			gl.uniform1f(MaterialShininessUniformPerFragment, materialShininess);

        }
        else {
            gl.uniform1i(LKeyIsPressedUniformPerFragment, gLighting);
        }
        gl.bindVertexArray(vao_Pyramid);
		gl.drawArrays(gl.TRIANGLES, 0, 12);
		gl.bindVertexArray(null);
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

        mat4.rotateY(rotationMatrix, rotationMatrix, degreeToRadian(rotatePyramid));

        mat4.multiply(modelMatrix, modelMatrix, rotationMatrix);
        gl.uniformMatrix4fv(mUniformPerVertex,
               false,
               modelMatrix);
        gl.uniformMatrix4fv(vUniformPerVertex,
                false,
                viewMatrix);
        gl.uniformMatrix4fv(pUniformPerVertex,
            false,
            perspectiveProjectionMatrix);
        if (gLighting == 1) {
           		
			gl.uniform1i(LKeyIsPressedUniformPerVertex,
				gLighting);

			gl.uniform4fv(LightPositionUniformOnePerVertex,lightPositionZero);
			gl.uniform3fv(LAUniformOnePerVertex, lightAmbientZero);
			gl.uniform3fv(LDUniformOnePerVertex,lightDiffusedZero);
			gl.uniform3fv(LSUniformOnePerVertex,lightSpecularZero);

			gl.uniform4fv(LightPositionUniformTwoPerVertex,lightPositionOne);
			gl.uniform3fv(LAUniformTwoPerVertex,lightAmbientOne);
			gl.uniform3fv(LDUniformTwoPerVertex, lightDiffusedOne);
			gl.uniform3fv(LSUniformTwoPerVertex, lightSpecularOne);

			gl.uniform3fv(KAUniformPerVertex,materialAmbient);
			gl.uniform3fv(KDUniformPerVertex, materialDiffused);
			gl.uniform3fv(KSUniformPerVertex, materialSpecular);
			gl.uniform1f(MaterialShininessUniformPerVertex, materialShininess);
        }
        else {
            gl.uniform1i(LKeyIsPressedUniformPerVertex, gLighting);
        }
        gl.bindVertexArray(vao_Pyramid);
		gl.drawArrays(gl.TRIANGLES, 0, 12);
		gl.bindVertexArray(null);
		
        gl.useProgram(null);
    }
    requestAnimationFrame(draw,canvas);

	rotatePyramid = rotatePyramid+0.10;
	if(rotatePyramid>=360.0)
	{
		rotatePyramid=0.0;
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
	if (vbo_Position_Pyramid)
	{
		gl.deleteBuffers(vbo_Position_Pyramid);
		vbo_Position_Pyramid = 0;
	}

	if (vbo_Normal_Pyramid)
	{
		gl.deleteBuffers(vbo_Normal_Pyramid);
		vbo_Normal_Pyramid = 0;
	}

	if (vao_Pyramid)
	{
		gl.deleteBuffers(vao_Pyramid);
		vao_Pyramid = 0;
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

    if(shaderProgramObject_PerVertex)
    {
        if(fragmentShaderObject_PerVertex)
        {
            gl.detachShader(shaderProgramObject_PerVertex,fragmentShaderObject_PerVertex);
            gl.deleteShader(fragmentShaderObject_PerVertex);
            fragmentShaderObject_PerVertex=null;
        }
        
        if(vertexShaderObject_PerFragment)
        {
            gl.detachShader(shaderProgramObject_PerVertex,vertexShaderObject_PerVertex);
            gl.deleteShader(vertexShaderObject_PerVertex);
            vertexShaderObject_PerVertex=null;
        }
        gl.deleteProgram(shaderProgramObject_PerVertex);
        shaderProgramObject_PerVertex=null;
    }
}