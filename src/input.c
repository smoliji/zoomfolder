#include "input.h"

#define ZOOM_SPEED 0.1f
#define ZOOM_MIN 1.0f
#define ZOOM_MAX 100.0f

static bool dragging = false;

static void clamp_camera(Camera *cam, int window_w)
{
    if (cam->target_offset_y > 0) cam->target_offset_y = 0;

    if (cam->target_offset_x > 0) cam->target_offset_x = 0;
    float min_x = (float)window_w / cam->target_zoom - (float)window_w;
    if (cam->target_offset_x < min_x) cam->target_offset_x = min_x;
}

void input_handle(SDL_Event *event, Camera *cam, int window_w, int window_h)
{
    (void)window_h;

    switch (event->type) {
    case SDL_EVENT_MOUSE_WHEEL: {
        float mouse_x = 0, mouse_y = 0;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        float wx = mouse_x / cam->target_zoom - cam->target_offset_x;

        float factor = (event->wheel.y > 0)
            ? (1.0f + ZOOM_SPEED)
            : (1.0f / (1.0f + ZOOM_SPEED));
        cam->target_zoom *= factor;
        if (cam->target_zoom < ZOOM_MIN) cam->target_zoom = ZOOM_MIN;
        if (cam->target_zoom > ZOOM_MAX) cam->target_zoom = ZOOM_MAX;

        cam->target_offset_x = mouse_x / cam->target_zoom - wx;
        clamp_camera(cam, window_w);
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event->button.button == SDL_BUTTON_LEFT)
            dragging = true;
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event->button.button == SDL_BUTTON_LEFT)
            dragging = false;
        break;

    case SDL_EVENT_MOUSE_MOTION:
        if (dragging) {
            cam->target_offset_x += event->motion.xrel / cam->zoom;
            cam->target_offset_y += event->motion.yrel / cam->zoom;
            clamp_camera(cam, window_w);
        }
        break;
    }
}
