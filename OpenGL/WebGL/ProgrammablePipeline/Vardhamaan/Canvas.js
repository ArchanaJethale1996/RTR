//Original Text for target in property for chrome
//"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe"

//So replace it with anyone in target in property for chrome
//CROS-disble
// "C:\Program Files (x86)\Google\Chrome\Application\chrome.exe" --disable-web-security --user-data-dir="E:\RTR2020\Codes\WebGL\temp"
// "C:\Program Files (x86)\Google\Chrome\Application\chrome.exe" --allow-file-access-from-files --user-data-dir="E:\RTR2020\Codes\WebGL\temp"
//then run above line in cmd
//then paste url from old browser to new browser thats opened from cmd

//Global
var canvas = null;
var gl = null;

var canvas_orignal_width;
var canvas_orignal_height;
var bFullscreen=false;

const WebGLMacros={
VSV_ATTRIBUTE_POSITION:0,
VSV_ATTRIBUTE_COLOR:1,
VSV_ATTRIBUTE_NORMAL:2,
VSV_ATTRIBUTE_TEXTURE:3,
};

var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var vao;

var vbo_position;
var vbo_texture;
var vbo_color;

var mvpMatrixUniform;

var textureSamplerUniform;
var checkerboard_textureID;

var perspectiveGraphicProjectionMatrix;

var CHECK_IMAGE_WIDTH=64;
var CHECK_IMAGE_HEIGHT=64;
var checkImage;

var requestAnimationFrame = window.requestAnimationFrame || 
                            window.webkitRequestAnimationFrame ||
                            window.mozRequestAnimationFrame ||
                            window.oRequestAnimationFrame || //opera
                            window.msRequestAnimationFrame ||
                            null;

var cancelAnimationFrame =  window.cancelAnimationFrame ||
                            window.webkitCancelRequestAnimationFrame ||  window.webkitCancelAnimation ||
                            window.mozCancelRequestAnimationFrame ||  window.mozCancelAnimationFrame ||
                            window.oCancelRequestAnimationFrame ||  window.oCancelAnimationFrame ||
                            window.msCancelRequestAnimationFrame ||  window.msCancelAnimationFrame ||
                            null;

function main()
{
    //step 1 : Get canvas from html dom
    //document is inbuild variable/DOM object    
    canvas=document.getElementById("canvasID_vsv");
    if(!canvas)
    {
        console.log("ERROR:Failed to obtain Canvas");
        unitialize();
    }else{
        console.log("INFO:Obtained Canvas Successfully");
    }

    //Step 2 : Retreive the height and width of canvas
    //console is inbuild variable/DOM object 
    console.log("INFO:Canvas width=" + canvas.width + " height=" + canvas.height + "\n" );

    canvas_orignal_width = canvas.width;
    canvas_orignal_height = canvas.height;

    //step 3 : Get drawing webgl2 from the canvas
    gl = canvas.getContext("webgl2");
    if(!gl)
    {
        console.log("ERROR:Failed to obtain webgl2 context");
        unitialize();
    }else{
        console.log("INFO:Obtained webgl2 context Successfully");
    }

    if (requestAnimationFrame == null)
    {
        console.log("ERROR:requestAnimationFrame is null");
        unitialize();
        return
    }

    //step 10 : Add event listener
    //window is inbuild variable/DOM object    
    window.addEventListener("keydown",//EVENT TYPE
                            keyDown,//FUNCTION NAME
                            false);//false-captured delegation/propagation or true-bubble delegation/propagation
    
    window.addEventListener("click",
                            mouseDown,
                            false);

    window.addEventListener("resize",resize,false);

    Init();

    resize();//Warm up resize call

    draw();//Warm up redraw call
}

function toggleFullscreen()
{
    var fullscreen_element =    document.fullscreenElement || 
                                document.webkitFullscreenElement || 
                                document.mozFullScreenElement ||
                                document.msFullscreenElement ||
                                null;

    if(fullscreen_element==null)
    {
        if (canvas.requestFullscreen)
        {   
            canvas.requestFullscreen();

        }else if(canvas.webkitRequestFullscreen)
        {
            canvas.webkitRequestFullscreen();

        }else if(canvas.mozRequestFullScreen)
        {
            canvas.mozRequestFullScreen();

        }else if(canvas.msRequestFullscreen)
        {
            canvas.msRequestFullscreen();
        }

        bFullscreen=true;

    }else{
        if(document.exitFullscreen)
        {
            document.exitFullscreen();

        }else if(document.webkitExitFullscreen)
        {
            document.webkitExitFullScreen();

        }else if(document.mozCancelFullScreen)
        {
            document.mozCancelFullScreen();
            
        }else if(document.msExitFullScreen)
        {
            document.msExitFullScreen();
        }

        bFullscreen=false;
    }
}

function keyDown(event)
{
    console.log(event);
   switch(event.keyCode)
   {
        case 70:
            toggleFullscreen();
            break;
        case 27:
            unitialize();
            window.close();
            break;
   }
}

function mouseDown()
{
    console.log("INFO:Mouse is pressed");
}

function Init()
{
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

	var vertexShaderSourceCode=
		"#version 300 es" +
		"\n" +
		"in vec4 vPosition;" +
		"in vec2 vTexCoord;" +
		"out vec2 out_TexCoord;" +

		"uniform mat4 u_mvpMatrix;" +
		"void main(void)" +
		"{" +
            "out_TexCoord=vTexCoord;"+
			"gl_Position=u_mvpMatrix * vPosition;" +
		"}";

    vertexShaderObject=gl.createShader(gl.VERTEX_SHADER);

	gl.shaderSource(vertexShaderObject,vertexShaderSourceCode);

	gl.compileShader(vertexShaderObject);

	if(gl.getShaderParameter(vertexShaderObject,gl.COMPILE_STATUS)==false)
	{
		var error;
        error = gl.getShaderInfoLog(vertexShaderObject);
        if(error.length > 0)
        {
            console.log("ERROR:Failed to compile vertexShaderSourceCode");
            alert(error);
            unitialize();
            return
        }
    }	

	//---------Fragment--------------

	var fragmentShaderSourceCode=
		"#version 300 es" +
		"\n" +
        "precision highp float;"+
		"in vec2 out_TexCoord;" +

		"out vec4 fragColor;" +
        "uniform sampler2D u_texture_sampler;"+
		"void main(void)" +
		"{"+
			"fragColor = texture(u_texture_sampler,out_TexCoord);" +
			// "fragColor = vec4(1.0,1.0,1.0,1.0);" +

		"}";

    fragmentShaderObject=gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fragmentShaderObject,fragmentShaderSourceCode);

	gl.compileShader(fragmentShaderObject);

	if(gl.getShaderParameter(fragmentShaderObject,gl.COMPILE_STATUS)==false)
	{
        var error;
        error = gl.getShaderInfoLog(fragmentShaderObject);
        if(error.length > 0)
        {
            console.log("ERROR:Failed to compile fragmentShaderSourceCode");
            alert(error);
            unitialize();
            return
        }
	}

	//-------------Shader Program---------------------------
	
	shaderProgramObject = gl.createProgram();

	gl.attachShader(shaderProgramObject,vertexShaderObject);
	gl.attachShader(shaderProgramObject,fragmentShaderObject);
	//Pre Link attachment
	gl.bindAttribLocation(shaderProgramObject,WebGLMacros.VSV_ATTRIBUTE_POSITION,"vPosition");
	gl.bindAttribLocation(shaderProgramObject,WebGLMacros.VSV_ATTRIBUTE_TEXTURE,"vTexCoord");

	gl.linkProgram(shaderProgramObject);

	if(!gl.getProgramParameter(shaderProgramObject,gl.LINK_STATUS))
	{
		var error;
		error = gl.getProgramInfoLog(shaderProgramObject);
        if(error.length > 0)
        {
            alert(error);
            unitialize();
            return
        }		
	}
	//Post Link attachment
	mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject,"u_mvpMatrix");
	textureSamplerUniform = gl.getUniformLocation(shaderProgramObject,"u_texture_sampler");

	//----------------Data prepration-------------------

    var rectangleTextureCoord = new Float32Array([
                            0.0,0.0,
		                    0.0,1.0,
		                    1.0,1.0,
		                    1.0,0.0]);

    ///----vao rectangle
    //-------------Texture Implementation Code---------
    MakeCheckImage();
    vao = gl.createVertexArray();

	gl.bindVertexArray(vao);

        vbo_position=gl.createBuffer();
		
		gl.bindBuffer(gl.ARRAY_BUFFER,
					vbo_position);
		
			gl.bufferData(gl.ARRAY_BUFFER,
                        3*4*gl.float,
						gl.DYNAMIC_DRAW);

			gl.vertexAttribPointer(WebGLMacros.VSV_ATTRIBUTE_POSITION,
								3,
								gl.FLOAT,
								false,
								0,
								0);
			
			gl.enableVertexAttribArray(WebGLMacros.VSV_ATTRIBUTE_POSITION);

		gl.bindBuffer(gl.ARRAY_BUFFER,
					null);

        vbo_texture=gl.createBuffer();
		
            gl.bindBuffer(gl.ARRAY_BUFFER,
                vbo_texture);
                    
                gl.bufferData(gl.ARRAY_BUFFER,
                    rectangleTextureCoord,
                    gl.STATIC_DRAW);

                gl.vertexAttribPointer(WebGLMacros.VSV_ATTRIBUTE_TEXTURE,
                                    2,
                                    gl.FLOAT,
                                    false,
                                    0,
                                    0);
        
                gl.enableVertexAttribArray(WebGLMacros.VSV_ATTRIBUTE_TEXTURE);

            gl.bindBuffer(gl.ARRAY_BUFFER,
                                null);
        
	gl.bindVertexArray(null);

   
    // gl.enable(gl.TEXTURE_2D);

    checkerboard_textureID = gl.createTexture();
    //bydefault browser support new Image()
    checkerboard_textureID.image = new Image(CHECK_IMAGE_WIDTH,CHECK_IMAGE_HEIGHT);
    checkerboard_textureID.image.src = "";
    checkerboard_textureID.image.onload = function ()
    {

        gl.bindTexture( gl.TEXTURE_2D,checkerboard_textureID);
            gl.pixelStorei( gl.UNPACK_FLIP_Y_WEBGL,1);

            gl.texParameteri( gl.TEXTURE_2D,
                            gl.TEXTURE_MAG_FILTER,
                            gl.NEAREST);//use NEAREST

            gl.texParameteri( gl.TEXTURE_2D,
                            gl.TEXTURE_MIN_FILTER,
                            gl.NEAREST);//use NEAREST

            gl.texParameteri( gl.TEXTURE_2D,
                            gl.TEXTURE_WRAP_S,
                            gl.REPEAT);

            gl.texParameteri( gl.TEXTURE_2D,
                            gl.TEXTURE_WRAP_T,
                            gl.REPEAT);

            // gl.texImage2D(gl.TEXTURE_2D,
            //         0,
            //         gl.RGBA,
            //         gl.RGBA,
            //         gl.UNSIGNED_BYTE,
            //         checkImage);
        
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA,
			CHECK_IMAGE_WIDTH, CHECK_IMAGE_HEIGHT, 0,
			gl.RGBA, gl.UNSIGNED_BYTE,checkImage
			);

            gl.generateMipmap(gl.TEXTURE_2D);

        gl.bindTexture( gl.TEXTURE_2D,null);
    }

	//-----Initialize ortho matrix
	perspectiveGraphicProjectionMatrix=mat4.create();

	//-----OpenGL state Machine States-----------------------
    gl.clearColor(0.0, 0.0, 0.0, 1.0);//RGBA
	gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);

}

function resize()
{
    if(bFullscreen==true)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }else{
        canvas.width = canvas_orignal_width;
        canvas.height = canvas_orignal_height;
    }

    gl.viewport(0,0,canvas.width, canvas.height);

    mat4.perspective(perspectiveGraphicProjectionMatrix,
                    45.0,
                    parseFloat(canvas.width)/parseFloat(canvas.height),
                    0.1,
                    100.0);
}
var Angle=0;

var CheckerboardRectangle1 = new Float32Array([

	-2.0, -1.0, 0.0,
	-2.0, 1.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, -1.0, 0.0
]);

var CheckerboardRectangle2 = new Float32Array([
	1.0, -1.0, 0.0,
	1.0, 1.0, 0.0,
	2.41421, 1.0, -1.41421,
	2.41421, -1.0, -1.41421
]);

function draw()
{
    var modelViewProjectMatrix = mat4.create();
    var modelViewMatrix = mat4.create();
    var rectanglePosition = null;
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT );
    // gl.enable(gl.TEXTURE_2D);
	gl.useProgram(shaderProgramObject);

        //rectangle
        mat4.translate(modelViewMatrix,modelViewMatrix,[0.0,0.0,-3.6]);

        mat4.multiply(modelViewProjectMatrix,
                    perspectiveGraphicProjectionMatrix ,
                    modelViewMatrix);

		gl.uniformMatrix4fv(mvpMatrixUniform,
							false,
							modelViewProjectMatrix);
        

        gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture( gl.TEXTURE_2D,checkerboard_textureID);
        gl.uniform1i(textureSamplerUniform,0);

        rectanglePosition = CheckerboardRectangle1;

		gl.bindVertexArray(vao);
            gl.bindBuffer(gl.ARRAY_BUFFER,vbo_position);      
                gl.bufferData(gl.ARRAY_BUFFER,
                    rectanglePosition,
                    gl.DYNAMIC_DRAW);
                    
                gl.vertexAttribPointer(WebGLMacros.VSV_ATTRIBUTE_POSITION,
                        3,
                        gl.FLOAT,
                        false,
                        0,
                        0);

                gl.enableVertexAttribArray(WebGLMacros.VSV_ATTRIBUTE_POSITION);


            gl.bindBuffer(gl.ARRAY_BUFFER,
                                null);

            gl.drawArrays(gl.TRIANGLE_FAN,0,4);

		gl.bindVertexArray(null);

        rectanglePosition = CheckerboardRectangle2;

		gl.bindVertexArray(vao);
            gl.bindBuffer(gl.ARRAY_BUFFER,vbo_position);      
                gl.bufferData(gl.ARRAY_BUFFER,
                    rectanglePosition,
                    gl.DYNAMIC_DRAW);

                gl.vertexAttribPointer(WebGLMacros.VSV_ATTRIBUTE_POSITION,
                        3,
                        gl.FLOAT,
                        false,
                        0,
                        0);

                gl.enableVertexAttribArray(WebGLMacros.VSV_ATTRIBUTE_POSITION);

            gl.bindBuffer(gl.ARRAY_BUFFER,
                                null);

            gl.drawArrays(gl.TRIANGLE_FAN,0,4);

		gl.bindVertexArray(null);
        
	gl.useProgram(null);
    
    Angle+=3;
    // requestAnimationFrame(update,canvas); //if using update() then use this way
    requestAnimationFrame(draw,canvas);
}

function degToRad(degrees)
{
    return ( degrees*Math.PI /180.0)
}

function unitialize()
{
    if(vao)
	{
		gl.deleteVertexArray(vao);
		vao=null;
	}

	if(vbo_position)
	{
		gl.deleteBuffer(vbo_position);
		vbo_position=null;
	}

    if(vbo_texture)
	{
		gl.deleteBuffer(vbo_texture);
		vbo_texture=null;
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

    gl.deleteTexture(checkerboard_textureID);
}

function MakeCheckImage()
{
	var vsv_i,vsv_j,vsv_c;
    checkImage=[];

    for(vsv_i=0;vsv_i<CHECK_IMAGE_HEIGHT;vsv_i++){
        checkImage[vsv_i]=[];
        for(vsv_j=0;vsv_j<CHECK_IMAGE_WIDTH;vsv_j++){

            vsv_c=( (vsv_i & 0x8 )==0)^( (vsv_j & 0x8 )==0);
			vsv_c*=255;
            checkImage[vsv_i][vsv_j]=[];
			checkImage[vsv_i][vsv_j][0]=vsv_c;
			checkImage[vsv_i][vsv_j][1]=vsv_c;
			checkImage[vsv_i][vsv_j][2]=vsv_c;
			checkImage[vsv_i][vsv_j][3]=255;
        }
    }
}
// gl.bindTexture(gl.TEXTURE_2D, textureCheckerboard);
// gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
// gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
// gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
// gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
// gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
// gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, CheckImageWidth, CheckImageHeight, 0, gl.RGBA, gl.UNSIGNED_BYTE, imageData);
// gl.generateMipmap(gl.TEXTURE_2D);
// gl.bindTexture(gl.TEXTURE_2D, null);