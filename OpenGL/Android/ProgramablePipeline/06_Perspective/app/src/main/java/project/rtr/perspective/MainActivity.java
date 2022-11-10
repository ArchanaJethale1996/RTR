package project.rtr.perspective;

//Default given package
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

//added extras
import android.view.Window;
import android.view.WindowManager;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
public class MainActivity extends AppCompatActivity {
	private GLESView glesView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.activity_main);
    
		//Get rid of title bar 
		this.supportRequestWindowFeature(Window.FEATURE_NO_TITLE);

		//Make Full Screen
		this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);

		//Landscape Force
		this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

		//background color
		this.getWindow().getDecorView().setBackgroundColor(Color.BLACK);

		//define our own view
		glesView=new GLESView(this);
		setContentView(glesView);
	}
	@Override
	protected void onPause()
	{
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		super.onResume();
	}
}
