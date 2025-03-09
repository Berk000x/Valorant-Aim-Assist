#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "helper.h"
#include "mouse_hook.h"

CONFIG cfg;
bool player_found = false;

DWORD WINAPI thread(PVOID param) {

    bool is_active = true;
    int pixel_count = cfg.scan_area * cfg.scan_area;
    int scan_area_x = cfg.scan_area;
    int scan_area_y = cfg.scan_area;

    while (true) {
        if (is_key_pressed(cfg.toggle_key)) {
            Beep(500, 100);
            player_found = false;
            is_active = !is_active;
            Sleep(150);
        }

        if (!is_active) {
            Sleep(1);
            continue;
        }

        unsigned int* pixels = get_screenshot(0, scan_area_x, scan_area_y);

        if (pixels == 0) {
            printf("Error: get_screenshot failed! \n");
            return 0;
        }

        if (is_color_found(pixels, pixel_count, cfg.red, cfg.green, cfg.blue, cfg.color_sens))
            player_found = true;
        else
            player_found = false;

        free(pixels);

        Sleep(1);
    }
    return 0;
}

// Dummy WndProc
LRESULT WndProc(HWND hWnd, unsigned int message, unsigned long long wParam, long long lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int main() {
    if (!get_cfg(&cfg)) {
        printf("Error: get_cfg() failed. \n");
        printf("\nPress enter to exit: "); getchar();
        return 1;
    }

    if (!start_mouse_hook()) {
        printf("Error: start_mouse_hook() failed. \n");
        printf("\nPress enter to exit: "); getchar();
        return 1;
    }

    HANDLE thread1 = CreateThread(NULL, 0, thread, NULL, 0, NULL);

    if (thread1 == NULL) {
        printf("Error: thread1 failed. \n");
        printf("\nPress enter to exit: "); getchar();
        return 1;
    }

    Beep(500, 100);
    disable_quickedit();
    printf("The aim assist is running successfully. Go into a game and enjoy! \n");
    
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		Sleep(1);
	}

    CloseHandle(thread1);

    return 0;
}
