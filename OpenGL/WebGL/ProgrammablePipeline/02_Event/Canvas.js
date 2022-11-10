//onload function
function main()
{
    //get <canvas element>
    var canvas=document.getElementById("AMC");
    if(!canvas)
        console.log("Obtaining canvas failed\n");
    else
        console.log("Obtaining canvas successfully");

    console.log("Canvas width: "+canvas.width+" And canvas height: "+canvas.height)    
 
    var context=canvas.getContext("2d");
    if(!context)
        console.log("Obtaining 2D context failed");
    else
        console.log("Obtaining 2D Context Succeeded");    
    //fill the canvas with blackcolor
    context.fillStyle="black"
    context.fillRect(0,0,canvas.width,canvas.height);

    //center the text
    context.fillStyle="green";
    context.textAlign="center";
    context.textBaseline="middle";

    //text
    var str="Hello World !!";
    context.font="48px sans-serif";

    context.fillText(str,canvas.width/2,canvas.height/2);

    //register keyboard's keydown event
    window.addEventListener("keydown",keyDown,false);
    window.addEventListener("click",mouseDown,false);

}

function keyDown(event)
{
    alert("A key is pressed");
}

function mouseDown()
{
    alert("Mouse is Clicked");
}