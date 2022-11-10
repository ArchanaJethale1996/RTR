package project.rtr.twodrotatingshape;

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
	private int fragmentShaderObject;
	private int shaderProgramObject;

	private int[] vao_Triangle=new int[1];
	private int[] vao_Rectangle=new int[1];

	private int[] vbo_Position_Triangle=new int[1];
	private int[] vbo_Position_Rectangle=new int[1];

	private int[] vbo_Color_Triangle=new int[1];
	private int[] vbo_Color_Rectangle=new int[1];
	private int mvpUniform;

	private float[] perspectiveProjectionMatrix=new float[16];
	private static float rotateTriangle = 0.0f;
	private static float rotateRectangle = 360.0f;

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
		GLES32.glClearColor(0.0f,0.0f,1.0f,1.0f);

		//VertexShader Object
		vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		//Write Shader source code
		final String vertexShaderSourceCode =
			"#version 320 es" +
			"\n" +
			"in vec4 vPosition;" +
			"in vec4 vColor;" +
			"out vec4 out_Color;"+
			"uniform mat4 u_mvp_matrix;" +
			"void main(void)" +
			"{" +
			"gl_Position=u_mvp_matrix*vPosition;" +
			"out_Color=vColor;" +
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
			"in vec4 out_Color;" +
			"out vec4 FragColor;" +
			"void main(void)" +
			"{" +
			"FragColor=out_Color;" +
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

		GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_COLOR, "vColor");
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

		final float[] triangleVertices = new float[]{
				0.0f,1.0f,0.0f,
				-1.0f,-1.0f,0.0f,
				1.0f,-1.0f,0.0f
				};

		final float[] triangleColor = new float[]{
				1.0f,1.0f,0.0f,
				0.0f,1.0f,0.0f,
				1.0f,0.0f,0.0f
		};

		//create vao triangle
		GLES32.glGenVertexArrays(1, vao_Triangle,0);
		GLES32.glBindVertexArray(vao_Triangle[0]);

		GLES32.glGenBuffers(1, vbo_Position_Triangle,0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Position_Triangle[0]);

		//Convert the array which can be sent to buffer Data
		ByteBuffer byteBuffer=ByteBuffer.allocateDirect(triangleVertices.length*4);
		//Arrange the buffer in native byte order
		byteBuffer.order(ByteOrder.nativeOrder());
		//To Float buffer
		FloatBuffer positionBuffer=byteBuffer.asFloatBuffer();
		//Put array in Cooked buffer
		positionBuffer.put(triangleVertices);
		//Start from 0th position
		positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
			triangleVertices.length*4,
			positionBuffer,
			GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

		//color
		GLES32.glGenBuffers(1, vbo_Color_Triangle,0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Color_Triangle[0]);

		//Convert the array which can be sent to buffer Data
		ByteBuffer byteBufferTriangleColor=ByteBuffer.allocateDirect(triangleColor.length*4);
		//Arrange the buffer in native byte order
		byteBufferTriangleColor.order(ByteOrder.nativeOrder());
		//To Float buffer
		FloatBuffer positionBufferTriangleColor=byteBufferTriangleColor.asFloatBuffer();
		//Put array in Cooked buffer
		positionBufferTriangleColor.put(triangleColor);
		//Start from 0th position
		positionBufferTriangleColor.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
			triangleColor.length*4,
			positionBufferTriangleColor,
			GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

		GLES32.glBindVertexArray(0);


		final float[] rectangleVertices = new float[]{
				1.0f,1.0f,0.0f,
				-1.0f,1.0f,0.0f,
				-1.0f,-1.0f,0.0f,
				1.0f,-1.0f,0.0f,
				};

		final float[]  rectangleColor = new float[]{
			1.0f,1.0f,0.0f,
			1.0f,1.0f,0.0f,
			1.0f,1.0f,0.0f,
			1.0f,1.0f,0.0f,
		};
		//create vao rectangle
		GLES32.glGenVertexArrays(1, vao_Rectangle,0);
		GLES32.glBindVertexArray(vao_Rectangle[0]);

		GLES32.glGenBuffers(1, vbo_Position_Rectangle,0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Position_Rectangle[0]);

		//Convert the array which can be sent to buffer Data
		ByteBuffer byteBufferRectangle=ByteBuffer.allocateDirect(rectangleVertices.length*4);
		//Arrange the buffer in native byte order
		byteBufferRectangle.order(ByteOrder.nativeOrder());
		//To Float buffer
		FloatBuffer positionBufferRectangle=byteBufferRectangle.asFloatBuffer();
		//Put array in Cooked buffer
		positionBufferRectangle.put(rectangleVertices);
		//Start from 0th position
		positionBufferRectangle.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
			rectangleVertices.length*4,
			positionBufferRectangle,
			GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);


		//color
		GLES32.glGenBuffers(1, vbo_Color_Rectangle,0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Color_Rectangle[0]);

		//Convert the array which can be sent to buffer Data
		ByteBuffer byteBufferRectangleColor=ByteBuffer.allocateDirect(rectangleColor.length*4);
		//Arrange the buffer in native byte order
		byteBufferRectangleColor.order(ByteOrder.nativeOrder());
		//To Float buffer
		FloatBuffer positionBufferRectangleColor=byteBufferRectangleColor.asFloatBuffer();
		//Put array in Cooked buffer
		positionBufferRectangleColor.put(rectangleColor);
		//Start from 0th position
		positionBufferRectangleColor.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
			rectangleColor.length*4,
			positionBufferRectangleColor,
			GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
		GLES32.glBindVertexArray(0);

		GLES32.glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
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
		
		//Declaration of matrices
		float[] modelViewMatrix=new float[16];
		float[] modelViewProjectionMatrix=new float[16];
		float[] translationMatrix=new float[16];
		float[] rotationMatrix=new float[16];

		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		//Do necessary transformation
		Matrix.translateM(translationMatrix,0,-1.5f, 0.0f, -6.0f);
		Matrix.setRotateM(rotationMatrix,0,rotateTriangle,0.0f, 1.0f, 0.0f);
		
		//Do necessary matrix multiplication	
		Matrix.multiplyMM(modelViewMatrix ,0,modelViewMatrix,0,translationMatrix,0);
		Matrix.multiplyMM(modelViewMatrix ,0,modelViewMatrix,0,rotationMatrix,0);
		Matrix.multiplyMM(modelViewProjectionMatrix ,0,perspectiveProjectionMatrix,0,modelViewMatrix,0);

		//send necessary matrices to shaders
		GLES32.glUniformMatrix4fv(mvpUniform,
			1, false,
			modelViewProjectionMatrix,0);

		//Bind with vao
		GLES32.glBindVertexArray(vao_Triangle[0]);

		//Bind with textures

		//Draw necessary scene
		GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 3);
		
		//Unbind vao
		GLES32.glBindVertexArray(0);

		//Rectangle
		Matrix.setIdentityM(modelViewMatrix,0);
		Matrix.setIdentityM(modelViewProjectionMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		//Do necessary transformation
		Matrix.translateM(translationMatrix,0,1.5f, 0.0f, -6.0f);
		Matrix.setRotateM(rotationMatrix,0,rotateRectangle,1.0f, 0.0f, 0.0f);
		
		//Do necessary matrix multiplication	
		Matrix.multiplyMM(modelViewMatrix ,0,modelViewMatrix,0,translationMatrix,0);
		Matrix.multiplyMM(modelViewMatrix ,0,modelViewMatrix,0,rotationMatrix,0);
		Matrix.multiplyMM(modelViewProjectionMatrix ,0,perspectiveProjectionMatrix,0,modelViewMatrix,0);

		//send necessary matrices to shaders
		GLES32.glUniformMatrix4fv(mvpUniform,
			1, false,
			modelViewProjectionMatrix,0);

		//Bind with vao
		GLES32.glBindVertexArray(vao_Rectangle[0]);

		//Bind with textures

		//Draw necessary scene
		GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
		
		//Unbind vao
		GLES32.glBindVertexArray(0);
		GLES32.glUseProgram(0);

		requestRender();
	}

	private void update()
	{
		rotateTriangle += 0.5f;
		if (rotateTriangle > 360.0f)
		{
			rotateTriangle = 0.0f;
		}

		rotateRectangle -= 0.5f;
		if (rotateRectangle < 0)
		{
			rotateRectangle = 360.0f;
		}

	}

	private void unInitialize()
	{
		if (vbo_Position_Triangle[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vbo_Position_Triangle,0);
			vbo_Position_Triangle[0] = 0;
		}
		if (vbo_Position_Rectangle[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vbo_Position_Rectangle,0);
			vbo_Position_Rectangle[0] = 0;
		}

		if (vbo_Color_Triangle[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vbo_Color_Triangle,0);
			vbo_Color_Triangle[0] = 0;
		}
		if (vbo_Color_Rectangle[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vbo_Color_Rectangle,0);
			vbo_Color_Rectangle[0] = 0;
		}
		if (vao_Triangle[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vao_Triangle,0);
			vao_Triangle[0] = 0;
		}
		if (vao_Rectangle[0]!=0)
		{
			GLES32.glDeleteBuffers(1, vao_Rectangle,0);
			vao_Rectangle[0] = 0;
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