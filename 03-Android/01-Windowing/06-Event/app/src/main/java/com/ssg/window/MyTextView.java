package com.ssg.window;

import android.content.Context;
import androidx.appcompat.widget.AppCompatTextView;

import android.graphics.Color;
import android.view.Gravity;

// Event related packages
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnDoubleTapListener;
import android.view.GestureDetector.OnGestureListener;

public class MyTextView extends AppCompatTextView implements OnDoubleTapListener, OnGestureListener
{
    // Class variables
    private GestureDetector gesture_detector;

    public MyTextView(Context context)
    {
        super(context);

        setTextColor(Color.rgb(0,255,0));
        setTextSize(50);
        setGravity(Gravity.CENTER);
        setText("Sandip Sham Gudle");

        // Create and set gesture detector object
        gesture_detector = new GestureDetector(context, this, null, false);
        gesture_detector.setOnDoubleTapListener(this);
    }

    // Implementation on touch event of view class
    @Override
    public boolean onTouchEvent(MotionEvent e)
    {
        if(!gesture_detector.onTouchEvent(e))
        {
            super.onTouchEvent(e);
        }

        return(true);
    }

    // Implementation of on three methods of on double tap listner interface
    @Override
    public boolean onDoubleTap(MotionEvent e)
    {
        setText("Double tap = SSG");

        return(true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
    {
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        setText("Single tap = SSG");

        return(true);
    }

    // Six methods of gesture listner interface
    @Override
    public boolean onDown(MotionEvent e)
    {
        return(true);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {
        return(true);
    }

    @Override
    public void onLongPress(MotionEvent e)
    {
        setText("Long press = SSG");
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
    {
        setText("Scroll/Swipe = SSG");

        return(true);
    }

    @Override
    public void  onShowPress(MotionEvent e)
    {

    }

    @Override
    public boolean onSingleTapUp(MotionEvent e)
    {
        return(true);
    }
}
