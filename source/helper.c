#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <shlwapi.h>
#include "helper.h"

#pragma comment(lib, "Shlwapi.lib")

/**
* This function allocates and returns the captured pixels, its the caller's responsibility to free them.
* Capturing a 1920x1080 screen takes around 20-50 ms, while a small scan area like 8x8 takes about 5-20 ms.
* Making an init function for GetDC, etc., does not improve performance and only adds unnecessary complexity.
**/

unsigned int* get_screenshot(const char* save_name, int crop_width, int crop_height) {

    if (crop_width <= 0 || crop_height <= 0) {
        printf("Error: invalid crop dimensions.\n");
        return 0;
    }

    unsigned int* pixels = 0;
    HDC mem_dc = 0;
    FILE* file = 0;

    HDC screen_dc = GetDC(0);

    if (screen_dc == 0) {
        printf("ERROR: GetDC() failed!");
        return 0;
    }

    HBITMAP bitmap = 0;
    BITMAP bmp = { 0 };
    BITMAPFILEHEADER bmp_file_header = { 0 };

    int screen_width = GetDeviceCaps(screen_dc, DESKTOPHORZRES);
    int screen_height = GetDeviceCaps(screen_dc, DESKTOPVERTRES);

    int crop_x = 0;
    int crop_y = 0;

    // Calculate coordinates for center crop 
    crop_x = (screen_width - crop_width) / 2;
    crop_y = (screen_height - crop_height) / 2;

    // Create memory DC and compatible bitmap 
    mem_dc = CreateCompatibleDC(screen_dc);

    if (mem_dc == 0) {
        printf("ERROR: CreateCompatibleDC() failed!");
        ReleaseDC(0, screen_dc);
        return 0;
    }

    bitmap = CreateCompatibleBitmap(screen_dc, crop_width, crop_height);

    if (bitmap == 0) {
        printf("ERROR: CreateCompatibleBitmap() failed.");
        ReleaseDC(0, screen_dc);
        DeleteDC(mem_dc);
        return 0;
    }

    SelectObject(mem_dc, bitmap);

    // Copy cropped screen contents to memory DC
    if (BitBlt(mem_dc, 0, 0, crop_width, crop_height, screen_dc, crop_x, crop_y, SRCCOPY) == 0) {
        printf("ERROR: BitBlt() failed!");
        ReleaseDC(0, screen_dc);
        DeleteDC(mem_dc);
        DeleteObject(bitmap);
        return 0;
    }

    // Prepare bitmap header 
    GetObject(bitmap, sizeof(BITMAP), &bmp);

    // Create bitmap file header 
    bmp_file_header.bfType = 0x4D42; // BM 
    bmp_file_header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
        bmp.bmWidthBytes * crop_height;
    bmp_file_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Create bitmap info header 
    BITMAPINFOHEADER bmp_info_header;
    memset(&bmp_info_header, 0, sizeof(BITMAPINFOHEADER));
    bmp_info_header.biSize = sizeof(BITMAPINFOHEADER);
    bmp_info_header.biWidth = bmp.bmWidth;
    bmp_info_header.biHeight = bmp.bmHeight;
    bmp_info_header.biPlanes = 1;
    bmp_info_header.biBitCount = 32; // Use 32-bit color 
    bmp_info_header.biCompression = BI_RGB;
    bmp_info_header.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;

    if (save_name) {
        // Save bitmap to file 
        file = fopen("dbg.bmp", "wb");

        if (!file) {
            printf("Error: fopen() failed. \n");
            ReleaseDC(0, screen_dc);
            DeleteDC(mem_dc);
            DeleteObject(bitmap);
            return 0;
        }

        fwrite(&bmp_file_header, sizeof(BITMAPFILEHEADER), 1, file);
        fwrite(&bmp_info_header, sizeof(BITMAPINFOHEADER), 1, file);
    }

    // Allocate buffer for bitmap data 
    pixels = (unsigned int*)malloc(bmp.bmWidthBytes * crop_height);

    if (!pixels) {
        printf("ERROR: malloc() failed!");
        ReleaseDC(0, screen_dc);
        DeleteDC(mem_dc);
        DeleteObject(bitmap);
        return 0;
    }

    // Get bitmap data 
    if (GetDIBits(mem_dc, bitmap, 0, crop_height, pixels, (BITMAPINFO*)&bmp_info_header, DIB_RGB_COLORS) == 0) {
        printf("ERROR: GetDIBits() failed!");
        ReleaseDC(0, screen_dc);
        DeleteDC(mem_dc);
        DeleteObject(bitmap);
        free(pixels);
        return 0;
    }

    if (save_name) {
        fwrite(pixels, bmp.bmWidthBytes * crop_height, 1, file);
        fclose(file);
    }

    ReleaseDC(0, screen_dc);
    DeleteDC(mem_dc);
    DeleteObject(bitmap);

    return pixels;
}

bool is_color_found(DWORD* pixels, int pixel_count, int red, int green, int blue, int color_sens) {
    for (int i = 0; i < pixel_count; i++) {
        DWORD pixelColor = pixels[i];

        int r = GetRed(pixelColor);
        int g = GetGreen(pixelColor);
        int b = GetBlue(pixelColor);

        if (r + color_sens >= red && r - color_sens <= red) {
            if (g + color_sens >= green && g - color_sens <= green) {
                if (b + color_sens >= blue && b - color_sens <= blue) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool get_cfg(CONFIG* ini) {

    char temp[256];
    DWORD result;
    const char* iniFilePath;

    char relativePath[] = "config.ini";
    char fullPath[MAX_PATH];

    if (GetFullPathName(relativePath, MAX_PATH, fullPath, NULL)) {
        iniFilePath = fullPath;
    }
    else {
        printf("Error getting full path\n");
        return false;
    }

    if (!PathFileExists(iniFilePath)) {
        printf("Error: config.ini was not found. \n");
        return false;
    }

    result = GetPrivateProfileString("Settings", "Toggle_key", NULL, temp, sizeof(temp), iniFilePath);
    if (!result) {
        printf("Error: Failed to read 'Toggle_key' from config.ini \n");
        return false;
    }
    else
        ini->toggle_key = (int)strtol(temp, NULL, 16);

    result = GetPrivateProfileString("Settings", "scan_area", NULL, temp, sizeof(temp), iniFilePath);
    if (!result) {
        printf("Error: Failed to read 'scan_area' from config.ini \n");
        return false;
    }
    else
        ini->scan_area = atoi(temp);

    result = GetPrivateProfileString("Settings", "lock_power", NULL, temp, sizeof(temp), iniFilePath);
    if (!result) {
        printf("Error: Failed to read 'lock_power' from config.ini \n");
        return false;
    }
    else
        ini->lock_power = atoi(temp);

    result = GetPrivateProfileString("ColorRGB", "Color_sens", NULL, temp, sizeof(temp), iniFilePath);
    if (!result) {
        printf("Error: Failed to read 'color_sens' from config.ini \n");
        return false;
    }
    else
        ini->color_sens = atoi(temp);

    result = GetPrivateProfileString("ColorRGB", "Red", NULL, temp, sizeof(temp), iniFilePath);
    if (!result) {
        printf("Error: Failed to read 'red' from config.ini \n");
        return false;
    }
    else
        ini->red = atoi(temp);

    result = GetPrivateProfileString("ColorRGB", "Green", NULL, temp, sizeof(temp), iniFilePath);
    if (!result) {
        printf("Error: Failed to read 'green' from config.ini \n");
        return false;
    }
    else
        ini->green = atoi(temp);

    result = GetPrivateProfileString("ColorRGB", "Blue", NULL, temp, sizeof(temp), iniFilePath);
    if (!result) {
        printf("Error: Failed to read 'blue' from config.ini \n");
        return false;
    }
    else
        ini->blue = atoi(temp);

    return true;
}

void disable_quickedit() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;

    GetConsoleMode(hInput, &prevMode);
    SetConsoleMode(hInput, prevMode & ~ENABLE_QUICK_EDIT_MODE);
}

bool is_key_pressed(int key) {
    return GetAsyncKeyState(key) & 0x8000;
}