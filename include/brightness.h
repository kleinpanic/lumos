#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <limits.h>             // for PATH_MAX
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define BACKLIGHT_BASE_PATH "/sys/class/backlight"

/* Sysfs backlight paths (internal) */
extern char brightness_path[PATH_MAX];
extern char max_brightness_path[PATH_MAX];
void init_backlight_paths(const char *interface);

/* Internal backlight I/O */
int  get_current_brightness(void);
int  get_max_brightness(void);
int  write_brightness(int value);
void set_brightness(int value, int raw, int no_display);
void adjust_brightness(const char *direction, int no_display);
void display_brightness(int brightness);

/* DDC/CI for external monitors (I2C‑dev based) */
int  set_ddc_brightness(const char *i2c_dev, int value);
char *resolve_ddc_connector(const char *connector);
int  read_ddc_brightness(const char *i2c_dev, int *current, int *max);
int  adjust_ddc_brightness(const char *i2c_dev, const char *direction);

/* Profiles & presets (skeleton) */
int  profile_new(const char *name);
int  profile_load(const char *name);

#endif // BRIGHTNESS_H

