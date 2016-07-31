// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

#include <jni.h>

#include <string.h>

#include "j.h"

/** The global virtual machine. */
JavaVM* g_JVM = NULL;

/** The global environment. */
JNIEnv* g_Env = NULL;

#if defined(_WIN32) || defined(__WIN32__)
	#define PATH_SEP ";"
#else
	#define PATH_SEP ":"
#endif

/** The Main class. */
jclass g_MainClass = NULL;
jobject g_MainObject = NULL;

void J_VideoQuickInit(void);

/**
 * {@inheritDoc}
 * @since 2016/05/05
 */
bool_t J_Init()
{
	JavaVMInitArgs args;
	JavaVMOption options;
	jclass versionclass;
	jfieldID rjver;
	jobject jvx;
	const char* jstr;
	int rv;
	
	/* Setup arguments used to initialize the virtual machine with. */
	memset(&args, 0, sizeof(args));
	memset(&options, 0, sizeof(options));
	
	/* Java 7 is desired. */
	args.version = JNI_VERSION_1_6;
	args.nOptions = 1;
	args.options = &options;
	
	/* Setup options. */
    options.optionString = "-Djava.class.path=remood-core.jar" PATH_SEP
    	"remood-javase.jar" PATH_SEP "remood-sdl.jar";
    
    /* Setup the virtual machine. */
    rv = JNI_CreateJavaVM(&g_JVM, (void**)&g_Env, &args);
    if (rv < 0 || g_Env == NULL)
    	return false;
    
    // Print version informaion
    versionclass = J_FindClass("org/remood/remood/core/Version");
    rjver = J_GetStaticFieldID(versionclass,
    	"FULL_DISPLAY_STRING", "Ljava/lang/String;");
    jvx = J_GetStaticObjectField(versionclass, rjver);
    jstr = J_GetStringUTFChars((jstring)jvx, 0);
    printf("ReMooD Java Version: %s\n", jstr);
    J_ReleaseStringUTFChars((jstring)jvx, jstr);
    
	// Ok
	return true;
}

extern int myargc;
extern char** myargv;

static jobjectArray __mainArguments()
{
	jobjectArray cargs;
	int i;
	
	// Allocate array
	cargs = J_NewObjectArray(myargc - 1, J_FindClass("java/lang/String"), NULL);
	
	// Set arguments
	for (i = 0; i < myargc - 1; i++)
		J_SetObjectArrayElement(cargs, i, J_NewStringUTF(myargv[i + 1]));
	
	return cargs;
}

void J_AltMain()
{
	jmethodID mainmethod;
	jmethodID mainloop;
	
	// Find main class
	g_MainClass = J_FindClass("org/remood/remood/core/Main");
	mainmethod = J_GetMethodID(g_MainClass, "<init>",
		"([Ljava/lang/String;)V");
	
	// Construct new main class
	g_MainObject = J_NewObject(g_MainClass, mainmethod, __mainArguments());
	
	// If pure Java, just enter the loop
	if (M_CheckParm("-java"))
	{
		mainloop = J_GetMethodID(g_MainClass, "loop", "()V");
		
		// Enter the main loop
		J_CallVoidMethod(g_MainObject, mainloop);
		
		// Stop
		I_Quit();
	}
	
	// Initialize some later things
	J_VideoQuickInit();
}

/** Exceptions. */

jthrowable J_ExceptionOccurred()
{
	return (*g_Env)->ExceptionOccurred(g_Env);
}

void J_ExceptionDescribe()
{
	return (*g_Env)->ExceptionDescribe(g_Env);
}

void __checkException()
{
	// See if one was thrown
	jthrowable x = J_ExceptionOccurred();
	
	// One was
	if (x != NULL)
	{
		// Describe it
		J_ExceptionDescribe();
		
		// Then explode
		I_Error("Exception thrown from Java code.");
	}
}

/** JNI Mirrors. */

jclass J_FindClass(const char *name)
{
	jclass rv = (*g_Env)->FindClass(g_Env, name);
	
	__checkException();
	
	return rv;
}

jmethodID J_GetMethodID(jclass clazz, const char *name, const char *sig)
{
	jmethodID rv = (*g_Env)->GetMethodID(g_Env, clazz, name, sig);
	
	__checkException();
	
	return rv;
}

jobject J_NewObject(jclass clazz, jmethodID methodID, ...)
{
	jobject rv;
	va_list ap;
	
	va_start(ap, methodID);
	rv = (*g_Env)->NewObjectV(g_Env, clazz, methodID, ap);
	va_end(ap);
	
	__checkException();
	
	return rv;
}

jobject J_GetStaticObjectField(jclass clazz, jfieldID fieldID)
{
	jobject rv = (*g_Env)->GetStaticObjectField(g_Env, clazz, fieldID);
	
	__checkException();
	
	return rv;
}

const char* J_GetStringUTFChars(jstring str, jboolean *isCopy)
{
	const char* rv = (*g_Env)->GetStringUTFChars(g_Env, str, isCopy);
	
	__checkException();
	
	return rv;
}

void J_ReleaseStringUTFChars(jstring str, const char* chars)
{
	(*g_Env)->ReleaseStringUTFChars(g_Env, str, chars);
	
	__checkException();
}

jint J_CallIntMethod(jobject obj, jmethodID methodID, ...)
{
	jint rv;
	va_list ap;
	
	va_start(ap, methodID);
	rv = (*g_Env)->CallIntMethodV(g_Env, obj, methodID, ap);
	va_end(ap);
	
	__checkException();
	
	return rv;
}

jobject J_CallObjectMethod(jobject obj, jmethodID methodID, ...)
{
	jobject rv;
	va_list ap;
	
	va_start(ap, methodID);
	rv = (*g_Env)->CallObjectMethodV(g_Env, obj, methodID, ap);
	va_end(ap);
	
	__checkException();
	
	return rv;
}

jfieldID J_GetStaticFieldID(jclass clazz, const char *name, const char *sig)
{
	jfieldID rv = (*g_Env)->GetStaticFieldID(g_Env, clazz, name, sig);
	
	__checkException();
	
	return rv;
}

jstring J_NewStringUTF(const char* str)
{
	jstring rv =  (*g_Env)->NewStringUTF(g_Env, str);
	
	__checkException();
	
	return rv;
}

jmethodID J_GetStaticMethodID(jclass clazz, const char *name, const char *sig)
{
	jmethodID rv = (*g_Env)->GetStaticMethodID(g_Env, clazz, name, sig);
	
	__checkException();
	
	return rv;
}

void J_CallStaticVoidMethod(jclass cls, jmethodID methodID, ...)
{
	va_list ap;
	
	va_start(ap, methodID);
	(*g_Env)->CallStaticVoidMethodV(g_Env, cls, methodID, ap);
	va_end(ap);
	
	__checkException();
}

jlong J_CallStaticLongMethod(jclass cls, jmethodID methodID, ...)
{
	va_list ap;
	jlong rv;
	
	va_start(ap, methodID);
	rv = (*g_Env)->CallStaticLongMethodV(g_Env, cls, methodID, ap);
	va_end(ap);
	
	__checkException();
	
	return rv;
}

jobjectArray J_NewObjectArray(jsize length, jclass elementclass,
	jobject initialelement)
{
	jobjectArray rv = (*g_Env)->NewObjectArray(g_Env, length, elementclass,
		initialelement);
	
	__checkException();
	
	return rv;
}

void J_SetObjectArrayElement(jobjectArray array, jsize index, jobject value)
{
	(*g_Env)->SetObjectArrayElement(g_Env, array, index, value);
	
	__checkException();
}

jint J_CallStaticIntMethod(jclass cls, jmethodID methodID, ...)
{
	va_list ap;
	jint rv;
	
	va_start(ap, methodID);
	rv = (*g_Env)->CallStaticIntMethodV(g_Env, cls, methodID, ap);
	va_end(ap);
	
	__checkException();
	
	return rv;
}

jsize J_GetArrayLength(jarray array)
{
	jsize rv = (*g_Env)->GetArrayLength(g_Env, array);
	
	__checkException();
	
	return rv;
}

void J_GetIntArrayRegion(jintArray array, jsize start, jsize len, jint *buf)
{
	(*g_Env)->GetIntArrayRegion(g_Env, array, start, len, buf);
	
	__checkException();
}

jboolean J_CallBooleanMethod(jobject obj, jmethodID methodID, ...)
{
	jboolean rv;
	va_list ap;
	
	va_start(ap, methodID);
	rv = (*g_Env)->CallBooleanMethodV(g_Env, obj, methodID, ap);
	va_end(ap);
	
	__checkException();
	
	return rv;
}

jint *J_GetIntArrayElements(jintArray array, jboolean *isCopy)
{
	jint* rv = (*g_Env)->GetIntArrayElements(g_Env, array, isCopy);
	
	__checkException();	
	
	return rv;
}

void J_ReleaseIntArrayElements(jintArray array, jint *elems, jint mode)
{
	(*g_Env)->ReleaseIntArrayElements(g_Env, array, elems, mode);
	
	__checkException();
}

void J_CallVoidMethod(jobject obj, jmethodID methodID, ...)
{
	va_list ap;
	
	va_start(ap, methodID);
	(*g_Env)->CallVoidMethodV(g_Env, obj, methodID, ap);
	va_end(ap);
	
	__checkException();
}



