#include <jni.h>
#include <android/log.h>

#if defined(DEBUG)
#include "memwatch.h"
#else
#include <malloc.h>
#endif

#define TAG "JLMedia"
#define logd(...)  ((void)__android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__))
#define logi(...)  ((void)__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#define logw(...)  ((void)__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))
#define loge(...)  ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))

#ifndef MEMWATCH
#error "You really, really don't want to run this without memwatch. Trust me."
#endif

#if !defined(MW_STDIO) && !defined(MEMWATCH_STDIO)
#error "Define MW_STDIO and try again, please."
#endif


JNIEXPORT jstring JNICALL
Java_com_hhbgk_memwatch_android_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject  thiz) {
    char* hello = "Hello, test native code with mwmwatch.";

    char *p;

    /* Collect stats on a line number basis */
    mwStatistics( 2 );

    /* Slows things down, but OK for this test prg */
    /* mwAutoCheck( 1 ); */

    TRACE("Hello world!\n");

    p = malloc(210);
    free(p);
    p = malloc(20);
    p = malloc(200);    /* causes unfreed error */
    p[-1] = 0;          /* causes underflow error */
    free(p);

    p = malloc(100);
    p[ -(int)(sizeof(long)*8) ] = -1; /* try to damage MW's heap chain */
    free( p ); /* should cause relink */

    mwSetAriFunc( mwAriHandler );
    ASSERT(1==2);

    mwLimit(1000000);
    mwNoMansLand( MW_NML_ALL );

    /* These may cause a general protection fault (segmentation fault) */
    /* They're here to help test the no-mans-land protection */
    if( mwIsSafeAddr(p+50000,1) ) {
        TRACE("Killing byte at %p\n", p+50000);
        *(p+50000) = 0;
    }
    if( mwIsSafeAddr(p+30000,1) ) {
        TRACE("Killing byte at %p\n", p+30000);
        *(p+30000) = 0;
    }
    if( mwIsSafeAddr(p+1000,1) ) {
        TRACE("Killing byte at %p\n", p+1000);
        *(p+1000) = 0;
    }
    if( mwIsSafeAddr(p-100,1) ) {
        TRACE("Killing byte at %p\n", p-100);
        *(p-100) = 0;
    }

    /* This may cause a GP fault as well, since MW data buffers  */
    /* have been damaged in the above killing spree */
    CHECK();

    p = malloc(12000);
    p[-5] = 1;
    p[-10] = 2;
    p[-15] = 3;
    p[-20] = 4;

    /* This may cause a GP fault since MW's buffer list may have */
    /* been damaged by above killing, and it will try to repair it. */
    free(p);

    p = realloc(p,10);	/* causes realloc: free'd from error */

    /* May cause GP since MW will inspect the memory to see if it owns it. */
    free( (void*)Java_com_hhbgk_memwatch_android_MainActivity_stringFromJNI );

    mwTerm();

    return (*env)->NewStringUTF(env, hello);
}
