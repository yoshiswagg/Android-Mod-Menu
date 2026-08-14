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
bool fpsUnlockToggle = false;
bool crashBellToggle = false;
bool voteGhostToggle = false;
bool alwaysLeaveToggle = false;
bool instantBellToggle = false;
bool versionBypassToggle = false;
bool hasChangedFrameRate = false;
float visionMultiplierValue = 1.0f;

// ========== FUNCTION POINTERS ==========
void (*set_timeScale)(float speed);
void (*CmdCallMeeting)(void* instance, int reported);
void (*CmdOpenSecurityCameraConsole)(void* instance);
void (*CmdOpenVitalsConsole)(void* instance);
void* (*get_LocalPlayerController)();
void (*set_targetFrameRate)(int fps);
void (*CmdUseTicket)(void* instance, int roleTicket);
void (*CmdVote)(void* instance, int v);

// ========== HELPER FUNCTIONS ==========
void* GetPlayerController() {
    if (get_LocalPlayerController) {
        return get_LocalPlayerController();
    }
    return nullptr;
}

void CallMeeting() {
    void* playerController = GetPlayerController();
    if (playerController && CmdCallMeeting) {
        if (crashBellToggle) {
            CmdCallMeeting(playerController, 69);
        } else {
            CmdCallMeeting(playerController, 0);
        }
    }
}

void BodyReport() {
    void* playerController = GetPlayerController();
    if (playerController && CmdCallMeeting) {
        if (crashBellToggle) {
            CmdCallMeeting(playerController, 69);
        } else {
            int playerID = *(int*)((uintptr_t)playerController + 0x98);
            CmdCallMeeting(playerController, playerID);
        }
    }
}

void OpenCamera() {
    void* playerController = GetPlayerController();
    if (playerController && CmdOpenSecurityCameraConsole) {
        CmdOpenSecurityCameraConsole(playerController);
    }
}

void OpenVitals() {
    void* playerController = GetPlayerController();
    if (playerController && CmdOpenVitalsConsole) {
        CmdOpenVitalsConsole(playerController);
    }
}

void SetFPS(bool toggle) {
    hasChangedFrameRate = hasChangedFrameRate || toggle;
    if (!hasChangedFrameRate) {
        return;
    }
    if (!set_targetFrameRate) {
        return;
    }
    if (toggle) {
        set_targetFrameRate(120);
    } else {
        set_targetFrameRate(30);
    }
}

void SetRoleGuest() {
    void* playerController = GetPlayerController();
    if (playerController && CmdUseTicket) {
        CmdUseTicket(playerController, 1);
    }
}

void SetRoleKiller() {
    void* playerController = GetPlayerController();
    if (playerController && CmdUseTicket) {
        CmdUseTicket(playerController, 2);
    }
}

// ========== HOOK CmdCallMeeting ==========
void (*old_CmdCallMeeting)(void* instance, int reported);
void CmdCallMeeting_Hook(void* instance, int reported) {
    if (crashBellToggle) {
        old_CmdCallMeeting(instance, 69);
    } else {
        old_CmdCallMeeting(instance, reported);
    }
}

// ========== HOOK VotePlayer IsDead ==========
bool (*old_VotePlayer_IsDead)(void* instance);
bool VotePlayer_IsDead(void* instance) {
    if (voteGhostToggle) {
        return false;
    }
    return old_VotePlayer_IsDead(instance);
}

// ========== HOOK Always Leave ==========
void (*old_ToggleLeaveMatchButton)(void* instance, bool isActive);
void ToggleLeaveMatchButton(void* instance, bool isActive) {
    if (alwaysLeaveToggle) {
        old_ToggleLeaveMatchButton(instance, true);
        return;
    }
    old_ToggleLeaveMatchButton(instance, isActive);
}

// ========== HOOK CmdVote (Instant Bell) ==========
void (*old_CmdVote)(void* instance, int v);
void CmdVote_Hook(void* instance, int v) {
    if (instantBellToggle) {
        for (int i = 0; i < 100; i++) {
            old_CmdVote(instance, 0);
        }
    }
    old_CmdVote(instance, v);
}

// ========== HOOK ClientVersionComparison (Always Return Same) ==========
int (*old_ClientVersionComparison_Get)(void* instance);
int ClientVersionComparison_Get_Hook(void* instance) {
    if (versionBypassToggle) {
        return 0; // ClientVersionComparison.Same
    }
    return old_ClientVersionComparison_Get(instance);
}

void (*old_ClientVersionComparison_Set)(void* instance, int value);
void ClientVersionComparison_Set_Hook(void* instance, int value) {
    if (versionBypassToggle) {
        // Force set to Same (0) always
        old_ClientVersionComparison_Set(instance, 0);
        return;
    }
    old_ClientVersionComparison_Set(instance, value);
}

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
            OBFUSCATE("311_CollapseAdd_Button_Call Meeting"),
            OBFUSCATE("312_CollapseAdd_Button_Body Report"),
            OBFUSCATE("313_CollapseAdd_Button_Open Camera"),
            OBFUSCATE("314_CollapseAdd_Button_Open Vitals"),
            OBFUSCATE("315_CollapseAdd_Toggle_Unlock FPS"),
            OBFUSCATE("316_CollapseAdd_Button_Set Role Guest"),
            OBFUSCATE("317_CollapseAdd_Button_Set Role Killer"),
            OBFUSCATE("318_CollapseAdd_Toggle_Crash Bell"),
            OBFUSCATE("319_CollapseAdd_Toggle_Vote Ghost"),
            OBFUSCATE("321_CollapseAdd_Toggle_Always Leave"),
            OBFUSCATE("322_CollapseAdd_Toggle_Instant Bell"),
            OBFUSCATE("323_CollapseAdd_Toggle_Version Bypass"),
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
            setGameSpeed(value);
            break;
        case 311:
            CallMeeting();
            break;
        case 312:
            BodyReport();
            break;
        case 313:
            OpenCamera();
            break;
        case 314:
            OpenVitals();
            break;
        case 315:
            SetFPS(boolean);
            break;
        case 316:
            SetRoleGuest();
            break;
        case 317:
            SetRoleKiller();
            break;
        case 318:
            crashBellToggle = boolean;
            break;
        case 319:
            voteGhostToggle = boolean;
            break;
        case 321:
            alwaysLeaveToggle = boolean;
            break;
        case 322:
            instantBellToggle = boolean;
            break;
        case 323:
            versionBypassToggle = boolean;
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

    // Get function addresses
    set_timeScale = (void (*)(float))getAbsoluteAddress(targetLibName, OBFUSCATE("0x30E6FE4"));
    CmdCallMeeting = (void (*)(void*, int))getAbsoluteAddress(targetLibName, OBFUSCATE("0x1166CAC"));
    CmdOpenSecurityCameraConsole = (void (*)(void*))getAbsoluteAddress(targetLibName, OBFUSCATE("0x116799C"));
    CmdOpenVitalsConsole = (void (*)(void*))getAbsoluteAddress(targetLibName, OBFUSCATE("0x1167E78"));
    get_LocalPlayerController = (void* (*)())getAbsoluteAddress(targetLibName, OBFUSCATE("0xFC7668"));
    set_targetFrameRate = (void (*)(int))getAbsoluteAddress(targetLibName, OBFUSCATE("0x2C567A4"));
    CmdUseTicket = (void (*)(void*, int))getAbsoluteAddress(targetLibName, OBFUSCATE("0x1165FAC"));
    CmdVote = (void (*)(void*, int))getAbsoluteAddress(targetLibName, OBFUSCATE("0x1166DD8"));

    HOOK(targetLibName, "0x2758374", AddOption, old_AddOption);
    HOOK(targetLibName, "0x2A90704", HasBanTime, old_HasBanTime);
    HOOK(targetLibName, "0x2FC7434", HasAssetsToDownload, old_HasAssetsToDownload);
    HOOK(targetLibName, "0x30E5E04", IsVersionCached, old_IsVersionCached);
    HOOK(targetLibName, "0x30E5D48", IsVersionCached2nd, old_IsVersionCached2nd);
    HOOK(targetLibName, "0x1164F4C", get_VisualName, old_get_VisualName);
    HOOK(targetLibName, "0x32F0AC0", StartMinigame, old_StartMinigame);
    HOOK(targetLibName, "0x275A248", ReportViewCtor, old_ReportViewCtor);
    HOOK(targetLibName, "0x1161AF0", get_VisionMultiplier, old_get_VisionMultiplier);
    HOOK(targetLibName, "0x1166CAC", CmdCallMeeting_Hook, old_CmdCallMeeting);
    HOOK(targetLibName, "0x346D7C4", VotePlayer_IsDead, old_VotePlayer_IsDead);
    HOOK(targetLibName, "0x23CC74C", ToggleLeaveMatchButton, old_ToggleLeaveMatchButton);
    HOOK(targetLibName, "0x1166DD8", CmdVote_Hook, old_CmdVote);
    // Version Bypass - hook both get and set
    HOOK(targetLibName, "0x17AA3FC", ClientVersionComparison_Get_Hook, old_ClientVersionComparison_Get);
    HOOK(targetLibName, "0x17AA404", ClientVersionComparison_Set_Hook, old_ClientVersionComparison_Set);

    LOGI(OBFUSCATE("Done"));
}

// ========== LIBRARY ENTRY ==========
__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}
