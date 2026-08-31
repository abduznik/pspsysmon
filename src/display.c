/*
 * display.c — Overlay rendering for PSPSysMon
 *
 * Renders system stats as a semi-transparent overlay on the framebuffer.
 * Uses sceDisplayGetFrameBuf for direct framebuffer access.
 * Built-in 4x6 bitmap font for text rendering.
 */

#include "psp_compat.h"

#include <pspkernel.h>
#include <pspdisplay.h>
#include <string.h>
#include <stdio.h>

#include "display.h"
#include "system.h"
#include "font.h"

/* Overlay position and style */
#define OVERLAY_X       10
#define OVERLAY_Y       10
#define LINE_HEIGHT     9
#define CHAR_WIDTH      5
#define BG_ALPHA        0x80   /* Semi-transparent black background */
#define TEXT_COLOR       0x00FFFFFF  /* White */
#define TEXT_COLOR_DIM   0x00AAAAAA  /* Dim gray for labels */
#define TEXT_COLOR_WARN  0x00FF4444  /* Red warning */

/* ── Helpers ───────────────────────────────────────────────── */

static void draw_rect(unsigned int *vram, int x, int y, int w, int h,
                       unsigned int color, int fb_width)
{
    for (int j = y; j < y + h && j < 272; j++) {
        for (int i = x; i < x + w && i < fb_width; i++) {
            unsigned int dst = vram[j * fb_width + i];
            /* Alpha blend */
            unsigned int sa = (color >> 24) & 0xFF;
            unsigned int da = 255 - sa;
            unsigned int r = ((color >> 16) & 0xFF) * sa / 255 + ((dst >> 16) & 0xFF) * da / 255;
            unsigned int g = ((color >> 8) & 0xFF) * sa / 255 + ((dst >> 8) & 0xFF) * da / 255;
            unsigned int b = (color & 0xFF) * sa / 255 + (dst & 0xFF) * da / 255;
            vram[j * fb_width + i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
}

static void draw_char(unsigned int *vram, int x, int y, char c,
                        unsigned int color, int fb_width)
{
    if (c < 32 || c > 126) c = '?';
    int idx = c - 32;
    const unsigned char *glyph = font_5x7[idx];

    for (int row = 0; row < 7; row++) {
        unsigned char bits = glyph[row];
        for (int col = 0; col < 5; col++) {
            if (bits & (1 << (4 - col))) {
                int px = x + col;
                int py = y + row;
                if (px < fb_width && py < 272) {
                    vram[py * fb_width + px] = color;
                }
            }
        }
    }
}

static void draw_string(unsigned int *vram, int x, int y, const char *str,
                          unsigned int color, int fb_width)
{
    while (*str) {
        draw_char(vram, x, y, *str, color, fb_width);
        x += CHAR_WIDTH + 1;
        str++;
    }
}

static void draw_int(unsigned int *vram, int x, int y, int val,
                       unsigned int color, int fb_width)
{
    char buf[16];
    int i = 0;
    if (val < 0) { buf[i++] = '-'; val = -val; }
    if (val == 0) { buf[i++] = '0'; }
    else {
        char tmp[12];
        int j = 0;
        while (val > 0) { tmp[j++] = '0' + (val % 10); val /= 10; }
        while (j > 0) { buf[i++] = tmp[--j]; }
    }
    buf[i] = '\0';
    draw_string(vram, x, y, buf, color, fb_width);
}

/* ── Main overlay renderer ─────────────────────────────────── */

void display_render_overlay(const SystemInfo *sys, const FrameStats *fps)
{
    void *fb_ptr = NULL;
    int fb_width = 0, fb_height = 0, fb_sync = 0;

    /* Get current framebuffer */
    sceDisplayGetFrameBuf(&fb_ptr, &fb_width, &fb_height, &fb_sync);

    if (!fb_ptr || fb_width <= 0) return;

    unsigned int *vram = (unsigned int *)fb_ptr;

    /* Calculate overlay dimensions */
    int lines = 6; /* FPS, CPU, RAM, BAT, UPT, spacer */
    int overlay_w = 160;
    int overlay_h = lines * LINE_HEIGHT + 8;

    /* Draw semi-transparent background */
    draw_rect(vram, OVERLAY_X, OVERLAY_Y, overlay_w, overlay_h, BG_ALPHA, fb_width);

    int y = OVERLAY_Y + 4;
    int x_label = OVERLAY_X + 4;
    int x_value = OVERLAY_X + 50;

    /* FPS */
    draw_string(vram, x_label, y, "FPS", TEXT_COLOR_DIM, fb_width);
    if (fps->current_fps < 20.0f) {
        draw_int(vram, x_value, y, (int)fps->current_fps, TEXT_COLOR_WARN, fb_width);
    } else {
        draw_int(vram, x_value, y, (int)fps->current_fps, TEXT_COLOR, fb_width);
    }
    draw_string(vram, x_value + 20, y, "(avg", TEXT_COLOR_DIM, fb_width);
    draw_int(vram, x_value + 45, y, (int)fps->avg_fps, TEXT_COLOR_DIM, fb_width);
    draw_string(vram, x_value + 65, y, ")", TEXT_COLOR_DIM, fb_width);
    y += LINE_HEIGHT;

    /* CPU Clock */
    draw_string(vram, x_label, y, "CPU", TEXT_COLOR_DIM, fb_width);
    draw_int(vram, x_value, y, sys->cpu_clock, TEXT_COLOR, fb_width);
    draw_string(vram, x_value + 22, y, "MHz", TEXT_COLOR_DIM, fb_width);
    y += LINE_HEIGHT;

    /* Bus Clock */
    draw_string(vram, x_label, y, "BUS", TEXT_COLOR_DIM, fb_width);
    draw_int(vram, x_value, y, sys->cpu_bus_clock, TEXT_COLOR, fb_width);
    draw_string(vram, x_value + 22, y, "MHz", TEXT_COLOR_DIM, fb_width);
    y += LINE_HEIGHT;

    /* RAM */
    draw_string(vram, x_label, y, "RAM", TEXT_COLOR_DIM, fb_width);
    /* Show free RAM in KB */
    draw_int(vram, x_value, y, sys->ram_free / 1024, TEXT_COLOR, fb_width);
    draw_string(vram, x_value + 30, y, "KB free", TEXT_COLOR_DIM, fb_width);
    y += LINE_HEIGHT;

    /* Battery */
    draw_string(vram, x_label, y, "BAT", TEXT_COLOR_DIM, fb_width);
    if (sys->battery_charging) {
        draw_string(vram, x_value, y, "CHG", TEXT_COLOR, fb_width);
    } else if (sys->battery_percent >= 0) {
        draw_int(vram, x_value, y, sys->battery_percent, TEXT_COLOR, fb_width);
        draw_string(vram, x_value + 18, y, "%", TEXT_COLOR_DIM, fb_width);
        if (sys->battery_percent <= 15) {
            draw_string(vram, x_value + 28, y, "!", TEXT_COLOR_WARN, fb_width);
        }
    } else {
        draw_string(vram, x_value, y, "---", TEXT_COLOR_DIM, fb_width);
    }
    /* Voltage */
    if (sys->battery_voltage > 0) {
        draw_int(vram, x_value + 45, y, sys->battery_voltage, TEXT_COLOR_DIM, fb_width);
        draw_string(vram, x_value + 75, y, "mV", TEXT_COLOR_DIM, fb_width);
    }
    y += LINE_HEIGHT;

    /* Uptime */
    draw_string(vram, x_label, y, "UP", TEXT_COLOR_DIM, fb_width);
    {
        unsigned int sec = sys->uptime_sec;
        int h = sec / 3600;
        int m = (sec % 3600) / 60;
        int s = sec % 60;
        char time_buf[12];
        time_buf[0] = '0' + h; time_buf[1] = ':';
        time_buf[2] = '0' + m / 10; time_buf[3] = '0' + m % 10; time_buf[4] = ':';
        time_buf[5] = '0' + s / 10; time_buf[6] = '0' + s % 10; time_buf[7] = '\0';
        draw_string(vram, x_value, y, time_buf, TEXT_COLOR, fb_width);
    }
}
