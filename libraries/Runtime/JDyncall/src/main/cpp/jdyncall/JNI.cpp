#include "com_nativelibs4java_runtime_JNI.h"

#include "dyncallback/dyncall_callback.h"
#include "dynload/dynload.h"
#include "RawNativeForwardCallback.h"

#include "jdyncall.hpp"
#include <string.h>
#include "Exceptions.h"

using namespace std;

#define JNI_SIZEOF(type, escType) \
jint JNICALL Java_com_nativelibs4java_runtime_JNI_sizeOf_1 ## escType(JNIEnv *, jclass) { return sizeof(type); }

#define JNI_SIZEOF_t(type) JNI_SIZEOF(type ## _t, type ## _1t)

JNI_SIZEOF_t(size)
JNI_SIZEOF_t(wchar)
JNI_SIZEOF_t(ptrdiff)

void JNICALL Java_com_nativelibs4java_runtime_JNI_init(JNIEnv *env, jclass)
{
	//DefineCommonClassesAndMethods(env);
}

jlong JNICALL Java_com_nativelibs4java_runtime_JNI_getDirectBufferAddress(JNIEnv *env, jobject jthis, jobject buffer) {
	BEGIN_TRY();
	return !buffer ? 0 : (jlong)env->GetDirectBufferAddress(buffer);
	END_TRY(env);
}
jlong JNICALL Java_com_nativelibs4java_runtime_JNI_getDirectBufferCapacity(JNIEnv *env, jobject jthis, jobject buffer) {
	BEGIN_TRY();
	return !buffer ? 0 : env->GetDirectBufferCapacity(buffer);
	END_TRY(env);
}

jlong JNICALL Java_com_nativelibs4java_runtime_JNI_getObjectPointer(JNIEnv *, jclass, jobject object)
{
	return (jlong)object;
}
 
jlong JNICALL Java_com_nativelibs4java_runtime_JNI_loadLibrary(JNIEnv *env, jclass, jstring pathStr)
{
	const char* path = env->GetStringUTFChars(pathStr, NULL);
	jlong ret = (jlong)dlLoadLibrary(path);
	env->ReleaseStringUTFChars(pathStr, path);
	return ret;
}

void JNICALL Java_com_nativelibs4java_runtime_JNI_freeLibrary(JNIEnv *, jclass, jlong libHandle)
{
	dlFreeLibrary((DLLib*)libHandle);
}

jlong JNICALL Java_com_nativelibs4java_runtime_JNI_findSymbolInLibrary(JNIEnv *env, jclass, jlong libHandle, jstring nameStr)
{
	const char* name = env->GetStringUTFChars(nameStr, NULL);
	jlong ret = (jlong)dlFindSymbol((DLLib*)libHandle, name);
	env->ReleaseStringUTFChars(nameStr, name);
	return ret;
}

jobject JNICALL Java_com_nativelibs4java_runtime_JNI_newDirectByteBuffer(JNIEnv *env, jobject jthis, jlong peer, jlong length) {
	BEGIN_TRY();
	return env->NewDirectByteBuffer((void*)peer, length);
	END_TRY(env);
}

JNIEXPORT jlong JNICALL Java_com_nativelibs4java_runtime_JNI_getMaxDirectMappingArgCount() {
#ifdef _WIN64
	return 4;
#else	
#ifdef _WIN64
	return 65000;
#else
	return -1;
#endif
#endif
}

JNIEXPORT jlong JNICALL Java_com_nativelibs4java_runtime_JNI_createCallback(
	JNIEnv *env, 
	jclass,
	jclass declaringClass,
	jstring methodName,
	jint callMode,
	jlong forwardedPointer, 
	jboolean direct, 
	jstring javaSignature, 
	jstring dcSignature,
	jint nParams,
	jint returnValueType, 
	jintArray paramsValueTypes
) {
	if (!forwardedPointer)
		return NULL;
	
	MethodCallInfo *info = (MethodCallInfo*)malloc(sizeof(MethodCallInfo));
	memset(info, 0, sizeof(MethodCallInfo));
	
	info->fDCMode = callMode;
	info->fReturnType = (ValueType)returnValueType;
	info->nParams = nParams;
	if (nParams) {
		info->fParamTypes = (ValueType*)malloc(nParams * sizeof(jint));	
		env->GetIntArrayRegion(paramsValueTypes, 0, nParams, (jint*)info->fParamTypes);
	}
	
	JNINativeMethod meth;
	meth.fnPtr = NULL;
	if (direct)
		info->fCallback = (DCCallback*)dcRawCallAdapterSkipTwoArgs((void (*)())forwardedPointer);
	if (!meth.fnPtr) {
		const char* ds = env->GetStringUTFChars(dcSignature, NULL);
		info->fCallback = dcNewCallback(ds, JavaToNativeCallHandler, info);
		env->ReleaseStringUTFChars(dcSignature, ds);
	}
	meth.fnPtr = info->fCallback;
	meth.name = (char*)env->GetStringUTFChars(methodName, NULL);
	meth.signature = (char*)env->GetStringUTFChars(javaSignature, NULL);
	env->RegisterNatives(declaringClass, &meth, 1);
	
	env->ReleaseStringUTFChars(methodName, meth.name);
	env->ReleaseStringUTFChars(javaSignature, meth.signature);
	
	return (jlong)info;
}

JNIEXPORT void JNICALL Java_com_nativelibs4java_runtime_JNI_freeCallbacks(JNIEnv *env, jclass, jlong nativeCallback)
{
	MethodCallInfo* info = (MethodCallInfo*)nativeCallback;
	if (info->nParams)
		free(info->fParamTypes);
	
	dcFreeCallback((DCCallback*)info->fCallback);
	free(info);
}


#include "PrimDefs_int.h"
#include "JNI_prim.h"

#include "PrimDefs_long.h"
#include "JNI_prim.h"

#include "PrimDefs_short.h"
#include "JNI_prim.h"

#include "PrimDefs_byte.h"
#include "JNI_prim.h"

#include "PrimDefs_char.h"
#include "JNI_prim.h"

#include "PrimDefs_boolean.h"
#include "JNI_prim.h"

#include "PrimDefs_float.h"
#include "JNI_prim.h"

#include "PrimDefs_double.h"
#include "JNI_prim.h"
