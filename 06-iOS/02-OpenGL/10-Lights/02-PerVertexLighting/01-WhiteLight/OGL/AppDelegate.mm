//
//  AppDelegate.m
//  Window
//
//  Created by Sandip Gudle on 31/10/24.
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "GLESView.h"

@implementation AppDelegate
{
@private
    UIWindow *window;
    ViewController *viewController;
    GLESView *glesView;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Code
    // Get the iphone screen rectangle
    CGRect screenRect = [[UIScreen mainScreen]bounds];
    
    // Create Window
    window = [[UIWindow alloc]initWithFrame:screenRect];
    
    // Create View Controller
    viewController = [[ViewController alloc]init];
    
    // Set this view controller as window's view controller
    [window setRootViewController:viewController];
    
    // Create our custum view
    glesView = [[GLESView alloc]initWithFrame:screenRect];
    
    // Set this view as view of view controller
    [viewController setView:glesView];
    
    // Decrement the refrence count of view incremented by set view
    [glesView release];
    
    // Set focus on this window and make it visible
    [window makeKeyAndVisible];
    
    // Start displayLink
    [glesView startDisplayLink];

    return YES;
}

-(void) applicationWillResignActive:(UIApplication *)application
{
    // Code
    // Stop displayLink
    [glesView stopDisplayLink];
}

-(void) applicationDidEnterBackground:(UIApplication *)application
{
    // Code
}

-(void) applicationWillEnterForeground:(UIApplication *)application
{
    // Code
}

-(void) applicationDidBecomeActive:(UIApplication *)application
{
    // Code
    // Start displayLink
    [glesView startDisplayLink];
}

-(void) applicationWillTerminate:(UIApplication *)application
{
    // Code
    // Stop displayLink
    [glesView stopDisplayLink];
}

-(void) dealloc
{
    // Code
    [super dealloc];
    [glesView release];
    [viewController release];
    [window release];
}
@end
