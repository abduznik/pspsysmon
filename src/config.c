/*
 * config.c — Configuration management for PSPSysMon
 *
 * Reads/writes simple key=value config from:
 *   ms0:/PSP/SYSTEM/pspsysmon.cfg
 */

#include <pspkernel.h>
#include <pspfileio.h>
#include <string.h>

#include "config.h"

#define CFG_PATH "ms0:/PSP/SYSTEM/pspsysmon.cfg"
#define CFG_MAGIC 0x50534D00 /* "PSM\0" */
#define CFG_VERSION 1

SysMonConfig g_config = {
    .overlay_x = CFG_DEFAULT_OVERLAY_X,
    .overlay_y = CFG_DEFAULT_OVERLAY_Y,
    .bg_alpha  = CFG_DEFAULT_ALPHA,
    .enabled   = CFG_DEFAULT_ENABLED,
};

/* Simple binary config format for reliability */
typedef struct {
    unsigned int magic;
    int version;
    int overlay_x;
    int overlay_y;
    int bg_alpha;
    int enabled;
    int reserved[8]; /* Future use */
} ConfigFile;

void config_load(void)
{
    SceUID fd = sceIoOpen(CFG_PATH, PSP_O_RDONLY, 0);
    if (fd < 0) {
        /* First run — create default config */
        config_save();
        return;
    }

    ConfigFile cfg;
    int read = sceIoRead(fd, &cfg, sizeof(cfg));
    sceIoClose(fd);

    if (read != sizeof(cfg) || cfg.magic != CFG_MAGIC || cfg.version != CFG_VERSION) {
        /* Invalid config — keep defaults */
        return;
    }

    g_config.overlay_x = cfg.overlay_x;
    g_config.overlay_y = cfg.overlay_y;
    g_config.bg_alpha  = cfg.bg_alpha;
    g_config.enabled   = cfg.enabled;
}

void config_save(void)
{
    /* Ensure directory exists */
    sceIoMkdir("ms0:/PSP/SYSTEM", 0777);

    ConfigFile cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.magic    = CFG_MAGIC;
    cfg.version  = CFG_VERSION;
    cfg.overlay_x = g_config.overlay_x;
    cfg.overlay_y = g_config.overlay_y;
    cfg.bg_alpha  = g_config.bg_alpha;
    cfg.enabled   = g_config.enabled;

    SceUID fd = sceIoOpen(CFG_PATH, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, &cfg, sizeof(cfg));
        sceIoClose(fd);
    }
}
