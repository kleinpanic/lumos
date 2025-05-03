#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "brightness.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>    
#include <errno.h>
#include <dirent.h>
#include <unistd.h>         
#include <math.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

char brightness_path[PATH_MAX];
char max_brightness_path[PATH_MAX];

void init_backlight_paths(const char *interface) {
    if (interface) {
        snprintf(brightness_path,     PATH_MAX, "%s/%s/brightness",
                 BACKLIGHT_BASE_PATH, interface);
        snprintf(max_brightness_path, PATH_MAX, "%s/%s/max_brightness",
                 BACKLIGHT_BASE_PATH, interface);
        if (access(brightness_path, R_OK|W_OK) != 0 ||
            access(max_brightness_path, R_OK)  != 0)
        {
            if (errno == EACCES)
                fprintf(stderr,
                        "Permission denied on %s or %s\n",
                        brightness_path, max_brightness_path);
            else
                fprintf(stderr,
                        "Cannot access %s or %s: %s\n",
                        brightness_path, max_brightness_path,
                        strerror(errno));
            exit(EXIT_FAILURE);
        }
        return;
    }

    /* auto‑detect */
    DIR *d = opendir(BACKLIGHT_BASE_PATH);
    if (!d) {
        fprintf(stderr,
                "Unable to open %s: %s\n",
                BACKLIGHT_BASE_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        char cand_bright[PATH_MAX];
        char cand_max[PATH_MAX];
        snprintf(cand_bright, PATH_MAX,
                 "%s/%s/brightness",
                 BACKLIGHT_BASE_PATH, entry->d_name);
        snprintf(cand_max,    PATH_MAX,
                 "%s/%s/max_brightness",
                 BACKLIGHT_BASE_PATH, entry->d_name);

        if (access(cand_bright, R_OK|W_OK) == 0 &&
            access(cand_max,    R_OK)     == 0)
        {
            strncpy(brightness_path,    cand_bright, PATH_MAX);
            strncpy(max_brightness_path, cand_max,    PATH_MAX);
            closedir(d);
            return;
        }
    }
    closedir(d);

    fprintf(stderr,
            "No writable backlight interface found in %s\n",
            BACKLIGHT_BASE_PATH);
    exit(EXIT_FAILURE);
}

int get_current_brightness(void) {
    FILE *f = fopen(brightness_path, "r");
    if (!f) {
        fprintf(stderr,
                "Error opening %s: %s\n",
                brightness_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int val;
    if (fscanf(f, "%d", &val) != 1) {
        fprintf(stderr,
                "Error reading %s\n",
                brightness_path);
        fclose(f);
        exit(EXIT_FAILURE);
    }
    fclose(f);
    return val;
}

int get_max_brightness(void) {
    FILE *f = fopen(max_brightness_path, "r");
    if (!f) {
        fprintf(stderr,
                "Error opening %s: %s\n",
                max_brightness_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int val;
    if (fscanf(f, "%d", &val) != 1) {
        fprintf(stderr,
                "Error reading %s\n",
                max_brightness_path);
        fclose(f);
        exit(EXIT_FAILURE);
    }
    fclose(f);
    return val;
}

int write_brightness(int value) {
    FILE *f = fopen(brightness_path, "w");
    if (!f) {
        fprintf(stderr,
                "Error writing %s: %s\n",
                brightness_path, strerror(errno));
        return -1;
    }
    if (fprintf(f, "%d\n", value) < 0) {
        fprintf(stderr,
                "Failed to write brightness to %s: %s\n",
                brightness_path, strerror(errno));
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

void set_brightness(int value, int raw, int no_display) {
    int max = get_max_brightness();
    int target = raw ? value : (value * max) / 100;
    if (target < 0)   target = 0;
    if (target > max) target = max;
    if (write_brightness(target) != 0)
        exit(EXIT_FAILURE);
    if (!no_display)
        display_brightness(target);
}

void adjust_brightness(const char *direction, int no_display) {
    int cur = get_current_brightness();
    int max = get_max_brightness();
    int step = max / 10;
    int target;
    if (strcmp(direction, "up") == 0)
        target = cur + step;
    else if (strcmp(direction, "down") == 0)
        target = cur - step;
    else {
        fprintf(stderr,
                "Invalid adjust '%s' (use up/down)\n",
                direction);
        exit(EXIT_FAILURE);
    }
    if (target < 0)   target = 0;
    if (target > max) target = max;
    if (write_brightness(target) != 0)
        exit(EXIT_FAILURE);
    if (!no_display)
        display_brightness(target);
}

void draw_icon(Display *d, Window w, GC gc) {
    int circle_radius = 22;     // Radius for a smaller circle
    int ray_length = 27;        // Length of the rays
    int line_thickness = 5;     // Increased line thickness
    int y_offset = 10;          // Amount to move the icon down

    XSetLineAttributes(d, gc, line_thickness, LineSolid, CapRound, JoinRound);

    XDrawArc(d, w, gc,
             100 - circle_radius,
             40  - circle_radius + y_offset,
             2 * circle_radius,
             2 * circle_radius,
             0,
             360 * 64);

    for (int i = 0; i < 8; ++i) {
        double angle = i * M_PI / 4.0;
        int x1 = 100 + (int)((circle_radius + 5) * cos(angle));
        int y1 = 40  + (int)((circle_radius + 5) * sin(angle)) + y_offset;
        int x2 = 100 + (int)((circle_radius + ray_length) * cos(angle));
        int y2 = 40  + (int)((circle_radius + ray_length) * sin(angle)) + y_offset;
        XDrawLine(d, w, gc, x1, y1, x2, y2);
    }
}

void display_brightness(int brightness) {
    Display *d;
    Window w;
    XEvent e;
    int screen;
    unsigned int display_width, display_height;
    int width = 200, height = 120;
    int sections = 10;
    int graph_height = 10;

    if ((d = XOpenDisplay(NULL)) == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(EXIT_FAILURE);
    }

    screen = DefaultScreen(d);
    display_width  = DisplayWidth(d, screen);
    display_height = DisplayHeight(d, screen);

    w = XCreateSimpleWindow(d,
            RootWindow(d, screen),
            (display_width - width)  / 2,
            (display_height - height) / 2,
            width, height, 1,
            BlackPixel(d, screen),
            WhitePixel(d, screen));
    XSetWindowBackground(d, w, 0xD3D3D3); // Light gray
    XStoreName(d, w, "lumos");
    XClassHint *classHint = XAllocClassHint();
    if (classHint) {
        classHint->res_name  = "lumos";
        classHint->res_class = "lumos";
        XSetClassHint(d, w, classHint);
        XFree(classHint);
    }

    XSelectInput(d, w, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(d, w);

    GC gc    = XCreateGC(d, w, 0, NULL);
    GC bg_gc = XCreateGC(d, w, 0, NULL);
    XSetForeground(d, gc,    BlackPixel(d, screen));
    XSetBackground(d, bg_gc, WhitePixel(d, screen));
    XSetLineAttributes(d, gc, 3, LineSolid, CapButt, JoinMiter);

    int max_brightness = get_max_brightness();

    /* wait until mapped */
    while (1) {
        XNextEvent(d, &e);
        if (e.type == MapNotify) break;
    }

    /* icon */
    draw_icon(d, w, gc);

    /* graph background */
    XSetForeground(d, bg_gc, 0xA9A9A9); // Dark gray
    XFillRectangle(d, w, bg_gc, 10, 100, 180, graph_height);

    /* graph fill */
    int filled = (int)((brightness / (double)max_brightness) * sections);
    for (int i = 0; i < sections; ++i) {
        if (i < filled)
            XSetForeground(d, gc, 0xFFFFFF);
        else
            XSetForeground(d, gc, 0xA9A9A9);
        XFillRectangle(
            d, w, gc,
            10 + i * (180 / sections),
            100,
            (180 / sections) - 2,
            graph_height
        );
    }

    XFlush(d);
    sleep(1);
    XDestroyWindow(d, w);
    XCloseDisplay(d);
}

/* - DDC/CI Support - */

int set_ddc_brightness(const char *i2c_dev, int value) {
    int fd = open(i2c_dev, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open %s: %s\n", i2c_dev, strerror(errno));
        return -1;
    }
    const int addr = 0x37;  // standard DDC/CI adapter address
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        fprintf(stderr, "I2C_SLAVE ioctl failed on %s: %s\n",
                i2c_dev, strerror(errno));
        close(fd);
        return -1;
    }
    // VCP code 0x10 = brightness (0–100)
    if (i2c_smbus_write_byte_data(fd, 0x10, value) < 0) {
        fprintf(stderr, "DDC write failed on %s: %s\n",
                i2c_dev, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int read_ddc_brightness(const char *i2c_dev, int *current, int *max) {
    int fd = open(i2c_dev, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Cannot open %s for read: %s\n",
                i2c_dev, strerror(errno));
        return -1;
    }
    const int addr = 0x37;
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        close(fd);
        return -1;
    }
    __s32 word = i2c_smbus_read_word_data(fd, 0x10);
    close(fd);
    if (word < 0) return -1;
    *current = (word >> 8) & 0xFF;
    *max     = word & 0xFF;
    return 0;
}

int adjust_ddc_brightness(const char *i2c_dev, const char *direction) {
    int cur, mx;
    if (read_ddc_brightness(i2c_dev, &cur, &mx) < 0) {
        fprintf(stderr, "Failed to read current DDC brightness\n");
        return -1;
    }
    int step = mx / 10, tgt;
    if (strcmp(direction, "up") == 0)      tgt = cur + step;
    else if (strcmp(direction, "down") == 0) tgt = cur - step;
    else {
        fprintf(stderr, "Invalid external adjust '%s' (use up/down)\n",
                direction);
        return -1;
    }
    if (tgt < 0) tgt = 0;
    if (tgt > mx) tgt = mx;
    return set_ddc_brightness(i2c_dev, tgt);
}

char *resolve_ddc_connector(const char *connector) {
    DIR *d = opendir("/sys/class/drm");
    if (!d) return NULL;
    struct dirent *e;

    // also try mapping "HDMI-1" → "HDMI-A-1"
    char alt[64] = {0};
    char *dash = strrchr(connector, '-');
    if (dash) {
        size_t prefix = dash - connector;
        if (prefix < sizeof(alt) && strlen(dash+1) < sizeof(alt) - prefix - 3) {
            snprintf(alt, sizeof(alt), "%.*s-A-%s",
                     (int)prefix, connector, dash+1);
        }
    }

    while ((e = readdir(d))) {
        if (e->d_name[0] == '.') continue;
        // match either the raw connector or the alt form
        if (!strcasestr(e->d_name, connector) &&
            (alt[0] == '\0' || !strcasestr(e->d_name, alt)))
            continue;

        char drm_path[PATH_MAX];
        snprintf(drm_path, sizeof(drm_path), "/sys/class/drm/%s", e->d_name);

        DIR *d2 = opendir(drm_path);
        if (!d2) continue;
        struct dirent *e2;
        while ((e2 = readdir(d2))) {
            // look for either i2c adapters or drm_dp_aux nodes
            if (strncmp(e2->d_name, "i2c-", 4) != 0 &&
                strncmp(e2->d_name, "drm_dp_aux", 10) != 0)
                continue;

            // safely build the sysfs path to the adapter
            char syspath[PATH_MAX];
            int L = snprintf(syspath, sizeof(syspath),
                             "%s/%s", drm_path, e2->d_name);
            if (L < 0 || L >= (int)sizeof(syspath)) continue;

            // resolve the symlink, yielding something like ".../i2c-4" or ".../drm_dp_aux0"
            char linkt[PATH_MAX];
            ssize_t r = readlink(syspath, linkt, sizeof(linkt)-1);
            if (r < 0) continue;
            linkt[r] = '\0';

            // take the part after the last '/'
            char *p = strrchr(linkt, '/');
            if (!p) continue;

            // form the device node path
            char devpath[PATH_MAX];
            snprintf(devpath, sizeof(devpath), "/dev/%s", p+1);

            // only accept it if the node really exists
            if (access(devpath, R_OK|W_OK) == 0) {
                closedir(d2);
                closedir(d);
                return strdup(devpath);
            }
        }
        closedir(d2);
    }

    closedir(d);
    return NULL;
}

/* Skeletons for profiles & presets */
int profile_new(const char *name) {
    /* TODO: launch TUI to collect settings for 'name' */
    (void)name;
    return 0;
}

int profile_load(const char *name) {
    /* TODO: apply stored settings for 'name' */
    (void)name;
    return 0;
}
