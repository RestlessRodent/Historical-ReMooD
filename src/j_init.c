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
	char* jstr;
	int rv;
	
	/* Setup arguments used to initialize the virtual machine with. */
	memset(&args, 0, sizeof(args));
	memset(&options, 0, sizeof(options));
	
	/* Java 7 is desired. */
	args.version = JNI_VERSION_1_6;
	args.nOptions = 1;
	args.options = &options;
	
	/* Setup options. */
    options.optionString = "-Djava.class.path=remood-core.jar";
    
    /* Setup the virtual machine. */
    rv = JNI_CreateJavaVM(&g_JVM, (void**)&g_Env, &args);
    if (rv < 0 || g_Env == NULL)
    	return false;
    
    // Print version informaion
    versionclass = (*g_Env)->FindClass(g_Env,
    	"org/remood/remood/core/Version");
    rjver = (*g_Env)->GetStaticFieldID(g_Env, versionclass,
    	"FULL_DISPLAY_STRING", "Ljava/lang/String;");
    jvx = (*g_Env)->GetStaticObjectField(g_Env, versionclass, rjver);
    jstr = (*g_Env)->GetStringUTFChars(g_Env, (jstring)jvx, 0);
    printf("ReMooD Java Version: %s\n", jstr);
    (*g_Env)->ReleaseStringUTFChars(g_Env, (jstring)jvx, jstr);
	
	// Ok
	return true;
}

