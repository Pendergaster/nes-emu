/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef UTILSDEFS
#define UTILSDEFS

#include <inttypes.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef uint64_t    u64;
typedef int64_t     i64;
typedef uint32_t    u32;
typedef int32_t     i32;
typedef uint16_t    u16;
typedef int16_t     i16;
typedef uint8_t     u8;
typedef int8_t      i8;

static const u8  numeric_max_u8  = 0xFF;
static const u16 numeric_max_u16 = 0xFFFF;
static const u32 numeric_max_u32 = 0xFFFFFFFF;
static const u64 numeric_max_u64 = 0xFFFFFFFFFFFFFFFF;

#define VA_ARGS(...) , ##__VA_ARGS__

//#define free(PTR) do{free((PTR));(PTR) = NULL;}while(0)

#define PTR_CAST(TYPE, VAL) (((union {typeof(VAL) src; TYPE dst;}*)(&VAL))->dst)

#define typeof __typeof__

#define BIT_CHECK(a,b) ((a & b) > 0)
#define BIT_SET(a,b) ( a |= b)
#define BIT_UNSET(a,b) (a &= ~b)

#define KILOS(NUM) (NUM * 1000)
#define MEGAS(NUM) (NUM * 1000000)
#define GIGAS(NUM) (NUM * 1000000000)

// NULL pointer should evaluate to 0
#define SIZEOF_ARRAY(ARR) (sizeof((ARR)) / sizeof(*(ARR)))
#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

// no tricks, can swap same value, hopefully optimized
#define SWAP_VALUES(FIRST, SECOND) \
    do{typeof(FIRST) temp = (FIRST); (FIRST) = (SECOND); (SECOND) = temp; }while(0)

// what is the real definition, will never know
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__) || defined(_MSC_VER)
#define WINDOWS_PLATFORM
#else
#define LINUX_PLATFORM
#endif

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]

static inline u8
address_is_between(u16 val, u16 start, u16 end) {
    return val >= start && val <= end;
}

GLenum
glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        unsigned char* error = NULL;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = (unsigned char*)"INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = (unsigned char*)"INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = (unsigned char*)"INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 error = (unsigned char*)"OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = (unsigned char*)"INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        //FATALERRORMESSAGE("GL ERROR %s \n", error);
        printf("GL ERROR %s (file %s, line %d)\n", error, file, line);
        exit(1);
    }
    return errorCode;
}

#define gl_check_error() glCheckError_(__FILE__, __LINE__)
#define GLCHECK(FUN) do{FUN; glCheckError_(__FILE__, __LINE__); } while(0)


static FILE* logfile;
#ifndef LOG
#define CHECKLOG if(!logfile){ printf("log file opened!\n"); logfile = fopen("logfile.txt", "w");}
#else
#define CHECKLOG
#endif

#endif // UTILSDEFS
