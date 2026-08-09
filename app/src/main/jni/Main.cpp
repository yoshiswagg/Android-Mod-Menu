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

// ========== TARGET LIBRARY ==========
#define targetLibName OBFUSCATE("libil2cpp.so")

// ========== TOGGLES ==========
bool ticketToggle = true;
bool hasBanTimeToggle = false;
bool remoteAssetsToggle = true;
bool showRealNamesToggle = false;
bool autoCompleteTasksToggle = false;
bool easyReportToggle = false;
float visionMultiplierValue = 1.0f;

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

// Show real names - hook get_VisualName to return NetworkPlayerName
void* (*old_get_VisualName)(void* instance);
void* get_VisualName(void* instance) {
    if (showRealNamesToggle) {
        auto get_NetworkPlayerName = (void* (*)(void*))getAbsoluteAddress(targetLibName, OBFUSCATE("0x116B4C0"));
        return get_NetworkPlayerName(instance);
    }
    return old_get_VisualName(instance);
}

// Auto Complete Tasks - hook StartMinigame to call OnMinigameCompleted
void (*old_StartMinigame)(void* instance, int minigameType, int roomType);
void StartMinigame(void* instance, int minigameType, int roomType) {
    if (autoCompleteTasksToggle) {
        auto OnMinigameCompleted = (void (*)(void*))getAbsoluteAddress(targetLibName, OBFUSCATE("0x32F0B70"));
        OnMinigameCompleted(instance);
        return;
    }
    old_StartMinigame(instance, minigameType, roomType);
}

// Easy Report - hook ReportView constructor to set hasEnoughCharactersOnMoreInfoToSendReport = true
void (*old_ReportViewCtor)(void* instance);
void ReportViewCtor(void* instance) {
    old_ReportViewCtor(instance);
    if (easyReportToggle) {
        bool* fieldPtr = (bool*)((uintptr_t)instance + 0x71);
        *fieldPtr = true;
    }
}

// Vision Multiplier - hook get_VisionMultiplier to multiply by custom value
float (*old_get_VisionMultiplier)(void* instance);
float get_VisionMultiplier(void* instance) {
    return old_get_VisionMultiplier(instance) * visionMultiplierValue;
}

// Speedhack - set timeScale
void (*set_timeScale)(float speed);
void setGameSpeed(float speed) {
    if (set_timeScale) {
        set_timeScale(speed);
    }
}

// ========== MENU FEATURES ==========
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
            OBFUSCATE("Collapse_Fun Mods"),
            OBFUSCATE("303_CollapseAdd_True_Toggle_Role Tickets"),
            OBFUSCATE("304_CollapseAdd_Toggle_No Ban Time"),
            OBFUSCATE("305_CollapseAdd_True_Toggle_Block Assets Download"),
            OBFUSCATE("306_CollapseAdd_Toggle_Show Real Names"),
            OBFUSCATE("307_CollapseAdd_Toggle_Auto Complete Tasks"),
            OBFUSCATE("308_CollapseAdd_Toggle_Easy Report"),
            OBFUSCATE("309_CollapseAdd_SeekBar_Vision Multiplier_1_15"),
            OBFUSCATE("310_CollapseAdd_SeekBar_Speedhack_1_10"),
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

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
        case 306:
            showRealNamesToggle = boolean;
            break;
        case 307:
            autoCompleteTasksToggle = boolean;
            break;
        case 308:
            easyReportToggle = boolean;
            break;
        case 309:
            visionMultiplierValue = (float)value;
            break;
        case 310:
            setGameSpeed((float)value);
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
    // Get timeScale setter from UnityEngine.CoreModule
    set_timeScale = (void (*)(float))getAbsoluteAddress(targetLibName, OBFUSCATE("0x306EF4"));

    HOOK(targetLibName, "0x2758374", AddOption, old_AddOption);
    HOOK(targetLibName, "0x2A90704", HasBanTime, old_HasBanTime);
    HOOK(targetLibName, "0x2FC7434", HasAssetsToDownload, old_HasAssetsToDownload);
    HOOK(targetLibName, "0x30E5E04", IsVersionCached, old_IsVersionCached);
    HOOK(targetLibName, "0x30E5D48", IsVersionCached2nd, old_IsVersionCached2nd);
    HOOK(targetLibName, "0x1164F4C", get_VisualName, old_get_VisualName);
    HOOK(targetLibName, "0x32F0AC0", StartMinigame, old_StartMinigame);
    HOOK(targetLibName, "0x275A248", ReportViewCtor, old_ReportViewCtor);
    HOOK(targetLibName, "0x1161AF0", get_VisionMultiplier, old_get_VisionMultiplier);
#endif

    LOGI(OBFUSCATE("Done"));
}

// ========== LIBRARY ENTRY ==========
__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}
