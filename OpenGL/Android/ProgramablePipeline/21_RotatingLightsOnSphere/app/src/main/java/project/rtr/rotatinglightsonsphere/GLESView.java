package project.rtr.rotatinglightsonsphere;

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
	private int vertexShaderObject;
	private int fragmentShaderObject;
	private int shaderProgramObject;

	private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];
	private int numElements,numVertices;
	private int mUniform, vUniform, pUniform, KDUniform, KAUniform, KSUniform, MaterialShininessUniform, LKeyIsPressedUniform;
	private int LDUniformZero, LAUniformZero, LSUniformZero, LightPositionUniformZero, LDUniformOne, LAUniformOne, LSUniformOne, LightPositionUniformOne, LDUniformTwo, LAUniformTwo, LSUniformTwo, LightPositionUniformTwo;

	private boolean gAnimate = false;
	private int gLighting = 0;
	private float[] perspectiveProjectionMatrix=new float[16];
	private float rotateSphere = 360.0f;
	private float[] lightAmbientZero = new float[]  { 0.0f,0.0f,0.0f,1.0f };
	private float[] lightDiffusedZero = new float[]  { 1.0f,0.0f,0.0f,1.0f };
	private float[] lightSpecularZero = new float[]  { 1.0f,0.0f,0.0f,1.0f };
	private float[] lightPositionZero =new float[]  { 0.0f,0.0f,0.0f,1.0f };
	private float lightAngleZero = 0.0f;

	private float[] lightAmbientOne = new float[]  { 0.0f,0.0f,0.0f,1.0f };
	private float[] lightDiffusedOne = new float[]  { 0.0f,1.0f,0.0f,1.0f };
	private float[] lightSpecularOne =new float[]   { 0.0f,1.0f,0.0f,1.0f };
	private float[] lightPositionOne =new float[]  { 0.0f,0.0f,0.0f,1.0f };
	private float lightAngleOne = 0.0f;

	private float[] lightAmbientTwo =new float[]  { 0.0f,0.0f,0.0f,1.0f };
	private float[] lightDiffusedTwo =new float[]  { 0.0f,0.0f,1.0f,1.0f };
	private float[] lightSpecularTwo =new float[]  { 0.0f,0.0f,1.0f,1.0f };
	private float[] lightPositionTwo =new float[]  { 0.0f,0.0f,0.0f,1.0f };
	private float lightAngleTwo = 0.0f;


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
			rotateSphere = rotateSphere + 0.01f;
				if (rotateSphere >= 360.0f)
				{
					rotateSphere = 0.0f;
				}
			}
	}
	private void Initialize()
	{
		GLES32.glClearColor(0.5f,0.5f,0.5f,1.0f);

		//VertexShader Object
		vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

		//Write Shader source code
		final String vertexShaderSourceCode =
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
			"in vec3 tNormVertexShader;" +
			"in vec4 eye_coordinatesVertexShader;" +
			"uniform vec3 u_LdZero;" +
			"uniform vec3 u_LdOne;" +
			"uniform vec3 u_LdTwo;" +
			"uniform vec3 u_Kd;" +
			"uniform vec3 u_LsZero;" +
			"uniform vec3 u_LsOne;" +
			"uniform vec3 u_LsTwo;" +
			"uniform vec3 u_Ks;" +
			"uniform vec3 u_LaZero;" +
			"uniform vec3 u_LaOne;" +
			"uniform vec3 u_LaTwo;" +
			"uniform vec3 u_Ka;"+ 
			"uniform vec4 u_Light_PositionZero;" +
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
			"vec3 lightDirectionZero=normalize(vec3(u_Light_PositionZero-eye_coordinatesVertexShader));" +
			"vec3 lightDirectionOne=normalize(vec3(u_Light_PositionOne-eye_coordinatesVertexShader));" +
			"vec3 lightDirectionTwo=normalize(vec3(u_Light_PositionTwo-eye_coordinatesVertexShader));" +
			"float tndotldZero=max(dot(lightDirectionZero,tNorm),0.0);" +
			"float tndotldOne=max(dot(lightDirectionOne,tNorm),0.0);" +
			"float tndotldTwo=max(dot(lightDirectionTwo,tNorm),0.0);" +
			"vec3 ReflectionVectorZero=reflect(-lightDirectionZero,tNorm);" +
			"vec3 ReflectionVectorOne=reflect(-lightDirectionOne,tNorm);" +
			"vec3 ReflectionVectorTwo=reflect(-lightDirectionTwo,tNorm);" +
			"vec3 viewerVector=normalize(vec3(-eye_coordinatesVertexShader.xyz));" +
			"vec3 ambientZero=u_LaZero*u_Ka;" +
			"vec3 diffusedZero=u_LdZero*u_Kd*tndotldZero;" +
			"vec3 specularZero=u_LsZero*u_Ks*pow(max(dot(ReflectionVectorZero,viewerVector),0.0),u_MaterialShininess);" +
			"vec3 ambientOne=u_LaOne*u_Ka;" +
			"vec3 diffusedOne=u_LdOne*u_Kd*tndotldOne;" +
			"vec3 specularOne=u_LsOne*u_Ks*pow(max(dot(ReflectionVectorOne,viewerVector),0.0),u_MaterialShininess);" +
			"vec3 ambientTwo=u_LaTwo*u_Ka;" +
			"vec3 diffusedTwo=u_LdTwo*u_Kd*tndotldTwo;" +
			"vec3 specularTwo=u_LsTwo*u_Ks*pow(max(dot(ReflectionVectorTwo,viewerVector),0.0),u_MaterialShininess);" +
			"vec3 phong_ads_light = ambientZero+diffusedZero+specularZero+ambientOne+diffusedOne+specularOne+ambientTwo+diffusedTwo+specularTwo;" +
			"FragColor=vec4(phong_ads_light,1.0);" +
			"}" +
			"else" +
			"{" +
			"FragColor=vec4(1.0,1.0,1.0,1.0);"+
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

		mUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_m_matrix");
		vUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_v_matrix");
		pUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_p_matrix");
		LDUniformZero = GLES32.glGetUniformLocation(shaderProgramObject, "u_LdZero");
		LAUniformZero = GLES32.glGetUniformLocation(shaderProgramObject, "u_LaZero");
		LSUniformZero = GLES32.glGetUniformLocation(shaderProgramObject, "u_LsZero");
		LightPositionUniformZero = GLES32.glGetUniformLocation(shaderProgramObject, "u_Light_PositionZero");

		LDUniformOne = GLES32.glGetUniformLocation(shaderProgramObject, "u_LdOne");
		LAUniformOne = GLES32.glGetUniformLocation(shaderProgramObject, "u_LaOne");
		LSUniformOne = GLES32.glGetUniformLocation(shaderProgramObject, "u_LsOne");
		LightPositionUniformOne = GLES32.glGetUniformLocation(shaderProgramObject, "u_Light_PositionOne");

		LDUniformTwo = GLES32.glGetUniformLocation(shaderProgramObject, "u_LdTwo");
		LAUniformTwo = GLES32.glGetUniformLocation(shaderProgramObject, "u_LaTwo");
		LSUniformTwo = GLES32.glGetUniformLocation(shaderProgramObject, "u_LsTwo");
		LightPositionUniformTwo = GLES32.glGetUniformLocation(shaderProgramObject, "u_Light_PositionTwo");


		KDUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Kd");
		KAUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ka");
		KSUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_Ks");
		MaterialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_MaterialShininess");
		LKeyIsPressedUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_LKeyIsPressed");


		//load sphere in vao -> vbo
		Sphere sphere=new Sphere();
        float sphere_vertices[]=new float[1146];
        float sphere_normals[]=new float[1146];
        float sphere_textures[]=new float[764];
        short sphere_elements[]=new short[2280];
        sphere.getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        numVertices = sphere.getNumberOfSphereVertices();
        numElements = sphere.getNumberOfSphereElements();

        // vao
        GLES32.glGenVertexArrays(1,vao_sphere,0);
        GLES32.glBindVertexArray(vao_sphere[0]);
        
        // position vbo
        GLES32.glGenBuffers(1,vbo_sphere_position,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,vbo_sphere_position[0]);
        
        ByteBuffer byteBuffer=ByteBuffer.allocateDirect(sphere_vertices.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_vertices);
        verticesBuffer.position(0);
        
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                            sphere_vertices.length * 4,
                            verticesBuffer,
                            GLES32.GL_STATIC_DRAW);
        
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,
                                     3,
                                     GLES32.GL_FLOAT,
                                     false,0,0);
        
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);
        
        // normal vbo
        GLES32.glGenBuffers(1,vbo_sphere_normal,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,vbo_sphere_normal[0]);
        
        byteBuffer=ByteBuffer.allocateDirect(sphere_normals.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_normals);
        verticesBuffer.position(0);
        
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                            sphere_normals.length * 4,
                            verticesBuffer,
                            GLES32.GL_STATIC_DRAW);
        
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL,
                                     3,
                                     GLES32.GL_FLOAT,
                                     false,0,0);
        
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);
        
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);
        
        // element vbo
        GLES32.glGenBuffers(1,vbo_sphere_element,0);
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,vbo_sphere_element[0]);
        
        byteBuffer=ByteBuffer.allocateDirect(sphere_elements.length * 2);
        byteBuffer.order(ByteOrder.nativeOrder());
        ShortBuffer elementsBuffer=byteBuffer.asShortBuffer();
        elementsBuffer.put(sphere_elements);
        elementsBuffer.position(0);
        
        GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER,
                            sphere_elements.length * 2,
                            elementsBuffer,
                            GLES32.GL_STATIC_DRAW);
        
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,0);

        GLES32.glBindVertexArray(0);

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
		float[] modelMatrix=new float[16];
		float[] viewMatrix=new float[16];
		float[] translationMatrix=new float[16];
		Matrix.setIdentityM(modelMatrix,0);
		Matrix.setIdentityM(viewMatrix,0);
		Matrix.setIdentityM(translationMatrix,0);

		//Do necessary transformation
		Matrix.translateM(translationMatrix,0,0.0f, 0.0f, -3.0f);

		//Do necessary matrix multiplication	
		Matrix.multiplyMM(modelMatrix ,0,modelMatrix,0,translationMatrix,0);
		
		//send necessary matrices to shaders
		GLES32.glUniformMatrix4fv(mUniform,
			1,
			false,
			modelMatrix,0);
		GLES32.glUniformMatrix4fv(vUniform,
			1,
			false,
			viewMatrix,0);
		GLES32.glUniformMatrix4fv(pUniform,
			1,
			false,
			perspectiveProjectionMatrix,0);
		if (gLighting == 1)
		{
				
			lightPositionZero[0] = 0.0f;
			lightPositionZero[1] = (float)Math.sin(rotateSphere) * 100.0f;
			lightPositionZero[2] = ((float)Math.cos(rotateSphere) * 100.0f) - 4.0f;
			GLES32.glUniform4fv(LightPositionUniformZero, 1, lightPositionZero,0);
			
			lightPositionOne[0] = (float)Math.sin(rotateSphere) * 100.0f;
			lightPositionOne[1] = 0.0f;
			lightPositionOne[2] = ((float)Math.cos(rotateSphere) * 100.0f) - 4.0f;
			GLES32.glUniform4fv(LightPositionUniformOne, 1, lightPositionOne,0);
			
			lightPositionTwo[0] = (float)Math.cos(rotateSphere) * 100.0f;
			lightPositionTwo[1] = (float)Math.sin(rotateSphere) * 100.0f;
			lightPositionTwo[2] = -4.0f;
			GLES32.glUniform4fv(LightPositionUniformTwo, 1, lightPositionTwo,0);

			GLES32.glUniform3fv(LAUniformZero, 1, lightAmbientZero,0);
			GLES32.glUniform3fv(LDUniformZero, 1, lightDiffusedZero,0);
			GLES32.glUniform3fv(LSUniformZero, 1, lightSpecularZero,0);

			GLES32.glUniform3fv(LAUniformOne, 1, lightAmbientOne,0);
			GLES32.glUniform3fv(LDUniformOne, 1, lightDiffusedOne,0);
			GLES32.glUniform3fv(LSUniformOne, 1, lightSpecularOne,0);

			GLES32.glUniform3fv(LAUniformTwo, 1, lightAmbientTwo,0);
			GLES32.glUniform3fv(LDUniformTwo, 1, lightDiffusedTwo,0);
			GLES32.glUniform3fv(LSUniformTwo, 1, lightSpecularTwo,0);

			GLES32.glUniform3fv(KAUniform, 1, materialAmbient,0);
			GLES32.glUniform3fv(KDUniform, 1, materialDiffused,0);
			GLES32.glUniform3fv(KSUniform, 1, materialSpecular,0);
			GLES32.glUniform1f(MaterialShininessUniform, materialShininess);
			GLES32.glUniform1i(LKeyIsPressedUniform,gLighting);
		}
		else
		{
			GLES32.glUniform1i(LKeyIsPressedUniform,gLighting);
		}
		//Bind with vao
		GLES32.glBindVertexArray(vao_sphere[0]);

		//Bind with textures

		//Draw necessary scene
		GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
        		
		//Unbind vao
        GLES32.glBindVertexArray(0);
		GLES32.glUseProgram(0);

		requestRender();
	}

	private void unInitialize()
	{
	 	if(vao_sphere[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_sphere, 0);
            vao_sphere[0]=0;
        }
        
        if(vbo_sphere_position[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_position, 0);
            vbo_sphere_position[0]=0;
        }
        
        if(vbo_sphere_normal[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_normal, 0);
            vbo_sphere_normal[0]=0;
        }
        
        if(vbo_sphere_element[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_element, 0);
            vbo_sphere_element[0]=0;
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