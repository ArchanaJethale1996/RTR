package project.rtr.checkerboard;

//Default given package
import android.content.Context;
import android.graphics.Color;
import android.view.Gravity;

//For Motion Event
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

import android.opengl.GLES10;

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

//For texture
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.opengl.GLUtils;


public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer,OnGestureListener, OnDoubleTapListener {
private final Context context;
    private GestureDetector gestureDetector = null;
    private final int CHECK_IMAGE_WIDTH = 64;
    private final int CHECK_IMAGE_HEIGHT = 64;

    private int vertexShaderObject = 0;
    private int fragmentShaderObject = 0;
    private int shaderProgramObject = 0;
    private int mvpUniform = 0;
    private int samplerUniform = 0;

    private int[] vao_Rectangle = new int[1];
    private int[] vbo_Position_Rectangle = new int[1];
    private int[] vbo_Texture_Rectangle = new int[1];
    private int[] texture_check = new int[1];
    private int[] checkImage = new int[CHECK_IMAGE_HEIGHT * CHECK_IMAGE_WIDTH];

    private float[] perspectiveProjectionMatrix = new float[16];


    GLESView(Context context) {
        super(context);
        this.context = context;

        setEGLContextClientVersion(3);
        setRenderer(this);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        gestureDetector = new GestureDetector(context, this, null, false);
        
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
		//System.out.println("RTR: "+version);
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


    private void Initialize() {
		int[] infoLogLength = new int[1];
        int[] shaderCompileStatus = new int[1];
        String infoLog = null;

        vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

        final String vertexShaderSourceCode = String.format(
            "#version 320 es"
            + "\n"
            + "in vec4 vPosition;"
            + "in vec2 vTexCoord0;"
            + "out vec2 vTextCoord;"
            + "uniform mat4 mvpMatrix;"
            + "void main(void)"
            + "{"
            + "   gl_Position = mvpMatrix * vPosition;"
            + "   vTextCoord = vTexCoord0;"
            + "}"
        );

        GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);
        GLES32.glCompileShader(vertexShaderObject);

        
        GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS, shaderCompileStatus, 0);
        if(shaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);
                System.out.println("Vertex shader compilation log: " + infoLog);
                unInitialize();
                System.exit(1);
            }
        }

		fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        final String fragmentShaderSourceCode = String.format(
            "#version 320 es"
            + "\n"
            + "precision highp float;"
            + "in vec2 vTextCoord;"
            + "out vec4 fragmentColor;"
            + "uniform sampler2D sampler;"
            + "void main(void)"
            + "{"
            + "   fragmentColor = texture(sampler, vTextCoord);"
            + "}"

        );

        GLES32.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);
        GLES32.glCompileShader(fragmentShaderObject);

        GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS, shaderCompileStatus, 0);
        if(shaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);
                System.out.println("Fragment shader compilation log: " + infoLog);
                unInitialize();
                System.exit(1);
            }
        }
        
		
		
		shaderProgramObject = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);
        GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);

        GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");

        GLES32.glBindAttribLocation(shaderProgramObject, GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, "vTexCoord0");

        GLES32.glLinkProgram(shaderProgramObject);
        
        int[] shaderProgramLinkStatus = new int[1];
        
        GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS, shaderProgramLinkStatus, 0);

        if(shaderProgramLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH, infoLogLength, 0);
            if(infoLogLength[0] > 0)
            {
                infoLog = GLES32.glGetProgramInfoLog(shaderProgramObject);
                System.out.println("Shader program link log: " + infoLog);
                unInitialize();
                System.exit(1);
            }
        }

        mvpUniform = GLES32.glGetUniformLocation(shaderProgramObject, "mvpMatrix");
        samplerUniform = GLES32.glGetUniformLocation(shaderProgramObject, "sampler");

        final float[] checkTexCoord = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        GLES32.glGenVertexArrays(1, vao_Rectangle, 0);
        GLES32.glBindVertexArray(vao_Rectangle[0]);
        GLES32.glGenBuffers(1, vbo_Position_Rectangle, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Position_Rectangle[0]);        
		GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, 12 * 4, null, GLES32.GL_DYNAMIC_DRAW);        
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glGenBuffers(1, vbo_Texture_Rectangle, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Texture_Rectangle[0]);
        ByteBuffer textureCoordinatesByteBuffer = ByteBuffer.allocateDirect(checkTexCoord.length * 4);        
		textureCoordinatesByteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer textureCoordinatesBuffer = textureCoordinatesByteBuffer.asFloatBuffer();
        textureCoordinatesBuffer.put(checkTexCoord);
        textureCoordinatesBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, checkTexCoord.length * 4, textureCoordinatesBuffer, GLES32.GL_STATIC_DRAW);
		GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, GLES32.GL_FLOAT, false, 0, 0);
		GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);


        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);

        loadCheckerboardTexture();
    }


    private void display() {
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT | GLES32.GL_STENCIL_BUFFER_BIT);

        GLES32.glUseProgram(shaderProgramObject);
        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];
        Matrix.setIdentityM(modelViewMatrix, 0);
        Matrix.setIdentityM(modelViewProjectionMatrix, 0);

        Matrix.translateM(modelViewMatrix, 0, 0.0f, 0.0f, -3.6f);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);
         GLES32.glBindVertexArray(vao_Rectangle[0]);
        float[] squareVertices = {
            0.0f, 1.0f, 0.0f,
            -2.0f, 1.0f, 0.0f,
            -2.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f
        };

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Position_Rectangle[0]);
        ByteBuffer verticesByteBuffer = ByteBuffer.allocateDirect(squareVertices.length * 4);
        verticesByteBuffer.order(ByteOrder.nativeOrder());

        FloatBuffer verticesBuffer = verticesByteBuffer.asFloatBuffer();
        verticesBuffer.put(squareVertices);
        verticesBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, squareVertices.length * 4, verticesBuffer, GLES32.GL_DYNAMIC_DRAW);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture_check[0]);
        GLES32.glUniform1i(samplerUniform, 0);

        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glBindVertexArray(0);

        GLES32.glBindVertexArray(vao_Rectangle[0]);

        float[] tiltedSquareVertices = {
            2.41421f, 1.0f, -1.41421f,
            1.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            2.41421f, -1.0f, -1.41421f
        };

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_Position_Rectangle[0]);
        ByteBuffer verticesByteBufferB = ByteBuffer.allocateDirect(tiltedSquareVertices.length * 4);
        verticesByteBufferB.order(ByteOrder.nativeOrder());

        FloatBuffer verticesBufferB = verticesByteBufferB.asFloatBuffer();
        verticesBufferB.put(tiltedSquareVertices);
        verticesBufferB.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, tiltedSquareVertices.length * 4, verticesBufferB, GLES32.GL_DYNAMIC_DRAW);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture_check[0]);

        GLES32.glUniform1i(samplerUniform, 0);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glBindVertexArray(0);

        GLES32.glUseProgram(0);
        requestRender();
    }


    
    private void loadCheckerboardTexture() {
        makeCheckImage();
        Bitmap bitmap = Bitmap.createBitmap(checkImage, CHECK_IMAGE_WIDTH, CHECK_IMAGE_HEIGHT, Bitmap.Config.ARGB_8888);
        GLES32.glGenTextures(1, texture_check, 0);
        GLES32.glPixelStorei(GLES32.GL_UNPACK_ALIGNMENT, 1);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture_check[0]);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_S, GLES32.GL_REPEAT);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_T, GLES32.GL_REPEAT);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_NEAREST);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_NEAREST);

        GLUtils.texImage2D(GLES32.GL_TEXTURE_2D, 0, bitmap, 0);
        GLES10.glTexEnvf(GLES10.GL_TEXTURE_ENV, GLES10.GL_TEXTURE_ENV_MODE, GLES10.GL_REPLACE);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);
    }

    void makeCheckImage() {
        int i = 0;
        int j = 0;

        for (i = 0; i < CHECK_IMAGE_HEIGHT; ++i)
        {
            for (j = 0; j < CHECK_IMAGE_WIDTH; ++j)
            {
                if(((i & 0x8) ^ (j & 0x8)) != 0) {
                    checkImage[i * CHECK_IMAGE_HEIGHT + j] = android.graphics.Color.WHITE;
                }
                else {
                    checkImage[i * CHECK_IMAGE_HEIGHT + j] = android.graphics.Color.BLACK;
                }
            }
        }
    }

    private void resize(int width, int height) {
        if(height == 0) {
            height = 1;
        }

        GLES32.glViewport(0, 0, width, height);
        Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 60.0f, (float)width / (float)height, 1.0f, 30.0f);
    }

    private void unInitialize() {
        if(vao_Rectangle[0] != 0) {
            GLES32.glDeleteVertexArrays(1, vao_Rectangle, 0);
            vao_Rectangle[0] = 0;
        }

        if(vbo_Position_Rectangle[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_Position_Rectangle, 0);
            vbo_Position_Rectangle[0] = 0;
        }

        if(vbo_Texture_Rectangle[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_Texture_Rectangle, 0);
            vbo_Texture_Rectangle[0] = 0;
        }

        if(shaderProgramObject != 0) {
            if(vertexShaderObject != 0) {
                GLES32.glDetachShader(shaderProgramObject, vertexShaderObject);
            }

            if(fragmentShaderObject != 0) {
                GLES32.glDetachShader(shaderProgramObject, fragmentShaderObject);
            }
        }

        if(vertexShaderObject != 0) {
            GLES32.glDeleteShader(vertexShaderObject);
            vertexShaderObject = 0;
        }

        if(fragmentShaderObject != 0) {
            GLES32.glDeleteShader(fragmentShaderObject);
            fragmentShaderObject = 0;
        }

        if(shaderProgramObject != 0) {
            GLES32.glDeleteProgram(shaderProgramObject);
            shaderProgramObject = 0;
        }

        GLES32.glUseProgram(0);
        if (texture_check[0] != 0) {
            GLES32.glDeleteTextures(1, texture_check, 0);
            texture_check[0] = 0;
        }
    }	
}