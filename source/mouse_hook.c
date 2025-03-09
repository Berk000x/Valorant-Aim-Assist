#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "mouse_hook.h"
#include "helper.h"

CONFIG cfg;
extern bool player_found;
HHOOK hook;

LRESULT CALLBACK hook_callback(int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode >= 0 && wParam == WM_MOUSEMOVE && player_found) {

        /**
        * This function is hooked by Vanguard, so the return statements don't have any effect, but calling the Sleep() function does.
        * But be careful, if you sleep for too long, it could delay other mouse functions like clicks as well.
        **/

        Sleep(cfg.lock_power); // Block the mouse movement input.
        // return 1;
    }

    return CallNextHookEx(hook, nCode, wParam, lParam);
}

bool start_mouse_hook() {

    hook = SetWindowsHookEx(WH_MOUSE_LL, hook_callback, 0, 0);
    
    if (!hook) {
        printf("Failed to set hook!\n");
        return false;
    }

    return true;
}
