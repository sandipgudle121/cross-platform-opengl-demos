// Our main fuction
function main()
{
    // Get canvas
    var canvas = document.getElementById("SSG");
    if(canvas == null)
    {
        console.log("Getting canvas failed\n");
    }
    else
    {
        console.log("Getting canvas succeeded\n");
    }

    // Get context from above canvas
    var context = canvas.getContext("2d");
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

    // Declare the text
    var text = "Hello World !!!";

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