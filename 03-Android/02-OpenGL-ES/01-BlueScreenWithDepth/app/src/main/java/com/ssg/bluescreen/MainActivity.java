package com.ssg.bluescreen;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.graphics.Color;

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
        // Get unhide action bar
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

        GLESView myTextView = new GLESView(this);
        
        setContentView(myTextView);
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