/*
  Written by Claude Opus 5.5, an AI model by Anthropic.

  Released under Public Domain. Where a dedication to the public domain is
  not recognized, this file is licensed under CC0 1.0 Universal
  (https://creativecommons.org/publicdomain/zero/1.0/).
*/

/* Grow a window drawn with the GPU renderer; with GPU debug validation, no draw may exceed the render target. */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static const SDL_Point sizes[] = {
    { 640, 480 },
    { 800, 600 },
    { 1000, 700 },
    { 1200, 800 },
};

static void draw_frame(SDL_Renderer *renderer)
{
    const SDL_FRect square = { 20.0f, 20.0f, 100.0f, 100.0f };

    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 200, 80, 60, 255);
    SDL_RenderFillRect(renderer, &square);
    SDL_RenderPresent(renderer);
}

static unsigned int assertions_triggered(void)
{
    const SDL_AssertData *item;
    unsigned int total = 0;

    for (item = SDL_GetAssertionReport(); item; item = item->next) {
        total += item->trigger_count;
    }
    return total;
}

int main(int argc, char **argv)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    int grown_steps = 0;
    unsigned int triggered;
    int i;

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "gpu");
    SDL_SetHint(SDL_HINT_RENDER_GPU_DEBUG, "1");
    SDL_SetHintWithPriority(SDL_HINT_ASSERT, "ignore", SDL_HINT_OVERRIDE);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Skipping: couldn't initialize video: %s", SDL_GetError());
        return 0;
    }
    if (!SDL_CreateWindowAndRenderer("testgpurender_resize", sizes[0].x, sizes[0].y, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Skipping: couldn't create a window with the GPU renderer: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    draw_frame(renderer);
    for (i = 1; i < (int)SDL_arraysize(sizes); ++i) {
        int before_w = 0, before_h = 0, after_w = 0, after_h = 0;

        SDL_GetWindowSizeInPixels(window, &before_w, &before_h);
        SDL_SetWindowSize(window, sizes[i].x, sizes[i].y);
        SDL_SyncWindow(window);
        SDL_PumpEvents();
        SDL_GetWindowSizeInPixels(window, &after_w, &after_h);
        if (after_w > before_w || after_h > before_h) {
            ++grown_steps;
        } else {
            SDL_Log("Window did not grow past %dx%d pixels", before_w, before_h);
        }
        draw_frame(renderer);
    }

    triggered = assertions_triggered();
    SDL_Log("Window grew %d of %d times; assertions triggered: %u", grown_steps, (int)SDL_arraysize(sizes) - 1, triggered);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    if (triggered) {
        return 1;
    }
    if (grown_steps == 0) {
        SDL_Log("Skipping: the window could not be grown, so nothing was tested");
    }
    return 0;
}
