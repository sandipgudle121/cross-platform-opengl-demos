package com.ssg.window;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity
{
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        TextView myTextView = new TextView(this);
        myTextView.setText("Hello World !!!");
        setContentView(myTextView);
    } 
}