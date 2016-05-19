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
    	"remood-sdl.jar";
    
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
    
    // Initialize some things
	
	// Ok
	return true;
}

/** JNI Mirrors. */

jclass J_FindClass(const char *name)
{
	return (*g_Env)->FindClass(g_Env, name);
}

jmethodID J_GetMethodID(jclass clazz, const char *name, const char *sig)
{
	return (*g_Env)->GetMethodID(g_Env, clazz, name, sig);
}

jobject J_NewObject(jclass clazz, jmethodID methodID, ...)
{
	jobject rv;
	va_list ap;
	
	va_start(ap, methodID);
	rv = (*g_Env)->NewObjectV(g_Env, clazz, methodID, ap);
	va_end(ap);
	
	return rv;
}

jobject J_GetStaticObjectField(jclass clazz, jfieldID fieldID)
{
	return (*g_Env)->GetStaticObjectField(g_Env, clazz, fieldID);
}

const char* J_GetStringUTFChars(jstring str, jboolean *isCopy)
{
	return (*g_Env)->GetStringUTFChars(g_Env, str, isCopy);
}

void J_ReleaseStringUTFChars(jstring str, const char* chars)
{
	(*g_Env)->ReleaseStringUTFChars(g_Env, str, chars);
}

jint J_CallIntMethod(jobject obj, jmethodID methodID, ...)
{
	jint rv;
	va_list ap;
	
	va_start(ap, methodID);
	rv = (*g_Env)->CallIntMethodV(g_Env, obj, methodID, ap);
	va_end(ap);
	
	return rv;
}

jobject J_CallObjectMethod(jobject obj, jmethodID methodID, ...)
{
	jobject rv;
	va_list ap;
	
	va_start(ap, methodID);
	rv = (*g_Env)->CallObjectMethodV(g_Env, obj, methodID, ap);
	va_end(ap);
	
	return rv;
}

jfieldID J_GetStaticFieldID(jclass clazz, const char *name, const char *sig)
{
	return (*g_Env)->GetStaticFieldID(g_Env, clazz, name, sig);
}

jstring J_NewStringUTF(const char* str)
{
	return (*g_Env)->NewStringUTF(g_Env, str);
}

jmethodID J_GetStaticMethodID(jclass clazz, const char *name, const char *sig)
{
	return (*g_Env)->GetStaticMethodID(g_Env, clazz, name, sig);
}

void J_CallStaticVoidMethod(jclass cls, jmethodID methodID, ...)
{
	va_list ap;
	
	va_start(ap, methodID);
	(*g_Env)->CallStaticVoidMethodV(g_Env, cls, methodID, ap);
	va_end(ap);
}

jlong J_CallStaticLongMethod(jclass cls, jmethodID methodID, ...)
{
	va_list ap;
	jlong rv;
	
	va_start(ap, methodID);
	rv = (*g_Env)->CallStaticLongMethodV(g_Env, cls, methodID, ap);
	va_end(ap);
	
	return rv;
}

