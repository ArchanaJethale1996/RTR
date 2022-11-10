package project.rtr.tessilationshader;

//Default given package
import android.content.Context;
import android.graphics.Color;
import android.view.Gravity;

//For Motion Event
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.opengl.GLES32;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;

//
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

//For matrix math
import android.opengl.Matrix;

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer,OnGestureListener, OnDoubleTapListener {
	private GestureDetector gestureDetector;
	private final Context context;
	private int vertexShaderObject;
	private int gTessellationControlShaderObject;
	private int gTessellationEvaluationShaderObject;
	private int fragmentShaderObject;
	private int shaderProgramObject;

	private int[] vao=new int[1];
	private int[] vbo=new int[1];

	private int mvpUniform;

	private float[] perspectiveProjectionMatrix=new float[16];

	
	int gNumberOfLineSegmentsUniform;
	int gNumberOfStripsUniform;
	int gLineColorUniform;

	static int gNumberOfLineSegments=1;

	public GLESView(Context drawingContext)
	{
		super(drawingContext);
		context=drawingContext;

		setEGLContextClientVersion(3);
		setRenderer(this);
		setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
		gestureDetector = new GestureDetector(drawingContext,this,null,false);

		gestureDetector.setOnDoubleTapListener(this);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		int eventaction=event.getAction();
		if(!gestureDetector.onTouchEvent(event))
			super.onTouchEvent(event);

		return true;
	}

	@Override
	public boolean onDoubleTap(MotionEvent event)
	{
			
		return true;
	}
	@Override
	public boolean onDoubleTapEvent(MotionEvent event)
	{
		return true;
	}

	@Override
	public boolean onSingleTapConfirmed(MotionEvent event)
	{
		return true;
	}

	@Override
	public boolean onDown(MotionEvent event)
	{
		return true;
	}
	@Override
	public void onLongPress(MotionEvent event)
	{
		gNumberOfLineSegments=gNumberOfLineSegments-5;
		if (gNumberOfLineSegments<= 0)
			gNumberOfLineSegments = 1;
	}

	@Override
	public void onShowPress(MotionEvent event)
	{
	}

	@Override
	public boolean onSingleTapUp(MotionEvent event)
	{
	
	gNumberOfLineSegments=gNumberOfLineSegments+5;
		if (gNumberOfLineSegments >= 100)
			gNumberOfLineSegments = 0;
		return true;
	}

	@Override
	public boolean onScroll(MotionEvent event1,MotionEvent event2,float distanceX,float distanceY)
	{
		unInitialize();
		System.exit(0);
		return true;
	}
	@Override
	public boolean onFling(MotionEvent event1,MotionEvent event2,float velocityX,float velocityY)
	{
		return true;
	}


	//Implement GLSurfaceView.Renderer methods
	@Override
	public void onSurfaceCreated(GL10 gl,EGLConfig config)
	{
		//String version=GLES32.GetString(GL10.GL_VERSION);
		//System.out.println("RTR : "+version);
		Initialize();
	}

	@Override
	public void onSurfaceChanged(GL10 unused,int width,int height)
	{
		resize(width,height);
	}

	@Override
	public void onDrawFrame(GL10 unused)
	{
		display();
	}

	//Custom Method
	private void Initialize()
	{
		GLES32.glClearColor(0.0f,0.0f,1.0f,1.0f);

		//VertexShader Object
		vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		//Write Shader source code
		final String vertexShaderSourceCode =
			"#version 320 es" +
			"\n" +
			"in vec2 vPosition;" +
			"void main(void)" +
			"{" +
			"gl_Position=vec4(vPosition,0.0,1.0);" +
			"}";

		//Specify the above source code to the shader object
		GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);

		//compile the vertex shader source code
		GLES32.glCompileShader(vertexShaderObject);

		//Error Checking
		int[] iShaderCompileStatus = new int[1];
		int[] iInfoLength = new int[1];
		String szInfoLog = null;

		GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus,0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetShaderInfoLog(vertexShaderObject);
					System.out.println("RTR : "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}

		//tessilationshader
		gTessellationControlShaderObject = GLES32.glCreateShader(GLES32.GL_TESS_CONTROL_SHADER);

		//Write Shader source code
		final String tessilationControlShaderSourceCode =
			"#version 320 es" +
			"\n" +
			"precision highp float;" +
			"precision mediump int;" +
		"layout(vertices=4)out;" +
		"uniform int numberOfSegments;" +
		"uniform int numberOfStrips;" +
		"void main(void)" +
		"{" +
		"gl_out[gl_InvocationID].gl_Position=gl_in[gl_InvocationID].gl_Position;" +
		"gl_TessLevelOuter[0]=float(numberOfStrips);"+
		"gl_TessLevelOuter[1]=float(numberOfSegments);"+
		"}";

		//Specify the above source code to the shader object
		GLES32.glShaderSource(gTessellationControlShaderObject, tessilationControlShaderSourceCode);

		//compile the vertex shader source code
		GLES32.glCompileShader(gTessellationControlShaderObject);

		//Error Checking
		iShaderCompileStatus[0] = 0;
		iInfoLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetShaderiv(gTessellationControlShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus,0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(gTessellationControlShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetShaderInfoLog(gTessellationControlShaderObject);
					System.out.println("RTR Tesssilation Shadder: "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}


		//tessilationshader
		gTessellationEvaluationShaderObject = GLES32.glCreateShader(GLES32.GL_TESS_EVALUATION_SHADER);

		//Write Shader source code
		final String tessillationEvaluationShaderSourceCode =
			"#version 320 es" +
			"\n" +
			"precision highp float;" +
			"precision mediump int;" +
		"layout(isolines)in;" +
		"uniform mat4 u_mvp_matrix;" +
		"void main(void)" +
		"{" +
		"float u=gl_TessCoord.x;" +
		"vec3 p0=gl_in[0].gl_Position.xyz;" +
		"vec3 p1=gl_in[1].gl_Position.xyz;" +
		"vec3 p2=gl_in[2].gl_Position.xyz;" +
		"vec3 p3=gl_in[3].gl_Position.xyz;" +
		"float u1=(1.0-u);" +
		"float u2=(u*u);" +
		"float b3=(u2*u);" +
		"float b2=(3.0*u2*u1);" +
		"float b1=(3.0*u*u1*u1);" +
		"float b0=(u1*u1*u1);" +
		"vec3 p=p0*b0+p1*b1+p2*b2+p3*b3;" +
		"gl_Position=u_mvp_matrix*vec4(p,1.0);" +
		"}";

		//Specify the above source code to the shader object
		GLES32.glShaderSource(gTessellationEvaluationShaderObject, tessillationEvaluationShaderSourceCode);

		//compile the vertex shader source code
		GLES32.glCompileShader(gTessellationEvaluationShaderObject);

		//Error Checking
		iShaderCompileStatus[0] = 0;
		iInfoLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetShaderiv(gTessellationEvaluationShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus,0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(gTessellationEvaluationShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetShaderInfoLog(gTessellationEvaluationShaderObject);
					System.out.println("RTR Tesssilation Shadder: "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}





		//FragmentShader Object
		fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

		//Write Shader source code
		final String fragmentShaderSourceCode =
			"#version 320 es" +
			"\n" +
			"precision highp float;" +
			"out vec4 FragColor;" +
			"uniform vec4 lineColor;"+
			"void main(void)" +
			"{" +
			"FragColor=lineColor;" +
			"}";

		//Specify the above source code to the shader object
		GLES32.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);

		//compile the vertex shader source code
		GLES32.glCompileShader(fragmentShaderObject);

		//Error Checking
		iShaderCompileStatus[0] = 0;
		iInfoLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus,0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetShaderInfoLog(fragmentShaderObject);
					System.out.println("RTR : "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}

		//Create a shader program Object
		shaderProgramObject = GLES32.glCreateProgram();

		//Attach Vertex Shader to Program
		GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);
		GLES32.glAttachShader(shaderProgramObject, gTessellationControlShaderObject);
		GLES32.glAttachShader(shaderProgramObject, gTessellationEvaluationShaderObject);

		//Attach Fragment Shader to Program
	 	GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);

		//Prelinking binding Vertex Attributes
		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");

		//Link the program 
		GLES32.glLinkProgram(shaderProgramObject);

		//error checking
		int[] iProgramLinkStatus = new int[1];
		iInfoLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, iProgramLinkStatus,0);
		if (iProgramLinkStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetProgramInfoLog(shaderProgramObject);
					System.out.println("RTR : "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}

		//post linking
		mvpUniform = GLES32.glGetUniformLocation(shaderProgramObject,"u_mvp_matrix");
		gNumberOfLineSegmentsUniform = GLES32.glGetUniformLocation(shaderProgramObject,"numberOfSegments");
		gNumberOfStripsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "numberOfStrips");
		gLineColorUniform = GLES32.glGetUniformLocation(shaderProgramObject, "lineColor");


		final float[] vertices = new float[]{
				-1.0f,-1.0f,
				-0.5f,1.0f,
				0.5f,-1.0f,
				1.0f,1.0f
		};

		//create vao
		GLES32.glGenVertexArrays(1, vao,0);
		GLES32.glBindVertexArray(vao[0]);

		GLES32.glGenBuffers(1, vbo,0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);

		//Convert the array which can be sent to buffer Data
		ByteBuffer byteBuffer=ByteBuffer.allocateDirect(vertices.length*4);
		//Arrange the buffer in native byte order
		byteBuffer.order(ByteOrder.nativeOrder());
		//To Float buffer
		FloatBuffer positionBuffer=byteBuffer.asFloatBuffer();
		//Put array in Cooked buffer
		positionBuffer.put(vertices);
		//Start from 0th position
		positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
			vertices.length*4,
			positionBuffer,
			GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 2, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		GLES32.glBindVertexArray(0);

		GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		GLES32.glClearDepthf(1.0f);
		GLES32.glEnable(GLES32.GL_DEPTH_TEST);
		GLES32.glDepthFunc(GLES32.GL_LEQUAL);
		GLES32.glEnable(GLES32.GL_CULL_FACE);
		Matrix.setIdentityM(perspectiveProjectionMatrix,0);
		gNumberOfLineSegments = 1;
	}

	private void resize(int width,int height)
	{
		GLES32.glViewport(0, 0, width, height);
	 	Matrix.perspectiveM(perspectiveProjectionMatrix,0,45.0f, (float)width / (float)height, 0.1f, 100.0f);
	}
	private void display()
	{
		GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT|GLES32.GL_DEPTH_BUFFER_BIT|GLES32.GL_STENCIL_BUFFER_BIT);	
		GLES32.glUseProgram(shaderProgramObject);
		
		//Declaration of matrices
		float[] modelViewMatrix=new float[16];
		float[] modelViewProjectionMatrix=new float[16];
		float[] translationMatrix=new float[16];
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);

		//Do necessary transformation
		Matrix.translateM(translationMatrix,0,0.5f, 0.5f, -2.0f);
		GLES32.glUniform1i(gNumberOfLineSegmentsUniform,gNumberOfLineSegments);
		GLES32.glUniform1i(gNumberOfStripsUniform, 1);
		GLES32.glUniform4f(gLineColorUniform,1.0f, 1.0f, 0.0f, 1.0f);
	
		//Do necessary matrix multiplication	
		Matrix.multiplyMM(modelViewMatrix ,0,modelViewMatrix,0,translationMatrix,0);
		Matrix.multiplyMM(modelViewProjectionMatrix ,0,perspectiveProjectionMatrix,0,modelViewMatrix,0);

		//send necessary matrices to shaders
		GLES32.glUniformMatrix4fv(mvpUniform,
			1, false,
			modelViewProjectionMatrix,0);

		//Bind with vao
		GLES32.glBindVertexArray(vao[0]);

		//Bind with textures

		//Draw necessary scene
		GLES32.glDrawArrays(GLES32.GL_PATCHES, 0, 4);
		
		//Unbind vao
		GLES32.glBindVertexArray(0);
		GLES32.glUseProgram(0);

		requestRender();
	}

	private void unInitialize()
	{
		if (vbo[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vbo,0);
			vbo[0] = 0;
		}
		if (vao[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vao,0);
			vao[0] = 0;
		}

		int[] shaderCount=new int[1];
		int shaderNumber;
		if (shaderProgramObject!=0)
		{
			GLES32.glUseProgram(shaderProgramObject);

			GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_ATTACHED_SHADERS, shaderCount,0);
			int[] pShaders = new int[4* shaderCount[0]];

			GLES32.glGetAttachedShaders(shaderProgramObject, shaderCount[0], shaderCount,0, pShaders,0);
			for (shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
			{
				GLES32.glDetachShader(shaderProgramObject, pShaders[shaderNumber]);
				GLES32.glDeleteShader(pShaders[shaderNumber]);
				pShaders[shaderNumber] = 0;
			}
				
			GLES32.glDeleteProgram(shaderProgramObject);
			shaderProgramObject = 0;
			GLES32.glUseProgram(0);
		}
		
	}
}