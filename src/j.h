// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

#ifndef REMOOD_J_H__
#define REMOOD_J_H__

#include <jni.h>

#include "doomtype.h"

/** The global virtual machine. */
extern JavaVM* g_JVM;

/** The global environment. */
extern JNIEnv* g_Env;

/** Main class. */
extern jclass g_MainClass;
extern jobject g_MainObject;

/**
 * Initializes the Java interface.
 *
 * @return {@code true} on success.
 * @since 2016/05/05
 */
bool_t J_Init();

void J_AltMain();

/** Mirrors of JNI library (removes env). */
jthrowable J_ExceptionOccurred();
void J_ExceptionDescribe();
jclass J_FindClass(const char *name);
jmethodID J_GetMethodID(jclass clazz, const char *name, const char *sig);
jobject J_NewObject(jclass clazz, jmethodID methodID, ...);
jobject J_GetStaticObjectField(jclass clazz, jfieldID fieldID);
const char* J_GetStringUTFChars(jstring str, jboolean *isCopy);
void J_ReleaseStringUTFChars(jstring str, const char* chars);
jint J_CallIntMethod(jobject obj, jmethodID methodID, ...);
jobject J_CallObjectMethod(jobject obj, jmethodID methodID, ...);
jfieldID J_GetStaticFieldID(jclass clazz, const char *name, const char *sig);
jstring J_NewStringUTF(const char* str);
jmethodID J_GetStaticMethodID(jclass clazz, const char *name, const char *sig);
void J_CallStaticVoidMethod(jclass cls, jmethodID methodID, ...);
jlong J_CallStaticLongMethod(jclass cls, jmethodID methodID, ...);
jobjectArray J_NewObjectArray(jsize length, jclass elementclass,
	jobject initialelement);
void J_SetObjectArrayElement(jobjectArray array, jsize index, jobject value);
jint J_CallStaticIntMethod(jclass cls, jmethodID methodID, ...);
jsize J_GetArrayLength(jarray array);
void J_GetIntArrayRegion(jintArray array, jsize start, jsize len, jint *buf);
jboolean J_CallBooleanMethod(jobject obj, jmethodID methodID, ...);
jint *J_GetIntArrayElements(jintArray array, jboolean *isCopy);
void J_ReleaseIntArrayElements(jintArray array, jint *elems, jint mode);
void J_CallVoidMethod(jobject obj, jmethodID methodID, ...);


#endif /* REMOOD_J_H__ */

