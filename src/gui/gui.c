#include "gui.h"

#include "../drivers/mouse/mouse.h"
#include "../framebuffer.h"
#include "window.h"

#define BTN_X 100
#define BTN_Y 100
#define BTN_W 160
#define BTN_H 40
#define TITLEBAR_H 24

static int button_pressed = 0;
static int prev_left = 0;
static int gui_dirty = 1;

static window_t terminal_window = {.x = 200, .y = 120, .width = 500, .height = 300};

static void draw_window(void) {
    fb_fill_rect(terminal_window.x, terminal_window.y, terminal_window.width,
                 terminal_window.height, RGB(40, 40, 40));
    fb_fill_rect(terminal_window.x, terminal_window.y, terminal_window.width, TITLEBAR_H,
                 RGB(0, 100, 255));
    fb_draw_string(terminal_window.x + 8, terminal_window.y + 8, "Terminal", RGB(255, 255, 255),
                   RGB(0, 100, 255));
}

void gui_draw(void) {
    if (!gui_dirty)
        return;
    gui_dirty = 0;
    fb_clear(RGB(20, 20, 20));
    draw_window();
    mouse_state_t mouse = mouse_get_state();
    fb_fill_rect(mouse.x, mouse.y, 8, 8, RGB(255, 255, 255));
    fb_present();
}

void gui_update(void) {
    mouse_state_t mouse = mouse_get_state();
    int inside_titlebar = mouse.x >= terminal_window.x &&
                          mouse.x < terminal_window.x + terminal_window.width &&
                          mouse.y >= terminal_window.y && mouse.y < terminal_window.y + TITLEBAR_H;

    if (!prev_left && mouse.left && inside_titlebar) {
        terminal_window.dragging = 1;
        terminal_window.drag_offset_x = mouse.x - terminal_window.x;
        terminal_window.drag_offset_y = mouse.y - terminal_window.y;
    }
    if (!mouse.left)
        terminal_window.dragging = 0;

    if (terminal_window.dragging) {
        terminal_window.x = mouse.x - terminal_window.drag_offset_x;
        terminal_window.y = mouse.y - terminal_window.drag_offset_y;
    }
    prev_left = mouse.left;
    if (terminal_window.dragging) {
        terminal_window.x = mouse.x - terminal_window.drag_offset_x;
        terminal_window.y = mouse.y - terminal_window.drag_offset_y;

        gui_dirty = 1;
    }
}
void cursor_draw(void) {
    mouse_state_t mouse = mouse_get_state();

    fb_fill_rect(mouse.x, mouse.y, 8, 8, RGB(255, 255, 255));
    fb_present();
}
void gui_init(void) { button_pressed = 0; }
void gui_invalidate(void) { gui_dirty = 1; }