var canvas = null;
var context = null;

// Our main fuction
function main()
{
    // Get canvas
    canvas = document.getElementById("SSG");
    if(canvas == null)
    {
        console.log("Getting canvas failed\n");
    }
    else
    {
        console.log("Getting canvas succeeded\n");
    }

    // Get context from above canvas
    context = canvas.getContext("2d");
    if(context == null)
    {
        console.log("Getting context failed\n");
    }
    else
    {
        console.log("Getting context succeeded\n");
    }

    // Set canvas background to black
    context.fillStyle = "black";

    // Paint the canvas rectangle with above color
    context.fillRect(0, 0, canvas.width, canvas.height);

    // Draw text
    drawText("Hello World !!!");

    // Register for keyboard events
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
}

function keyDown(event)
{
    // Code
    switch(event.keyCode)
    {
        case 70 :
        case 120 :
            toggleFullscreen();
            //drawText("Hello World !!!");
            break;
    }
}

function mouseDown()
{
    
}

function drawText(text)
{
    // Code
    // Set font 
    context.font = "48px sans-serif";

    // Set the text in middle of canvas
    context.textAlign = "center";
    context.textBaseline = "middle";

    // Set text color to white
    context.fillStyle = "lime";

    // Paint the text with above color
    context.fillText(text, canvas.width/2, canvas.height/2);
}

function toggleFullscreen()
{
    var fullscreen_element = document.fullscreenElement ||
                             document.webkitFullscreenElement ||
                             document.mozFullScreenElement ||
                             document.msFullscreenElement ||
                             null;

    if(fullscreen_element == null)
    {
        if(canvas.requstFullscreen)
        {
            canvas.requstFullscreen();
        }
        else if(canvas.webkitRequestFullscreen)
        {
            canvas.webkitRequestFullscreen();
        }
        else if(canvas.mozRequestFullScreen)
        {
            canvas.mozRequestFullScreen();
        }
        else if(canvas.msRequestFullscreen)
        {
            canvas.msRequestFullscreen();
        }
        
    }
    else
    {
        if(document.exitFullscreen)
        {
            document.exitFullscreen();
        }
        else if(document.webkitExitFullscreen)
        {
            document.webkitExitFullscreen();
        }
        else if(document.mozCancelFullScreen)
        {
            document.mozCancelFullScreen();
        }
        else if(document.msExitFullscreen)
        {
            document.msExitFullscreen();
        }
    }
}

