/*
 * PSPSysMon — PSP System Monitor Overlay Plugin
 *
 * Displays real-time FPS, CPU clock, RAM usage, and battery stats
 * as an overlay on top of any game or application.
 *
 * Toggle: L+R+START
 *
 * License: MIT
 */

#include <pspkernel.h>
#include <pspdisplay.h>
#include <psppower.h>
#include <pspsysmem.h>
#include <pspmoduleinfo.h>
#include <psploadcore.h>
#include <pspctrl.h>
#include <string.h>

#include "display.h"
#include "system.h"
#include "config.h"

PSP_MODULE_INFO("PSPSysMon", PSP_MODULE_KERNEL, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

/* ── State ─────────────────────────────────────────────────── */

static volatile int running = 1;
static volatile int overlay_enabled = 1;

static SceUID main_thread_id = -1;

/* ── Button polling (toggle: L+R+START) ────────────────────── */

static u32 prev_buttons = 0;

static void poll_buttons(void)
{
    SceCtrlData pad;
    sceCtrlPeekBufferPositive(&pad, 1);

    u32 pressed = pad.Buttons & ~prev_buttons;
    prev_buttons = pad.Buttons;

    if ((pressed & (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START)) ==
        (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START)) {
        overlay_enabled = !overlay_enabled;
    }
}

/* ── Display thread ────────────────────────────────────────── */

static int display_thread(SceSize args, void *argp)
{
    (void)args;
    (void)argp;

    SystemInfo sys;
    FrameStats fps;

    system_info_init(&sys);
    frame_stats_init(&fps);

    while (running) {
        poll_buttons();
        /* Sample system data every frame */
        system_info_update(&sys);
        frame_stats_update(&fps);

        if (overlay_enabled) {
            display_render_overlay(&sys, &fps);
        }

        /* ~60fps polling (16ms) */
        sceKernelDelayThread(16000);
    }

    return 0;
}

/* ── Module start ──────────────────────────────────────────── */

int module_start(SceSize args, void *argp)
{
    (void)args;
    (void)argp;

    config_load();

    /* Start display thread at high priority */
    main_thread_id = sceKernelCreateThread(
        "SysMonDisplay",
        display_thread,
        0x10,          /* high priority */
        0x1000,        /* 4KB stack */
        0, NULL);

    if (main_thread_id >= 0) {
        sceKernelStartThread(main_thread_id, 0, NULL);
    }

    return 0;
}

/* ── Module stop ───────────────────────────────────────────── */

int module_stop(SceSize args, void *argp)
{
    (void)args;
    (void)argp;

    running = 0;

    /* Wait for display thread to finish */
    if (main_thread_id >= 0) {
        sceKernelWaitThreadEnd(main_thread_id, NULL);
        sceKernelDeleteThread(main_thread_id);
    }

    config_save();

    return 0;
}
