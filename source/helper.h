#ifndef HELPER_H
#define HELPER_H

#define GetBlue(rgb)     (LOBYTE(rgb))
#define GetGreen(rgb)    (LOBYTE(((WORD)(rgb)) >> 8))
#define GetRed(rgb)      (LOBYTE((rgb)>>16))

typedef struct {
    int toggle_key;
    int scan_area;
    int lock_power;

    int color_sens;
    int red;
    int green;
    int blue;
} CONFIG;


unsigned int* get_screenshot(const char* save_name, int crop_width, int crop_height);
bool is_color_found(DWORD* pPixels, int pixel_count, int red, int green, int blue, int color_sens);
bool get_cfg(CONFIG* ini);
void disable_quickedit();
bool is_key_pressed(int key);

#endif