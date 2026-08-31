#ifndef SYSTEM_H
#define SYSTEM_H

#include <psptypes.h>

typedef struct {
    /* FPS */
    float current_fps;
    float avg_fps;
    unsigned int frame_count;
    unsigned int last_time;

    /* Per-frame tracking */
    unsigned int fps_frames;
    unsigned int fps_last_tick;
} FrameStats;

typedef struct {
    /* CPU */
    int cpu_clock;          /* MHz */
    int cpu_bus_clock;      /* MHz */

    /* RAM */
    unsigned int ram_total;     /* bytes */
    unsigned int ram_free;      /* bytes */
    unsigned int ram_usage_pct; /* 0-100 */

    /* Battery */
    int battery_percent;
    int battery_life_time;  /* minutes, -1 if charging/unknown */
    int battery_charging;
    int battery_voltage;    /* mV */
    int battery_temp;       /* Celsius if available */

    /* Uptime */
    unsigned int uptime_sec;
} SystemInfo;

/* Initialize system info struct */
void system_info_init(SystemInfo *info);

/* Update all system readings */
void system_info_update(SystemInfo *info);

#endif /* SYSTEM_H */
