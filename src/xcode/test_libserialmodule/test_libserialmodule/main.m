//
//  main.m
//  test_libserialmodule
//
//  Created by Thuan T.Nguyen on 3/22/25.
//  Copyright © 2025 salmon. All rights reserved.
//

#include <serialmodule.h>
#import <Cocoa/Cocoa.h>

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    int ret = 0;
    ret = spl_init_log((char*)"/Users/ntthuan/Documents/libserialmodule/src/mach/simplelog.cfg");
    if(ret) {
        exit(1);
    }
    
    ret = NSApplicationMain(argc, argv);
    
    spl_finish_log();
    return ret;
}
	
