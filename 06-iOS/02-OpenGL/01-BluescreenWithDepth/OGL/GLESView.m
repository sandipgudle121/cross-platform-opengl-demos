//
//  MyView.m
//  Window
//
//  Created by Sandip Gudle on 31/10/24.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3//glext.h>
#import "GLESView.h"

@implementation GLESView
{
    EAGLContext *eaglContext;
    
    GLuint custumFrameBuffer;
    GLuint colorRenderBuffer;
    GLuint depthRenderBuffer;
    
    id displayLink;
    NSInteger framesPerSecond;
    BOOL isDisplayLink;
}

-(id) initWithFrame:(CGRect)frame
{
    // Code
    self = [super initWithFrame:frame];
    if(self)
    {
        // Set background to black
        [self setBackgroundColor:[UIColor blackColor]];
        
        // Create CAGL Layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)[super layer];
        
        [eaglLayer setOpaque:YES];
        
        NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        [eaglLayer setDrawableProperties:dictionary];
        
        // Create eagl context
        eaglContext = [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if(eaglContext == nil)
        {
            printf("OpenGLES context creation failed\n");
            [self uninitialize];
            [self release];
            exit(0);
        }
        
        // Set this context as current context
        [EAGLContext setCurrentContext:eaglContext];
        
        // Create custom framebuffer
        glGenFramebuffers(1, &custumFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, custumFrameBuffer);
        
        // Create color render buffer
        glGenRenderbuffers(1, &colorRenderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
        
        // Give storage to color renderbuffer using iOS method
        [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        
        // Assign above colorbuffer to custom colorbuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderBuffer);
        
        // Depth renderbuffer
        // Find width of colorbuffer
        GLint width;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        
        // Find height of colorbuffer
        GLint height;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
        
        // Create and bind with depth render buffer
        glGenRenderbuffers(1, &depthRenderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
        
        // Give storage to depth renderbuffer using OpenGL method
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        
        // Give this depth renderbuffer to custom frambuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
    
        // Check Status of newly created custom buffer
        GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Framebuffer status is not complete\n");
            [self uninitialize];
            [self release];
            exit(0);
        }
        
        // Initialize frames per second
        framesPerSecond = 60;       // 60 is recommended for iOS 8.2
        
        // Initialize display link existing variable
        isDisplayLink = NO;
        
        // Call our initialize method
        int result = [self initialize];
        if(result != 0)
        {
            printf("Initialization failed\n");
            [self uninitialize];
            [self release];
            exit(0);
        }
        
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

+(Class) layerClass
{
    // Code
    
    return([CAEAGLLayer class]);
}

//-(void) drawRect:(CGRect)rect
//{
//    // Code
//}


- (void)drawView:(id)displayLink
{
    [EAGLContext setCurrentContext:eaglContext];
    glBindFramebuffer(GL_FRAMEBUFFER, custumFrameBuffer);
    
    [self display];
    [self update];
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)[self layer]];
    
    GLint width;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    
    GLint height;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    // Bind with depth render buffer
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    
    // Give storage to depth renderbuffer using OpenGL method
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    
    glBindFramebuffer(GL_FRAMEBUFFER, custumFrameBuffer);
    
    GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer status is not complete\n");
        [self uninitialize];
        [self release];
        exit(0);
    }
    
    [self resize:width :height];
    [self drawView:displayLink]; // Warm-Up resize();
}

// start displayLink custom method which will be called by AppDelegate.m
-(void) startDisplayLink
{
    // Code
    if(isDisplayLink == NO)
    {
        // Create displayLink
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
        
        // Set frams per second for displayLink
        [displayLink setPreferredFramesPerSecond:framesPerSecond];
        
        // Add this displayLink to our run loop
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        // Set isDisplayLink to YES
        isDisplayLink = YES;
    }
}

-(void) stopDisplayLink
{
    // Code
    if(isDisplayLink == YES)
    {
        // Remove displayLink from runloop by invalidating it
        [displayLink invalidate];
        
        isDisplayLink = NO;
    }
}

- (int)initialize
{
    // Code
    [self printGLInfo];

    // Set Depth
    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set ClearColor
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

    return(0);
}

- (void)printGLInfo
{
    int numExtensions;
    int i;

    printf("OpenGL Vender : %s\n",glGetString(GL_VENDOR));
    printf("OpenGL Renderer : %s\n",glGetString(GL_RENDERER));
    printf("OpenGL Version : %s \n",glGetString(GL_VERSION));
    printf("OpenGL Shading language version(GLSL) : %s \n",glGetString(GL_SHADING_LANGUAGE_VERSION));

    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(i = 0; i < numExtensions; i++)
    {
        printf("%s\n",glGetStringi(GL_EXTENSIONS, i));
    }
}

- (void) resize:(int)width :(int)height
{
    // Code
    if(height <= 0)
    {
        height = 1;
    }

    glViewport(0,0, (GLsizei)width, (GLsizei)height);
}

- (void) display
{
    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

- (void) update
{
    // Code
}

- (void) uninitialize
{
    // Code
    // Delete depth renderbuffer
    if(depthRenderBuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderBuffer);
        depthRenderBuffer = 0;
    }
    
    // Delete color renderbuffer
    if(colorRenderBuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderBuffer);
        colorRenderBuffer = 0;
    }
    
    // Delete custom framebuffer
    if(custumFrameBuffer)
    {
        glDeleteFramebuffers(1, &custumFrameBuffer);
        custumFrameBuffer = 0;
    }
    
    // Release EAGL context
    if(eaglContext && [EAGLContext currentContext] == eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
        [eaglContext release];
        eaglContext = nil;
    }
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
}

-(void) onDoubleTap:(UITapGestureRecognizer *)gestureReconnizer
{
    // Code
}

-(void) onSwipe:(UISwipeGestureRecognizer *)gestureRecognizer
{
    // Code
    [self uninitialize];
    [self release];
    exit(0);
}

-(void) onLongPress:(UILongPressGestureRecognizer *)gestureRecognizer
{
    // Code
}

-(void) dealloc
{
    // Code
    [super dealloc];
    [self uninitialize];
    
    // Release displayLink
    if(displayLink)
    {
        [displayLink invalidate];
        [displayLink stop];
        [displayLink release];
        displayLink = nil;
    }
}
@end


