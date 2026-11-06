//
//  main.m
//  Window
//
//  Created by Sandip Gudle on 31/10/24.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char *argv[])
{
    // Code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];
    
    // Get AppDelegate class name as string
    NSString *delegateClassName = NSStringFromClass([AppDelegate class]);
    
    UIApplicationMain(argc, argv, nil, delegateClassName);
    
    [pool release];
    
    return(0);
}
