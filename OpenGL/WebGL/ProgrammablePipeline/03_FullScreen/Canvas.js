var canvas=null;
var context=null;

//onload function
function main()
{
    //get <canvas element>
    canvas=document.getElementById("AMC");
    if(!canvas)
        console.log("Obtaining canvas failed\n");
    else
        console.log("Obtaining canvas successfully");

    console.log("Canvas width: "+canvas.width+" And canvas height: "+canvas.height)    
 
    context=canvas.getContext("2d");
    if(!context)
        console.log("Obtaining 2D context failed");
    else
        console.log("Obtaining 2D Context Succeeded");    

   //fill the canvas with blackcolor
    context.fillStyle="black"
    context.fillRect(0,0,canvas.width,canvas.height);

    drawText("Hello World !!");
 
    //register keyboard's keydown event
    window.addEventListener("keydown",keyDown,false);
    window.addEventListener("click",mouseDown,false);

}

function drawText(text)
{
   //center the text
   context.fillStyle="green";
   context.textAlign="center";
   context.textBaseline="middle";

   //text
   context.font="48px sans-serif";

   context.fillText(text,canvas.width/2,canvas.height/2);
}

function toggleFullScreen()
{
    //code
    var fullscreen_element=document.fullscreenElement||
                        document.webkitFullscreenElement||
                        document.mozFullScreenElement||
                        document.msFullscreenElement||
                        null;

    if(fullscreen_element==null)
    {
        if(canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if(canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if(canvas.webkitRequestFullscreen)
            canvas.webkitRequestFullscreen();
        else if(canvas.msRequestFullscreen)
            canvas.msRequestFullscreen();
    }   
    else
    {
        if(document.exitFullscreen)
            document.exitFullscreen();
        else if(document.mozCancelFullScreen) 
            document.mozCancelFullScreen();
        else if(document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if(document.msExitFullscreen)
            document.msExitFullscreen();
            
    }                 
}

function keyDown(event)
{
    switch(event.keyCode)
    {
        case 70:
            toggleFullScreen();
            drawText("Hello World !!");
            break;
    }
}

function mouseDown()
{}