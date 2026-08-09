#include <list>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <string>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"

// ========== TOGGLES ==========
bool ticketToggle = true;
bool hasBanTimeToggle = false;
bool remoteAssetsToggle = true;

// ========== HOOK FUNCTIONS ==========
void (*old_AddOption)(void* instance, int role, int quantity, int price, bool isSelected, void* action);
void AddOption(void* instance, int role, int quantity, int price, bool isSelected, void* action) {
    if (ticketToggle) {
        quantity = 999;
    }
    old_AddOption(instance, role, quantity, price, isSelected, action);
}

bool (*old_HasBanTime)(void* instance);
bool HasBanTime(void* instance) {
    if (hasBanTimeToggle) {
        return false;
    }
    return old_HasBanTime(instance);
}

bool (*old_HasAssetsToDownload)(void* instance);
bool HasAssetsToDownload(void* instance) {
    if (remoteAssetsToggle) {
        return false;
    }
    return old_HasAssetsToDownload(instance);
}

bool (*old_IsVersionCached)(void* instance, void* url, void* assetBundleName, void* hash);
bool IsVersionCached(void* instance, void* url, void* assetBundleName, void* hash) {
    if (remoteAssetsToggle) {
        return true;
    }
    return old_IsVersionCached(instance, url, assetBundleName, hash);
}

bool (*old_IsVersionCached2nd)(void* instance, void* cachedBundle);
bool IsVersionCached2nd(void* instance, void* cachedBundle) {
    if (remoteAssetsToggle) {
        return true;
    }
    return old_IsVersionCached2nd(instance, cachedBundle);
}

// ========== MENU FEATURES ==========
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
            OBFUSCATE("Collapse_Fun Mods"),
            OBFUSCATE("303_CollapseAdd_True_Toggle_Role Tickets"),
            OBFUSCATE("304_CollapseAdd_Toggle_No Ban Time"),
            OBFUSCATE("305_CollapseAdd_True_Toggle_Block Assets Download"),
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

// Target main library name
#define targetLibName OBFUSCATE("libil2cpp.so")

// ========== HANDLE TOGGLES ==========
void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jstring featName, jint value, jlong Lvalue, jboolean boolean, jstring text) {

    if (!isLibraryLoaded(targetLibName)) {
        LOGI("Target library not loaded yet, skipping switch handler.");
        return;
    }

    switch (featNum) {
        case 303:
            ticketToggle = boolean;
            break;
        case 304:
            hasBanTimeToggle = boolean;
            break;
        case 305:
            remoteAssetsToggle = boolean;
            break;
        default:
            break;
    }
}

// ========== MEMORY PATCHING THREAD ==========
void hack_thread() {
    while (!isLibraryLoaded(targetLibName)) {
        sleep(1);
    }

#if defined(__aarch64__)
    HOOK(targetLibName, "0x2758374", AddOption, old_AddOption);
    HOOK(targetLibName, "0x2A90704", HasBanTime, old_HasBanTime);
    HOOK(targetLibName, "0x2FC7434", HasAssetsToDownload, old_HasAssetsToDownload);
    HOOK(targetLibName, "0x30E5E04", IsVersionCached, old_IsVersionCached);
    HOOK(targetLibName, "0x30E5D48", IsVersionCached2nd, old_IsVersionCached2nd);
#endif

    LOGI(OBFUSCATE("Done"));
}

// ========== LIBRARY ENTRY ==========
__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}
