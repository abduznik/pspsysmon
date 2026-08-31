#ifndef DISPLAY_H
#define DISPLAY_H

#include "system.h"

/* Initialize the frame statistics tracker */
void frame_stats_init(FrameStats *stats);

/* Update frame counter (call once per render cycle) */
void frame_stats_update(FrameStats *stats);

/* Render the overlay on the current framebuffer */
void display_render_overlay(const SystemInfo *sys, const FrameStats *fps);

#endif /* DISPLAY_H */
