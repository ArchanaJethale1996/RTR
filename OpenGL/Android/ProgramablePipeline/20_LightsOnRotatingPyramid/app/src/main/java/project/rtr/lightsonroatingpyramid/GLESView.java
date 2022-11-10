package project.rtr.lightsonroatingpyramid;

//Default given package
import android.content.Context;
import android.graphics.Color;
import android.view.Gravity;
import java.util.*; ///For sin and Cos

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
import java.nio.ShortBuffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

//For matrix math
import android.opengl.Matrix;

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer,OnGestureListener, OnDoubleTapListener {
	private GestureDetector gestureDetector;
	private final Context context;

	private int vertexShaderObjectPerVertex;
	private int fragmentShaderObjectPerVertex;
	private int shaderProgramObjectPerVertex;

	private int vertexShaderObjectPerFragment;
	private int fragmentShaderObjectPerFragment;
	private int shaderProgramObjectPerFragment;

	private int[] vao_Pyramid = new int[1];
    private int[] vbo_Position_Pyramid = new int[1];
    private int[] vbo_Normal_Pyramid = new int[1];
	private int numElements,numVertices;
	private int mUniformPerVertex, vUniformPerVertex, pUniformPerVertex,KDUniformPerVertex,  KAUniformPerVertex, KSUniformPerVertex, MaterialShininessUniformPerVertex, LKeyIsPressedUniformPerVertex;
	private int LDUniformOnePerVertex, LAUniformOnePerVertex, LSUniformOnePerVertex, LightPositionUniformOnePerVertex, LDUniformTwoPerVertex, LAUniformTwoPerVertex, LSUniformTwoPerVertex, LightPositionUniformTwoPerVertex;

	private int mUniformPerFragment, vUniformPerFragment, pUniformPerFragment,KDUniformPerFragment,  KAUniformPerFragment, KSUniformPerFragment, MaterialShininessUniformPerFragment, LKeyIsPressedUniformPerFragment;
	private int LDUniformOnePerFragment, LAUniformOnePerFragment, LSUniformOnePerFragment, LightPositionUniformOnePerFragment, LDUniformTwoPerFragment, LAUniformTwoPerFragment, LSUniformTwoPerFragment, LightPositionUniformTwoPerFragment;

	private boolean gPerFragment = false;
	private boolean gAnimate = false;
	private int gLighting = 0;
	private float[] perspectiveProjectionMatrix=new float[16];
	private float rotatePyramid= 360.0f;
	public	class Light
	{
		public float[] lightAmbient=new float[3];
		public float[] lightDiffused=new float[3];
		public float[] lightSpecular=new float[3];	
		public float[] lightPosition=new float[4];
	}
	
	private Light lightsZero=new Light();
	private Light lightsOne=new Light();
	private float[] materialAmbient =new float[] { 0.0f,0.0f,0.0f,1.0f };
	private float[] materialDiffused =new float[] { 1.0f,1.0f,1.0f,1.0f };
	private float[] materialSpecular =new float[] { 1.0f,1.0f,1.0f,1.0f };
	private float materialShininess = 128.0f ;

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
		gAnimate = !gAnimate;
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
		if (gLighting == 0)
				gLighting = 1;
			else
				gLighting = 0;
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
		gPerFragment=!gPerFragment;
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
	private void update()
	{
		if (gAnimate)
			{
				rotatePyramid = rotatePyramid + 0.5f;
				if (rotatePyramid >= 360.0f)
				{
					rotatePyramid = 0.0f;
				}
			}
	}
	private void Initialize()
	{
		GLES32.glClearColor(0.5f,0.5f,0.5f,1.0f);
		//Per Vertex
		
		//VertexShader Object
		vertexShaderObjectPerVertex = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		//Write Shader source code
		final String vertexShaderSourceCodePerVertex =
			"#version 320 es" +
			"\n" +
			"in vec4 vPosition;"+ 
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
			"uniform mediump int u_LKeyIsPressed;" +
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

		//Specify the above source code to the shader object
		GLES32.glShaderSource(vertexShaderObjectPerVertex, vertexShaderSourceCodePerVertex);

		//compile the vertex shader source code
		GLES32.glCompileShader(vertexShaderObjectPerVertex);

		//Error Checking
		int[] iShaderCompileStatus = new int[1];
		int[] iInfoLength = new int[1];
		String szInfoLog = null;

		GLES32.glGetShaderiv(vertexShaderObjectPerVertex, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus,0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(vertexShaderObjectPerVertex, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetShaderInfoLog(vertexShaderObjectPerVertex);
					System.out.println("RTR : "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}


		//FragmentShader Object
		fragmentShaderObjectPerVertex = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

		//Write Shader source code
		final String fragmentShaderSourceCodePerVertex =
			"#version 320 es" +
			"\n" +
			"precision highp float;" +
			"in vec3 phong_ads_light;" +
			"out vec4 FragColor;" +
			"void main(void)" +
			"{" +
			"FragColor=vec4(phong_ads_light,1.0);" +
			"}";


		//Specify the above source code to the shader object
		GLES32.glShaderSource(fragmentShaderObjectPerVertex, fragmentShaderSourceCodePerVertex);

		//compile the vertex shader source code
		GLES32.glCompileShader(fragmentShaderObjectPerVertex);

		//Error Checking
		iShaderCompileStatus[0] = 0;
		iInfoLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetShaderiv(fragmentShaderObjectPerVertex, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus,0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(fragmentShaderObjectPerVertex, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetShaderInfoLog(fragmentShaderObjectPerVertex);
					System.out.println("RTR : "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}

		//Create a shader program Object
		shaderProgramObjectPerVertex = GLES32.glCreateProgram();

		//Attach Vertex Shader to Program
		GLES32.glAttachShader(shaderProgramObjectPerVertex, vertexShaderObjectPerVertex);

		//Attach Fragment Shader to Program
	 	GLES32.glAttachShader(shaderProgramObjectPerVertex, fragmentShaderObjectPerVertex);

		//Prelinking binding Vertex Attributes
		GLES32.glBindAttribLocation(shaderProgramObjectPerVertex, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
		GLES32.glBindAttribLocation(shaderProgramObjectPerVertex, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");


		//Link the program 
		GLES32.glLinkProgram(shaderProgramObjectPerVertex);

		//error checking
		int[] iProgramLinkStatus = new int[1];
		iInfoLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetProgramiv(shaderProgramObjectPerVertex, GLES32.GL_LINK_STATUS, iProgramLinkStatus,0);
		if (iProgramLinkStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetProgramiv(shaderProgramObjectPerVertex, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetProgramInfoLog(shaderProgramObjectPerVertex);
					System.out.println("RTR : "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}

		//post linking
		mUniformPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_m_matrix");
		vUniformPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_v_matrix");
		pUniformPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_p_matrix");
		LDUniformOnePerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_LdOne");
		LAUniformOnePerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_LaOne");
		LSUniformOnePerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_LsOne");
		LightPositionUniformOnePerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_Light_PositionOne");

		LDUniformTwoPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_LdTwo");
		LAUniformTwoPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_LaTwo");
		LSUniformTwoPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_LsTwo");
		LightPositionUniformTwoPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_Light_PositionTwo");


		KDUniformPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_Kd");
		KAUniformPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_Ka");
		KSUniformPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_Ks");
		MaterialShininessUniformPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_MaterialShininess");
		
		LKeyIsPressedUniformPerVertex = GLES32.glGetUniformLocation(shaderProgramObjectPerVertex, "u_LKeyIsPressed");


		//Per Fragment

		//VertexShader Object
		vertexShaderObjectPerFragment = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		//Write Shader source code
		final String vertexShaderSourceCodePerFragment =
			"#version 320 es" +
			"\n" +
			"in vec4 vPosition;" +
			"in vec3 vNormal;" +
			"uniform mat4 u_m_matrix;" +
			"uniform mat4 u_v_matrix;" +
			"uniform mat4 u_p_matrix;" +
			"uniform mediump int u_LKeyIsPressed;" +
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

		//Specify the above source code to the shader object
		GLES32.glShaderSource(vertexShaderObjectPerFragment, vertexShaderSourceCodePerFragment);

		//compile the vertex shader source code
		GLES32.glCompileShader(vertexShaderObjectPerFragment);

		//Error Checking
		iShaderCompileStatus[0] = 0;
		iInfoLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetShaderiv(vertexShaderObjectPerFragment, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus,0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(vertexShaderObjectPerFragment, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetShaderInfoLog(vertexShaderObjectPerFragment);
					System.out.println("RTR : "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}


		//FragmentShader Object
		fragmentShaderObjectPerFragment = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

		//Write Shader source code
		final String fragmentShaderSourceCodePerFragment =
			"#version 320 es" +
			"\n" +
			"precision highp float;" +
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
			"uniform mediump int u_LKeyIsPressed;" +
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


		//Specify the above source code to the shader object
		GLES32.glShaderSource(fragmentShaderObjectPerFragment, fragmentShaderSourceCodePerFragment);

		//compile the vertex shader source code
		GLES32.glCompileShader(fragmentShaderObjectPerFragment);

		//Error Checking
		iShaderCompileStatus[0] = 0;
		iInfoLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetShaderiv(fragmentShaderObjectPerFragment, GLES32.GL_COMPILE_STATUS, iShaderCompileStatus,0);
		if (iShaderCompileStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetShaderiv(fragmentShaderObjectPerFragment, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetShaderInfoLog(fragmentShaderObjectPerFragment);
					System.out.println("RTR : "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}

		//Create a shader program Object
		shaderProgramObjectPerFragment = GLES32.glCreateProgram();

		//Attach Vertex Shader to Program
		GLES32.glAttachShader(shaderProgramObjectPerFragment, vertexShaderObjectPerFragment);

		//Attach Fragment Shader to Program
	 	GLES32.glAttachShader(shaderProgramObjectPerFragment, fragmentShaderObjectPerFragment);

		//Prelinking binding Vertex Attributes
		GLES32.glBindAttribLocation(shaderProgramObjectPerFragment, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
		GLES32.glBindAttribLocation(shaderProgramObjectPerFragment, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");


		//Link the program 
		GLES32.glLinkProgram(shaderProgramObjectPerFragment);

		//error checking
		iProgramLinkStatus[0] =0;
		iInfoLength[0] = 0;
		szInfoLog = null;

		GLES32.glGetProgramiv(shaderProgramObjectPerFragment, GLES32.GL_LINK_STATUS, iProgramLinkStatus,0);
		if (iProgramLinkStatus[0] == GLES32.GL_FALSE)
		{
			GLES32.glGetProgramiv(shaderProgramObjectPerFragment, GLES32.GL_INFO_LOG_LENGTH, iInfoLength,0);
			if (iInfoLength[0] > 0)
			{
					szInfoLog=GLES32.glGetProgramInfoLog(shaderProgramObjectPerFragment);
					System.out.println("RTR : "+szInfoLog);
					unInitialize();
					System.exit(0);
			}
		}

		//post linking
		mUniformPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_m_matrix");
		vUniformPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_v_matrix");
		pUniformPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_p_matrix");
		LDUniformOnePerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_LdOne");
		LAUniformOnePerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_LaOne");
		LSUniformOnePerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_LsOne");
		LightPositionUniformOnePerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_Light_PositionOne");

		LDUniformTwoPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_LdTwo");
		LAUniformTwoPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_LaTwo");
		LSUniformTwoPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_LsTwo");
		LightPositionUniformTwoPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_Light_PositionTwo");


		KDUniformPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_Kd");
		KAUniformPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_Ka");
		KSUniformPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_Ks");
		MaterialShininessUniformPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_MaterialShininess");
		
		LKeyIsPressedUniformPerFragment = GLES32.glGetUniformLocation(shaderProgramObjectPerFragment, "u_LKeyIsPressed");


		final float[] PyramidVertices = new float[]{
					0.0f, 1.0f, 0.0f,
					-1.0f, -1.0f, 1.0f,
					1.0f, -1.0f, 1.0f,

					0.0f, 1.0f, 0.0f,
					-1.0f, -1.0f, -1.0f,
					-1.0f, -1.0f, 1.0f,

					0.0f, 1.0f, 0.0f,
					-1.0f, -1.0f, -1.0f,
					1.0f, -1.0f, -1.0f,

					0.0f, 1.0f, 0.0f,
					1.0f, -1.0f, 1.0f,
					1.0f, -1.0f, -1.0f
				};

		final float[] PyramidNormal = new float[]{
			// Front face
			0.0f, 0.447214f, 0.894427f,
			0.0f, 0.447214f, 0.894427f,
			0.0f, 0.447214f, 0.894427f,

			// Right face
			0.894427f, 0.447214f, 0.0f,
			0.894427f, 0.447214f, 0.0f,
			0.894427f, 0.447214f, 0.0f,

			// Back face
			0.0f, 0.447214f, -0.894427f,
			0.0f, 0.447214f, -0.894427f,
			0.0f, 0.447214f, -0.894427f,

			// Left face
			-0.894427f, 0.447214f, 0.0f,
			-0.894427f, 0.447214f, 0.0f,
			-0.894427f, 0.447214f, 0.0f
		};

		//create vao Pyramid
		GLES32.glGenVertexArrays(1, vao_Pyramid,0);
		GLES32.glBindVertexArray(vao_Pyramid[0]);

		GLES32.glGenBuffers(1, vbo_Position_Pyramid,0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Position_Pyramid[0]);

		//Convert the array which can be sent to buffer Data
		ByteBuffer byteBuffer=ByteBuffer.allocateDirect(PyramidVertices.length*4);
		//Arrange the buffer in native byte order
		byteBuffer.order(ByteOrder.nativeOrder());
		//To Float buffer
		FloatBuffer positionBuffer=byteBuffer.asFloatBuffer();
		//Put array in Cooked buffer
		positionBuffer.put(PyramidVertices);
		//Start from 0th position
		positionBuffer.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
			PyramidVertices.length*4,
			positionBuffer,
			GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

		//normal
		GLES32.glGenBuffers(1, vbo_Normal_Pyramid,0);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Normal_Pyramid[0]);

		//Convert the array which can be sent to buffer Data
		ByteBuffer byteBufferPyramidNormal=ByteBuffer.allocateDirect(PyramidNormal.length*4);
		//Arrange the buffer in native byte order
		byteBufferPyramidNormal.order(ByteOrder.nativeOrder());
		//To Float buffer
		FloatBuffer positionBufferPyramidNormal=byteBufferPyramidNormal.asFloatBuffer();
		//Put array in Cooked buffer
		positionBufferPyramidNormal.put(PyramidNormal);
		//Start from 0th position
		positionBufferPyramidNormal.position(0);

		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
			PyramidNormal.length*4,
			positionBufferPyramidNormal,
			GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);
		GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

		GLES32.glBindVertexArray(0);


		GLES32.glClearDepthf(1.0f);
		GLES32.glEnable(GLES32.GL_DEPTH_TEST);
		GLES32.glDepthFunc(GLES32.GL_LEQUAL);
		
		GLES32.glDisable(GLES32.GL_CULL_FACE);
		Matrix.setIdentityM(perspectiveProjectionMatrix,0);

		lightsZero.lightAmbient[0] = 1.0f;
		lightsZero.lightAmbient[1] = 0.0f;
		lightsZero.lightAmbient[2] = 0.0f;
		
		lightsZero.lightDiffused[0] = 1.0f;
		lightsZero.lightDiffused[1] = 0.0f;
		lightsZero.lightDiffused[2] = 0.0f;
		
		lightsZero.lightSpecular[0] = 1.0f;
		lightsZero.lightSpecular[1] = 0.0f;
		lightsZero.lightSpecular[2] = 0.0f;
		
		lightsZero.lightPosition[0] = -2.0f;
		lightsZero.lightPosition[1] = 0.0f;
		lightsZero.lightPosition[2] = 0.0f;
		lightsZero.lightPosition[3] = 1.0f;

		//light1
		lightsOne.lightAmbient[0] = 0.0f;
		lightsOne.lightAmbient[1] = 0.0f;
		lightsOne.lightAmbient[2] = 1.0f;

		lightsOne.lightDiffused[0] = 0.0f;
		lightsOne.lightDiffused[1] = 0.0f;
		lightsOne.lightDiffused[2] = 1.0f;
		
		lightsOne.lightSpecular[0] = 0.0f;
		lightsOne.lightSpecular[1] = 0.0f;
		lightsOne.lightSpecular[2] = 1.0f;


		lightsOne.lightPosition[0] = 2.0f;
		lightsOne.lightPosition[1] = 0.0f;
		lightsOne.lightPosition[2] = 0.0f;
		lightsOne.lightPosition[3] = 1.0f;

	}

	private void resize(int width,int height)
	{
		GLES32.glViewport(0, 0, width, height);
	 	Matrix.perspectiveM(perspectiveProjectionMatrix,0,45.0f, (float)width / (float)height, 0.1f, 100.0f);
	}
	private void display()
	{
		GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT|GLES32.GL_DEPTH_BUFFER_BIT);	
		
		//Declaration of matrices
		float[] modelMatrix=new float[16];
		float[] viewMatrix=new float[16];
		float[] translationMatrix=new float[16];
		float[] rotationMatrix=new float[16];
		if(!gPerFragment)
		{
		GLES32.glUseProgram(shaderProgramObjectPerVertex);
		
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		//Do necessary transformation
		Matrix.translateM(translationMatrix,0,0.0f, 0.0f, -5.0f);
		Matrix.setRotateM(rotationMatrix,0,rotatePyramid,0.0f, 1.0f, 0.0f);
		//Do necessary matrix multiplication	
		Matrix.multiplyMM(modelMatrix ,0,modelMatrix,0,translationMatrix,0);
		Matrix.multiplyMM(modelMatrix ,0,modelMatrix,0,rotationMatrix,0);
		
		//send necessary matrices to shaders
		GLES32.glUniformMatrix4fv(mUniformPerVertex,
			1,
			false,
			modelMatrix,0);
		GLES32.glUniformMatrix4fv(vUniformPerVertex,
			1,
			false,
			viewMatrix,0);
		GLES32.glUniformMatrix4fv(pUniformPerVertex,
			1,
			false,
			perspectiveProjectionMatrix,0);
		if (gLighting == 1)
		{
			GLES32.glUniform1i(LKeyIsPressedUniformPerVertex,
			gLighting);

			GLES32.glUniform4fv(LightPositionUniformOnePerVertex, 1, lightsZero.lightPosition,0);
			GLES32.glUniform3fv(LAUniformOnePerVertex, 1, lightsZero.lightAmbient,0);
			GLES32.glUniform3fv(LDUniformOnePerVertex, 1, lightsZero.lightDiffused,0);
			GLES32.glUniform3fv(LSUniformOnePerVertex, 1, lightsZero.lightSpecular,0);
			
			GLES32.glUniform4fv(LightPositionUniformTwoPerVertex, 1, lightsOne.lightPosition,0);
			GLES32.glUniform3fv(LAUniformTwoPerVertex, 1, lightsOne.lightAmbient,0);
			GLES32.glUniform3fv(LDUniformTwoPerVertex, 1, lightsOne.lightDiffused,0);
			GLES32.glUniform3fv(LSUniformTwoPerVertex, 1, lightsOne.lightSpecular,0);

			GLES32.glUniform3fv(KAUniformPerVertex, 1, materialAmbient,0);
			GLES32.glUniform3fv(KDUniformPerVertex, 1, materialDiffused,0);
			GLES32.glUniform3fv(KSUniformPerVertex, 1, materialSpecular,0);
			GLES32.glUniform1f(MaterialShininessUniformPerVertex, materialShininess);

		}
		else
		{
			GLES32.glUniform1i(LKeyIsPressedUniformPerVertex,gLighting);
		}
		//Bind with vao
		GLES32.glBindVertexArray(vao_Pyramid[0]);

		//Bind with textures

		//Draw necessary scene
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);
        		
		//Unbind vao
        GLES32.glBindVertexArray(0);
		}
		else{

		GLES32.glUseProgram(shaderProgramObjectPerFragment);
		
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);
		Matrix.setIdentityM(rotationMatrix,0);

		//Do necessary transformation
		Matrix.translateM(translationMatrix,0,0.0f, 0.0f, -5.0f);
		Matrix.setRotateM(rotationMatrix,0,rotatePyramid,0.0f, 1.0f, 0.0f);
		//Do necessary matrix multiplication	
		Matrix.multiplyMM(modelMatrix ,0,modelMatrix,0,translationMatrix,0);
		Matrix.multiplyMM(modelMatrix ,0,modelMatrix,0,rotationMatrix,0);
			
		//send necessary matrices to shaders
		GLES32.glUniformMatrix4fv(mUniformPerFragment,
			1,
			false,
			modelMatrix,0);
		GLES32.glUniformMatrix4fv(vUniformPerFragment,
			1,
			false,
			viewMatrix,0);
		GLES32.glUniformMatrix4fv(pUniformPerFragment,
			1,
			false,
			perspectiveProjectionMatrix,0);
		if (gLighting == 1)
		{
			GLES32.glUniform1i(LKeyIsPressedUniformPerFragment,
			gLighting);

			GLES32.glUniform4fv(LightPositionUniformOnePerFragment, 1, lightsZero.lightPosition,0);
			GLES32.glUniform3fv(LAUniformOnePerFragment, 1, lightsZero.lightAmbient,0);
			GLES32.glUniform3fv(LDUniformOnePerFragment, 1, lightsZero.lightDiffused,0);
			GLES32.glUniform3fv(LSUniformOnePerFragment, 1, lightsZero.lightSpecular,0);
			
			GLES32.glUniform4fv(LightPositionUniformTwoPerFragment, 1, lightsOne.lightPosition,0);
			GLES32.glUniform3fv(LAUniformTwoPerFragment, 1, lightsOne.lightAmbient,0);
			GLES32.glUniform3fv(LDUniformTwoPerFragment, 1, lightsOne.lightDiffused,0);
			GLES32.glUniform3fv(LSUniformTwoPerFragment, 1, lightsOne.lightSpecular,0);

			GLES32.glUniform3fv(KAUniformPerFragment, 1, materialAmbient,0);
			GLES32.glUniform3fv(KDUniformPerFragment, 1, materialDiffused,0);
			GLES32.glUniform3fv(KSUniformPerFragment, 1, materialSpecular,0);
			GLES32.glUniform1f(MaterialShininessUniformPerFragment, materialShininess);

		}
		else
		{
			GLES32.glUniform1i(LKeyIsPressedUniformPerFragment,gLighting);
		}
		//Bind with vao
		GLES32.glBindVertexArray(vao_Pyramid[0]);

		//Bind with textures

		//Draw necessary scene
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);
        		
		//Unbind vao
        GLES32.glBindVertexArray(0);
		}
		GLES32.glUseProgram(0);

		requestRender();
	}

	private void unInitialize()
	{
	 	if(vao_Pyramid[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_Pyramid, 0);
            vao_Pyramid[0]=0;
        }
        
        if(vbo_Position_Pyramid[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_Position_Pyramid, 0);
            vbo_Position_Pyramid[0]=0;
        }
        
        if(vbo_Normal_Pyramid[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_Normal_Pyramid, 0);
            vbo_Normal_Pyramid[0]=0;
        }
 
		int[] shaderCount=new int[1];
		int shaderNumber;
		if (shaderProgramObjectPerVertex!=0)
		{
			GLES32.glUseProgram(shaderProgramObjectPerVertex);

			GLES32.glGetProgramiv(shaderProgramObjectPerVertex, GLES32.GL_ATTACHED_SHADERS, shaderCount,0);
			int[] pShaders = new int[4* shaderCount[0]];

			GLES32.glGetAttachedShaders(shaderProgramObjectPerVertex, shaderCount[0], shaderCount,0, pShaders,0);
			for (shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
			{
				GLES32.glDetachShader(shaderProgramObjectPerVertex, pShaders[shaderNumber]);
				GLES32.glDeleteShader(pShaders[shaderNumber]);
				pShaders[shaderNumber] = 0;
			}
				
			GLES32.glDeleteProgram(shaderProgramObjectPerVertex);
			shaderProgramObjectPerVertex = 0;
			GLES32.glUseProgram(0);
		}
		
		if (shaderProgramObjectPerFragment!=0)
		{
			GLES32.glUseProgram(shaderProgramObjectPerFragment);

			GLES32.glGetProgramiv(shaderProgramObjectPerFragment, GLES32.GL_ATTACHED_SHADERS, shaderCount,0);
			int[] pShadersPerFrag = new int[4* shaderCount[0]];

			GLES32.glGetAttachedShaders(shaderProgramObjectPerFragment, shaderCount[0], shaderCount,0, pShadersPerFrag,0);
			for (shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
			{
				GLES32.glDetachShader(shaderProgramObjectPerFragment, pShadersPerFrag[shaderNumber]);
				GLES32.glDeleteShader(pShadersPerFrag[shaderNumber]);
				pShadersPerFrag[shaderNumber] = 0;
			}
				
			GLES32.glDeleteProgram(shaderProgramObjectPerFragment);
			shaderProgramObjectPerFragment = 0;
			GLES32.glUseProgram(0);
		}
	}
}