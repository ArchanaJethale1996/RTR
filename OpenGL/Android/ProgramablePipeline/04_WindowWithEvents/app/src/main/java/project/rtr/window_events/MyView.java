package project.rtr.window_events;

//Default given package
import androidx.appcompat.widget.AppCompatTextView;
import android.content.Context;
import android.graphics.Color;
import android.view.Gravity;

//For Motion Event
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

public class MyView extends AppCompatTextView implements OnGestureListener, OnDoubleTapListener {
	private GestureDetector gestureDetector;

	public MyView(Context drawingContext)
	{
		super(drawingContext);
		setTextColor(Color.rgb(0,255,0));
		setTextSize(60);
		setGravity(Gravity.CENTER);
		setText("Hello World");

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
		setText("Double Tap");
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
		setText("Single Tap");
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
		setText("Long Press");
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
		setText("Scroll");
		return true;
	}
	@Override
	public boolean onFling(MotionEvent event1,MotionEvent event2,float velocityX,float velocityY)
	{
		return true;
	}
}
