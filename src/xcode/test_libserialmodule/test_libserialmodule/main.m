//
//  main.m
//  test_libserialmodule
//
//  Created by Thuan T.Nguyen on 3/22/25.
//  Copyright © 2025 salmon. All rights reserved.


#include <serialmodule.h>
#import <Cocoa/Cocoa.h>

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    int ret = 0;
    ret = spl_init_log((char*)"/Users/ntthuan/Library/Developer/Xcode/DerivedData/"\
                       "test_libserialmodule-bgxiutcbhqcyifdcinmepeuqethd/Build/Products/"\
                       "Debug/test_libserialmodule.app/Contents/Resources/simplelog.cfg");
    if(ret) {
        exit(1);
    }	                
    
    ret = NSApplicationMain(argc, argv);
    
    spl_finish_log();
    return ret;
}
	
