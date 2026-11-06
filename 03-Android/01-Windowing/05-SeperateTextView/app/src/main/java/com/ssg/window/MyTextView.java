package com.ssg.window;

import android.content.Context;

import androidx.appcompat.widget.AppCompatTextView;
//import android.widget.TextView;

import android.graphics.Color;

import android.view.Gravity;

public class MyTextView extends AppCompatTextView
{
    public MyTextView(Context context)
    {
        super(context);

        setTextColor(Color.rgb(0,255,0));
        setTextSize(60);
        setGravity(Gravity.CENTER);
        setText("Sandip Sham Gudle");
    }
}
