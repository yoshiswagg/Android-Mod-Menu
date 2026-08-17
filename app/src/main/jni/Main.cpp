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
bool deadChatToggle = false;
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
void (*CmdSendMessage)(void* instance, void* playerId, void* message);
void* (*il2cpp_string_new)(const char* str);

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
        case 324:
            deadChatToggle = boolean;
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
    CmdSendMessage = (void (*)(void*, void*, void*))getAbsoluteAddress(targetLibName, OBFUSCATE("0x24F2D40"));
    
    // Get il2cpp_string_new function
    il2cpp_string_new = (void* (*)(const char*))getAbsoluteAddress(targetLibName, OBFUSCATE("il2cpp_string_new"));

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
    HOOK(targetLibName, "0x24F2D40", CmdSendMessage_Hook, old_CmdSendMessage);
    HOOK(targetLibName, "0x2A8ED28", SetInputInteractable_Hook, old_SetInputInteractable);

    LOGI(OBFUSCATE("Done"));
}

// ========== LIBRARY ENTRY ==========
__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}
