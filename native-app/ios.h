#ifndef DR_SOCKET_IOS_H
#define DR_SOCKET_IOS_H

#if __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/objc-auto.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#define IOS

#endif
#endif
#endif