
//headers
#import<Foundation/Foundation.h>
#import<Cocoa/Cocoa.h>

//interface declarations
@interface AppDelegate:NSObject<NSApplicationDelegate,NSWindowDelegate>
@end

@interface MyView:NSView
@end

//Entry point function
int main(int argc,const char *argv[])
{
    NSAutoreleasePool *pPool=[[NSAutoreleasePool alloc]init];

    NSApp=[NSApplication sharedApplication];

    [NSApp setDelegate:[[AppDelegate alloc]init]];

    [NSApp run];
    [pPool release];
    return(0);
}

@implementation AppDelegate
{
@private
    NSWindow *window;
    MyView *view;
}

-(void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //code
    //window
    NSRect win_rect;
    win_rect=NSMakeRect(0.0,0.0,800.0,600.0);
    window=[[NSWindow alloc] initWithContentRect:win_rect styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];
    [window setTitle:@"macOS Window"];
    [window center];   
    view=[[MyView alloc]initWithFrame:win_rect];
    [window setContentView:view];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    //code
}

-(void)applicatonWillCLose:(NSNotification *)notification
{
    [NSApp terminate:self];
}

-(void)dealloc
{
    [view release];
    [window release];
    [super dealloc];
}
@end

@implementation MyView
{
    NSString *centralString;
}

-(id)initWithFrame:(NSRect)frame
{
    //code
    self=[super initWithFrame:frame];
    if(self)
    {
        [[self window]setContentView:self];
        centralString=@"Hello World !!";
    }
    return(self);
}

-(void)drawRect:(NSRect)dirtyRect
{
    NSColor *fillColor=[NSColor blackColor];
    [fillColor set];
    NSRectFill(dirtyRect);

    NSDictionary *dictionaryForTextAttributes=[NSDictionary dictionaryWithObjectsAndKeys:
                            [NSFont fontWithName:@"Helvetica" size:32],NSFontAttributeName,
                            [NSColor greenColor],
                            NSForegroundColorAttributeName,nil];
    NSSize textSize=[centralString sizeWithAttributes:dictionaryForTextAttributes];
    NSPoint point;
    point.x=(dirtyRect.size.width/2)-(textSize.width/2);
    point.y=(dirtyRect.size.height/2)-(textSize.height/2);

    [centralString drawAtPoint:point withAttributes:dictionaryForTextAttributes];
    
}

-(BOOL)acceptsFirstResponder
{
    [[self window]makeFirstResponder:self];
    return(YES);
}

-(void)keyDown:(NSEvent *)theEvent
{
    int key=(int)[[theEvent characters]characterAtIndex:0];
    switch(key)
    {
        case 27:
            [self release];
            [NSApp terminate:self];
            break;
        case 'F':
        case 'f':
            centralString=@"'F' or 'f' key is pressed";
            [[self window]toggleFullScreen:self];
            break;
        default:
            break;        
    }
}

-(void)mouseDown:(NSEvent *)theEvent
{
    //code
    centralString=@"Left Mouse Button Is Pressed";
    [self setNeedsDisplay:YES];
}

-(void)mouseDragged:(NSEvent *)theEvent
{
    //code
}

-(void)rightMouseDown:(NSEvent *)theEvent
{
    //code
    centralString=@"Right mouse Button is clicked";
    [self setNeedsDisplay:YES];
}

-(void) dealloc{
    [super dealloc];
}
@end
