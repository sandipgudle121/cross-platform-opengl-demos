//
//  MyView.h
//  Window
//
//  Created by Sandip Gudle on 31/10/24.
//

#import <UIKit/UIKit.h>
#import "GLESView.h"

@interface GLESView : UIView <UIGestureRecognizerDelegate>
-(void) startDisplayLink;
-(void) stopDisplayLink;
@end
