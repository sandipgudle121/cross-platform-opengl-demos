//
//  MyView.m
//  Window
//
//  Created by Sandip Gudle on 31/10/24.
//

#import "MyView.h"

@implementation MyView
{
    NSString *string;
}

-(id) initWithFrame:(CGRect)frame
{
    // Code
    self = [super initWithFrame:frame];
    if(self)
    {
        // Set background to black
        [self setBackgroundColor:[UIColor blackColor]];
        
        // Set the string
        string = @"Events...";
        
        //--------------------- Single Tap ------------------
        // Create single tap gesture recognizer object
        UITapGestureRecognizer *singleTapGestureRecongnizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onSingleTap:)];
        
        // Set number of taps
        [singleTapGestureRecongnizer setNumberOfTapsRequired:1];
        
        // Set number of fingers
        [singleTapGestureRecongnizer setNumberOfTouchesRequired:1];
        
        // Set the delegate as self
        [singleTapGestureRecongnizer setDelegate:self];
        
        // Add this gesture recognizer to self
        [self addGestureRecognizer:singleTapGestureRecongnizer];
        
        //------------------- Double Tap -----------------
        UITapGestureRecognizer *doubleTapGestureRecongnizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onDoubleTap:)];
        
        [doubleTapGestureRecongnizer setNumberOfTapsRequired:2];
        [doubleTapGestureRecongnizer setNumberOfTouchesRequired:1];
        [doubleTapGestureRecongnizer setDelegate:self];
        [self addGestureRecognizer:doubleTapGestureRecongnizer];
        
        // Double Tap should fail for two single tap
        [singleTapGestureRecongnizer requireGestureRecognizerToFail:doubleTapGestureRecongnizer];
        
        //------------------- Long Press -----------------
        UILongPressGestureRecognizer *longPressGestureRecongnizer = [[UILongPressGestureRecognizer alloc]initWithTarget:self action:@selector(onLongPress:)];
        
        [longPressGestureRecongnizer setDelegate:self];
        [self addGestureRecognizer:longPressGestureRecongnizer];
        
        //------------------- Swipe ---------------------
        UISwipeGestureRecognizer *swipeGestureRecongnizer = [[UISwipeGestureRecognizer alloc]initWithTarget:self action:@selector(onSwipe:)];
        
        [swipeGestureRecongnizer setDelegate:self];
        [self addGestureRecognizer:swipeGestureRecongnizer];
    }
    
    return (self);
}

-(void) drawRect:(CGRect)rect
{
    // Code
    // Declare the background color to black
    UIColor *bgColor = [UIColor blackColor];

    // Set this color
    [bgColor set];

    // Fill the window rectangle with black color
    UIRectFill(rect);

    // Create the font for our screen
    UIFont *stringFont = [UIFont fontWithName:@"Helvetica" size:32];

    // Create string color
    UIColor *fgColor = [UIColor greenColor];

    // Create dictionary for our string attribute using above two
    NSDictionary *stringDictionary = [NSDictionary dictionaryWithObjectsAndKeys:stringFont,
                                                                                NSFontAttributeName,
                                                                                fgColor,
                                                                                NSForegroundColorAttributeName,
                                                                                nil];
    
    // Create the size of string according to size of font and other attributes
    CGSize stringSize = [string sizeWithAttributes:stringDictionary];

    // Create point for drawing the string
    CGPoint stringPoint = CGPointMake((rect.size.width / 2 - stringSize.width / 2),
                                       (rect.size.height / 2 - stringSize.height / 2));

    // Finally draw the string
    [string drawAtPoint:stringPoint withAttributes:stringDictionary];
}

-(BOOL) becomeFirstResponder
{
    // Code
    
    return(YES);
}

-(void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    // Code
}

-(void) onSingleTap:(UITapGestureRecognizer *)gestureRecongnizer
{
    // Code
    string = @"Single Tap";
    [self setNeedsDisplay];
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gestureReconnizer
{
    // Code
    string = @"Double Tap";
    [self setNeedsDisplay];
}

-(void) onSwipe:(UISwipeGestureRecognizer *)gestureRecognizer
{
    // Code
    string = @"Swipe";
    [self setNeedsDisplay];
}

-(void) onLongPress:(UILongPressGestureRecognizer *)gestureRecognizer
{
    // Code
    string = @"Long Press";
    [self setNeedsDisplay];
}

-(void) dealloc
{
    // Code
    [super dealloc];
}
@end
