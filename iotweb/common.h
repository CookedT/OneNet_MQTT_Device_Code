#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stddef.h>

#include "Arduino.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define VERBOSE       1
#define INFO          2
#define DEBUG         3
#define WARNING       4
#define ERROR         5
#define NONE          6

#define __LEVEL__   VERBOSE

#define LOG

#ifdef LOG
#define LOG(level, format, ...)  \
        if( level >= __LEVEL__ ) {   \
	        printf("[%d]%s(%d): ",level,__FUNCTION__,__LINE__);\
	        printf(format,##__VA_ARGS__);\
		}
#else
#define LOG(level, format, ...) {}
#endif


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus



#endif



