package com.ssg.window;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import android.widget.TextView;

import android.graphics.Color;

import android.view.Gravity;

// Packages for fullscreen
import androidx.core.view.WindowCompat;

import androidx.core.view.WindowInsetsControllerCompat;

import androidx.core.view.WindowInsetsCompat;

import android.content.pm.ActivityInfo;

public class MainActivity extends AppCompatActivity
{
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // Fullscreen
        // Get unhide actionbar
        getSupportActionBar().hide();

        // Do fullscreen
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);

        // Get window insets controller object 
        WindowInsetsControllerCompat windowInsetsControllerCompat = WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());

        // Tell above object to hide system bar i.e status bar, navigation bar, caption bar and input method editor
        windowInsetsControllerCompat.hide(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.ime());

        // Do landscape
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

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