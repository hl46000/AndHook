#include <jni.h>
#include <android/log.h>
#include <alloca.h>
#include "include/AndHook.h"
#define MSHook(X) MSHookFunction(reinterpret_cast<void *>(X), reinterpret_cast<void *>(my_##X), reinterpret_cast<void **>(&sys_##X));

static intptr_t sys_getGTalkDeviceId;

static jstring JNICALL my_getGTalkDeviceId(JNIEnv *env, jclass obj, jlong j)
{
	jmethodID jm = MSGetMethodID(env, sys_getGTalkDeviceId, alloca(JNI_METHOD_SIZE));
	jstring   js = reinterpret_cast<jstring>(env->CallStaticObjectMethod(obj, jm, j));
	if (js != NULL) {
		__android_log_print(ANDROID_LOG_INFO, __FUNCTION__, "%s", env->GetStringUTFChars(js, NULL));
	} //if

	return env->NewStringUTF("faked_GTalkDeviceId");
}

static int(*sys_access)(const char *pathname, int mode);

static int my_access(const char *pathname, int mode)
{
	if (strstr(pathname, "/system/bin/su") != NULL ||
	    strstr(pathname, "/system/xbin/su") != NULL) {
		return -1;
	} //if

	__android_log_print(ANDROID_LOG_INFO, __FUNCTION__, "access %s, %d", pathname, mode);
	return sys_access(pathname, mode);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    __android_log_print(ANDROID_LOG_INFO, __FUNCTION__, "starting jni hook...");
    jclass clazz = env->FindClass("android/provider/Settings");
    MSJavaHookMethod(env, clazz, "getGTalkDeviceId", "(J)Ljava/lang/String;",
                     reinterpret_cast<void *>(my_getGTalkDeviceId), &sys_getGTalkDeviceId);
    /* the following code is equivalent to call MSJavaHookMethod/MSJavaHookMethodV
    jmethodID md = env->GetStaticMethodID(clazz,
                                          "getGTalkDeviceId", "(J)Ljava/lang/String;");
    void **jni_entrypoint = NULL;
    sys_getGTalkDeviceId = MSForceNativeMethod(md, &jni_entrypoint, true);
    if (jni_entrypoint != NULL) *jni_entrypoint = reinterpret_cast<void *>(my_getGTalkDeviceId);
    */
    __android_log_print(ANDROID_LOG_INFO, __FUNCTION__, "jni hook done.");

    __android_log_print(ANDROID_LOG_INFO, __FUNCTION__, "starting native hook...");
    // MSSuspendAllThreads();
    MSHook(access);
    // MSResumeAllThreads();
    __android_log_print(ANDROID_LOG_INFO, __FUNCTION__, "native hook done.");

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
}