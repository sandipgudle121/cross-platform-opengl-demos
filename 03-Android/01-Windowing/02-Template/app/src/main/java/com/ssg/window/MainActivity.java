package com.ssg.window;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import android.widget.TextView;

import android.graphics.Color;

import android.view.Gravity;

public class MainActivity extends AppCompatActivity
{
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // Set background color to black
        getWindow().getDecorView().setBackgroundColor(Color.BLACK);

        TextView myTextView = new TextView(this);
        myTextView.setTextColor(Color.rgb(0,255,0));
        myTextView.setTextSize(60);
        myTextView.setGravity(Gravity.CENTER);
        myTextView.setText("Hello World !!!");
        setContentView(myTextView);
    }
}