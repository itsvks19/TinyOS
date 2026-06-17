#pragma once

typedef struct {
    int x;
    int y;

    int width;
    int height;

    int dragging;

    int drag_offset_x;
    int drag_offset_y;
} window_t;