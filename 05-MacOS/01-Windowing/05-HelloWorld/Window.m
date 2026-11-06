#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

@interface AppDelegate:NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface View:NSView
@end

// Main function
int main(int argc, char *argv[])
{
    // Code
    // Create auto release pool for memory management 
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    // Create global shared NSApp object
    NSApp = [NSApplication sharedApplication];


    // Set its deligate to our own custom app deligate 
    [NSApp setDelegate:[[AppDelegate alloc]init]];

    // Start NSApp run loop
    [NSApp run];

    // Let auto release pool release all pending objects in our application
    [pool release];

    return(0);
}

// Implementation of our custom AppDelegate
@implementation AppDelegate
{
    NSWindow *window;
    View *view;
}

- (void)applicationDidFinishLaunching:(NSNotification *) notification
{
    // Code
    // Declare rectangle for our border window
    NSRect win_rect = NSMakeRect(0.0, 0.0, 800.0, 600.0);

    // Create the window
    window = [[NSWindow alloc] initWithContentRect: win_rect
                                        styleMask:(NSWindowStyleMaskTitled |
                                        NSWindowStyleMaskClosable |
                                        NSWindowStyleMaskMiniaturizable |
                                        NSWindowStyleMaskResizable)
                                        backing:NSBackingStoreBuffered
                                        defer:NO];
    
    // Give title to window
    [window setTitle:@"SSG: Cocoa Window"];

    // Centre the window
    [window center];

    // Create the custom view
    view = [[View alloc]initWithFrame:win_rect];

    // Set this newly created custom view as view of our newly created custom window
    [window setContentView:view];
    [window setDelegate:self];

    // Actualy show the window, give it keyboard focus and make it top on Z-order
    [window makeKeyAndOrderFront:self];
}

// Terminate the application
- (void)applicationWillTerminate:(NSNotification *)notification
{
    // Code
}

- (void)windowWillClose:(NSNotification *)notification
{
  // Code
  [NSApp terminate:self];  
}

// Free memory
- (void)dealloc
{
    [super dealloc];
    [view release];
    [window release];
}

@end

// Implementation of custom view interface
@implementation View
{
    NSString *string;
}

- (id)initWithFrame:(NSRect)frame 
{
    // Code
    self = [super initWithFrame:frame];
    if(self)
    {
        string = @"Hello World !!!";
    }

    return(self);
}

// Paint (WM_PAINT)
- (void)drawRect:(NSRect)dirtyRect
{
    // Code
    // Declare the background color to black
    NSColor *bgColor = [NSColor blackColor];

    // Set this color
    [bgColor set];

    // Fill the window rectangle with black color
    NSRectFill(dirtyRect);

    // Create the font for our screen
    NSFont *stringFont = [NSFont fontWithName:@"Helvetica" size:32];

    // Create string color
    NSColor *fgColor = [NSColor greenColor];

    // Create dictionary for our string attribute using above two
    NSDictionary *stringDictionary = [NSDictionary dictionaryWithObjectsAndKeys:stringFont, 
                                                                                NSFontAttributeName, 
                                                                                fgColor, 
                                                                                NSForegroundColorAttributeName,
                                                                                nil];
    
    // Create the size of string according to size of font and other attributes 
    NSSize stringSize = [string sizeWithAttributes:stringDictionary];

    // Create point for drawing of string
    NSPoint stringPoint = NSMakePoint((dirtyRect.size.width/2 - stringSize.width/2),(dirtyRect.size.height/2 - stringSize.height/2));

    // Finally draw the string
    [string drawAtPoint:stringPoint withAttributes:stringDictionary];
}

- (BOOL)acceptsFirstResponder
{
    // Code
    [[self window]makeFirstResponder:self];

    return(YES);
}

- (void)keyDown:(NSEvent *)event
{
    // Code
    int key = (int) [[event characters]characterAtIndex:0];

    switch(key)
    {
        case 27:
            [self release];
            [NSApp terminate:self];
            break;

        case 'f':
        case 'F':
            [[self window]toggleFullScreen:self];
            break;
        
        default :
            break;
    }
}

- (void)dealloc
{
    [super dealloc];
}

@end
