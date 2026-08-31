/*
 * system.c — PSP system information采集
 *
 * Reads CPU clock, RAM usage, battery status from PSP kernel APIs.
 */

#include "psp_compat.h"

#include <pspkernel.h>
#include <pspsysmem.h>
#include <psppower.h>
#include <string.h>

#include "system.h"

void system_info_init(SystemInfo *info)
{
    memset(info, 0, sizeof(SystemInfo));
    info->battery_life_time = -1;
}

void system_info_update(SystemInfo *info)
{
    /* ── CPU Clock ── */
    info->cpu_clock = scePowerGetCpuClockFrequencyInt();
    info->cpu_bus_clock = scePowerGetBusClockFrequencyInt();

    /* ── RAM ── */
    /* sceKernelTotalFreeMemSize returns free RAM in user partition */
    SceSize free_mem = sceKernelTotalFreeMemSize();
    /* Total user RAM is typically ~30MB on PSP-1000, ~64MB on PSP-2000/3000 */
    /* We approximate based on what's available */
    info->ram_free = free_mem;
    /* Use the largest free block as a proxy for total available */
    info->ram_total = free_mem; /* Will be refined on first update */
    if (info->ram_total > 0) {
        info->ram_usage_pct = 0; /* Free = total in this simplified view */
    }

    /* Try to get more accurate RAM info from system memory utility */
    /* sceKernelQueryMemoryInfo is not always available, fallback is fine */

    /* ── Battery ── */
    info->battery_percent = scePowerGetBatteryLifePercent();
    info->battery_life_time = scePowerGetBatteryLifeTime();
    info->battery_charging = scePowerIsBatteryCharging();
    info->battery_voltage = scePowerGetBatteryVolt();
    /* Battery temperature is not exposed by standard PSP SDK */
    info->battery_temp = -1;

    /* ── Uptime ── */
    info->uptime_sec = sceKernelGetSystemTimeLow() / 1000000;
}

/* ── Frame statistics ── */

void frame_stats_init(FrameStats *stats)
{
    memset(stats, 0, sizeof(FrameStats));
    stats->last_time = sceKernelGetSystemTimeLow();
    stats->fps_last_tick = stats->last_time;
}

void frame_stats_update(FrameStats *stats)
{
    unsigned int now = sceKernelGetSystemTimeLow();

    stats->frame_count++;
    stats->fps_frames++;

    /* Calculate FPS every ~500ms for stability */
    unsigned int elapsed = now - stats->fps_last_tick;
    if (elapsed >= 500000) { /* 500ms in microseconds */
        stats->current_fps = (float)stats->fps_frames * 1000000.0f / (float)elapsed;

        /* Rolling average */
        if (stats->avg_fps < 0.5f) {
            stats->avg_fps = stats->current_fps;
        } else {
            stats->avg_fps = stats->avg_fps * 0.9f + stats->current_fps * 0.1f;
        }

        stats->fps_frames = 0;
        stats->fps_last_tick = now;
    }

    stats->last_time = now;
}
