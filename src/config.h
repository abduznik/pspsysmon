#ifndef CONFIG_H
#define CONFIG_H

/* Default configuration */
#define CFG_DEFAULT_OVERLAY_X    10
#define CFG_DEFAULT_OVERLAY_Y    10
#define CFG_DEFAULT_ALPHA        0x80
#define CFG_DEFAULT_ENABLED      1

typedef struct {
    int overlay_x;
    int overlay_y;
    int bg_alpha;
    int enabled;
} SysMonConfig;

/* Load config from ms0:/PSP/SYSTEM/pspsysmon.cfg */
void config_load(void);

/* Save current config */
void config_save(void);

/* Global config instance */
extern SysMonConfig g_config;

#endif /* CONFIG_H */
