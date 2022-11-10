package project.rtr.diffusedlightoncube;

//Default given package
import android.content.Context;
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
	private int fragmentShaderObject;
	private int shaderProgramObject;

	private int[] vao_Cube=new int[1];
	private int[] vbo_Position_Cube=new int[1];
	private int[] vbo_Normal_Cube=new int[1];
	private boolean gAnimate=false;
	private boolean gLighting=false;
	private int mvUniform,pUniform ,LDUiniform ,KDUiniform ,LightPositionUiniform ,LKeyIsPressedUiniform;

	private float[] perspectiveProjectionMatrix=new float[16];
	private static float rotateCube = 360.0f;

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
		gLighting =!gLighting;
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
		gAnimate=!gAnimate;	
	}

	@Override
	public void onShowPress(MotionEvent event)
	{
	}

	@Override
	public boolean onSingleTapUp(MotionEvent event)
	{
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
		update();
		display();
	}

	//Custom Method
	private void Initialize()
	{
		//VertexShader Object
		vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		//Write Shader source code
		final String vertexShaderSourceCode =
			"#version 320 es" +
			"\n" +
			"in vec4 vPosition;" +
			"in vec3 vNormal;" +
			"uniform mediump int u_LKeyIsPressed;" +
			"uniform mat4 u_mv_matrix;" +
			"uniform mat4 u_p_matrix;" +
			"uniform vec3 u_Ld;" +
			"uniform vec3 u_Kd;" +
			"uniform vec4 u_Light_Position;" +
			"out vec3 out_DiffusedColor;" +
			"void main(void)" +
			"{" +
			"if(u_LKeyIsPressed==1)" +
			"{" +
			"vec4 eye_coordinates=u_mv_matrix*vPosition;" +
			"mat3 normalMatrix=mat3(transpose(inverse(u_mv_matrix)));" +
			"vec3 tNorm=normalize(normalMatrix*vNormal);" +
			"vec3 s=normalize(vec3(u_Light_Position-eye_coordinates));" +
			"out_DiffusedColor=u_Ld*u_Kd*max(dot(s,tNorm),0.0);" +
			"}" +
			"gl_Position=u_p_matrix*u_mv_matrix*vPosition;" +
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


		//FragmentShader Object
		fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

		//Write Shader source code
		final String fragmentShaderSourceCode =
				"#version 320 es" +
				"\n" +
				"precision highp float;" +
				"in vec3 out_DiffusedColor;" +
				"uniform int u_LKeyIsPressed;" +
				"out vec4 FragColor;" +
				"void main(void)" +
				"{" +
				"if(u_LKeyIsPressed==1)" +
				"{" +
				"FragColor=vec4(out_DiffusedColor,1.0);" +
				"}" +
				"else" +
				"{" +
				"FragColor=vec4(1.0,1.0,1.0,1.0);" +
				"}" +
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

		//Attach Fragment Shader to Program
	 	GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);

		//Prelinking binding Vertex Attributes
		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");

		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");
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
		mvUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_mv_matrix");
		pUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_p_matrix");
		LDUiniform  = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ld");
		KDUiniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Kd");
		LightPositionUiniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Light_Position");
		LKeyIsPressedUiniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_LKeyIsPressed");
	
		final float[] cubeVertices = new float[]{
				1.0f, 1.0f, -1.0f,
				-1.0f, 1.0f, -1.0f,
				-1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,

				//bottom face
				1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f, 1.0f,
				1.0f, -1.0f, 1.0f,

				//front face
				1.0f, 1.0f, 1.0f,
				-1.0f, 1.0f, 1.0f,
				-1.0f, -1.0f, 1.0f,
				1.0f, -1.0f, 1.0f,

				//back face
				1.0f, 1.0f, -1.0f,
				-1.0f, 1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,

				//right face
				1.0f, 1.0f, -1.0f,
				1.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 1.0f,
				1.0f, -1.0f, -1.0f,

				//left face
				-1.0f, 1.0f, 1.0f,
				-1.0f, 1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f, 1.0f
		};

		final float[] cubeNormal = new float[]{
					0.0f, 1.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					0.0f, 1.0f, 0.0f,

					0.0f, -1.0f, 0.0f,
					0.0f, -1.0f, 0.0f,
					0.0f, -1.0f, 0.0f,
					0.0f, -1.0f, 0.0f,

					0.0f, 0.0f, 1.0f,
					0.0f, 0.0f, 1.0f,
					0.0f, 0.0f, 1.0f,
					0.0f, 0.0f, 1.0f,

					0.0f, 0.0f, -1.0f,
					0.0f, 0.0f, -1.0f,
					0.0f, 0.0f, -1.0f,
					0.0f, 0.0f, -1.0f,

					1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 0.0f,
					1.0f, 0.0f, 0.0f,
					
					-1.0f, 0.0f, 0.0f,
					-1.0f, 0.0f, 0.0f,
					-1.0f, 0.0f, 0.0f,
					-1.0f, 0.0f, 0.0f
		};

		//create vao Cube
		GLES32.glGenVertexArrays(1, vao_Cube,0);
		GLES32.glBindVertexArray(vao_Cube[0]);

		GLES32.glGenBuffers(1, vbo_Position_Cube,0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Position_Cube[0]);

		//Convert the array which can be sent to buffer Data
		ByteBuffer byteBuffer=ByteBuffer.allocateDirect(cubeVertices.length*4);
		//Arrange the buffer in native byte order
		byteBuffer.order(ByteOrder.nativeOrder());
		//To Float buffer
		FloatBuffer positionBuffer=byteBuffer.asFloatBuffer();
		//Put array in Cooked buffer
		positionBuffer.put(cubeVertices);
		//Start from 0th position
		positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
			cubeVertices.length*4,
			positionBuffer,
			GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

		//Normal
		GLES32.glGenBuffers(1, vbo_Normal_Cube,0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Normal_Cube[0]);

		//Convert the array which can be sent to buffer Data
		ByteBuffer byteBufferCubeNormal=ByteBuffer.allocateDirect(cubeNormal.length*4);
		//Arrange the buffer in native byte order
		byteBufferCubeNormal.order(ByteOrder.nativeOrder());
		//To Float buffer
		FloatBuffer positionBufferCubeNormal=byteBufferCubeNormal.asFloatBuffer();
		//Put array in Cooked buffer
		positionBufferCubeNormal.put(cubeNormal);
		//Start from 0th position
		positionBufferCubeNormal.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
			cubeNormal.length*4,
			positionBufferCubeNormal,
			GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

		GLES32.glBindVertexArray(0);

		GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		GLES32.glClearDepthf(1.0f);
		GLES32.glEnable(GLES32.GL_DEPTH_TEST);
		GLES32.glDepthFunc(GLES32.GL_LEQUAL);
		GLES32.glDisable(GLES32.GL_CULL_FACE);
		
		Matrix.setIdentityM(perspectiveProjectionMatrix,0);

	}

	private void resize(int width,int height)
	{
		GLES32.glViewport(0, 0, width, height);
	 	Matrix.perspectiveM(perspectiveProjectionMatrix,0,45.0f, (float)width / (float)height, 0.1f, 100.0f);
	}
	private void display()
	{
		GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT|GLES32.GL_DEPTH_BUFFER_BIT);	
		GLES32.glUseProgram(shaderProgramObject);
		

		//Bind with vao
		GLES32.glBindVertexArray(vao_Cube[0]);
		System.out.println("RTR : Vao Binding");

		//Declaration of matrices
		float[] modelViewMatrix=new float[16];
		float[] projectionMatrix=new float[16];
		float[] translationMatrix=new float[16];
		float[] rotationMatrix=new float[16];

		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(projectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);


		//Do necessary transformation
		Matrix.translateM(translationMatrix,0,0.0f, 0.0f, -15.0f);
		Matrix.setRotateM(rotationMatrix,0,rotateCube,0.0f, 1.0f, 0.0f);
		
		//Do necessary matrix multiplication	
		Matrix.multiplyMM(modelViewMatrix ,0,modelViewMatrix,0,translationMatrix,0);
		Matrix.multiplyMM(modelViewMatrix ,0,modelViewMatrix,0,rotationMatrix,0);
		
		//send necessary matrices to shaders
		GLES32.glUniformMatrix4fv(mvUniform,
				1,
				false,
				modelViewMatrix,0);
		GLES32.glUniformMatrix4fv(pUniform,
			1,
			false,
			projectionMatrix,0);
		if (gLighting == true)
		{
			GLES32.glUniform1i(LKeyIsPressedUiniform,
				1);
			
			GLES32.glUniform3f(LDUiniform, 1.0f, 1.0f,1.0f);
			GLES32.glUniform3f(KDUiniform, 0.5f,0.5f,0.5f);
			float[] lightPosition = new float[]{ 0.0f, 0.0f, 2.0f, 1.0f };
			GLES32.glUniform4fv(LightPositionUiniform, 1, lightPosition,0);
		}
		else
		{
			GLES32.glUniform1i(LKeyIsPressedUiniform,0);
		}

		//Bind with textures

		//Draw necessary scene
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);
	
		//Unbind vao
		GLES32.glBindVertexArray(0);
		GLES32.glUseProgram(0);

		requestRender();
	}

	private void update()
	{
		rotateCube += 0.1f;
		if (rotateCube > 360.0f)
		{
			rotateCube = 0.0f;
		}
	}

	private void unInitialize()
	{
		if (vbo_Position_Cube[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vbo_Position_Cube,0);
			vbo_Position_Cube[0] = 0;
		}
		
		if (vbo_Normal_Cube[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vbo_Normal_Cube,0);
			vbo_Normal_Cube[0] = 0;
		}
	
		if (vao_Cube[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vao_Cube,0);
			vao_Cube[0] = 0;
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