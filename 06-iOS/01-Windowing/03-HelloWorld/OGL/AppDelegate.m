
//
//  AppDelegate.m
//  Window
//
//  Created by Sandip Gudle on 31/10/24.
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "MyView.h"

@implementation AppDelegate
{
@private
    UIWindow *window;
    ViewController *viewController;
    MyView *myView;
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
    myView = [[MyView alloc]initWithFrame:screenRect];
    
    // Set this view as view of view controller
    [viewController setView:myView];
    
    // Decrement the refrence count of view incremented by set view
    [myView release];
    
    // Set focus on this window and make it visible
    [window makeKeyAndVisible];

    return YES;
}

-(void) applicationWillResignActive:(UIApplication *)application
{
    // Code
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
}

-(void) applicationWillTerminate:(UIApplication *)application
{
    // Code
}

-(void) dealloc
{
    // Code
    [super dealloc];
    [myView release];
    [viewController release];
    [window release];
}
@end
