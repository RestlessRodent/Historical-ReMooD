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

/**
 * Initializes the Java interface.
 *
 * @return {@code true} on success.
 * @since 2016/05/05
 */
bool_t J_Init();


/** Mirrors of JNI library (removes env). */
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

#endif /* REMOOD_J_H__ */

