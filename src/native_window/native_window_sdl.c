/**
 * File:   native_window_sdl.h
 * Author: AWTK Develop Team
 * Brief:  native window sdl
 *
 * Copyright (c) 2019 - 2019  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-07-21 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include <SDL.h>
#include "lcd/lcd_sdl2.h"
#include "lcd/lcd_nanovg.h"
#include "base/native_window.h"

typedef struct _native_window_sdl_t {
  native_window_t native_window;
  SDL_Renderer* render;
  SDL_Window* window;
  canvas_t canvas;
} native_window_sdl_t;

static native_window_t* s_shared_win = NULL;

#define NATIVE_WINDOW_SDL(win) ((native_window_sdl_t*)(win))

static ret_t native_window_sdl_move(native_window_t* win, xy_t x, xy_t y) {
  int oldx = 0;
  int oldy = 0;
  native_window_sdl_t* sdl = NATIVE_WINDOW_SDL(win);

  win->x = x;
  win->y = y;
  SDL_GetWindowPosition(sdl->window, &oldx, &oldy);
  if(oldx != x || oldy != y) {
    SDL_SetWindowPosition(sdl->window, x, y);
  }

  return RET_OK;
}

static ret_t native_window_sdl_resize(native_window_t* win, wh_t w, wh_t h) {
  int oldw = 0;
  int oldh = 0;
  native_window_sdl_t* sdl = NATIVE_WINDOW_SDL(win);

  win->w = w;
  win->h = h;
  SDL_GetWindowSize(sdl->window, &oldw, &oldh);
  if(w != oldw || h != oldh) {
    SDL_SetWindowSize(sdl->window, w, h);
  }

  return RET_OK;
}

static ret_t native_window_sdl_close(native_window_t* win) {
  native_window_sdl_t* sdl = NATIVE_WINDOW_SDL(win);

  if (sdl->render != NULL) {
    SDL_DestroyRenderer(sdl->render);
  }

  if (sdl->window != NULL) {
    SDL_DestroyWindow(sdl->window);
  }

  sdl->render = NULL;
  sdl->window = NULL;

  return RET_OK;
}

static canvas_t* native_window_sdl_get_canvas(native_window_t* win) {
  native_window_sdl_t* sdl = NATIVE_WINDOW_SDL(win);

  return &(sdl->canvas);
}

static const native_window_vtable_t s_native_window_vtable = {
    .type = "native_window_sdl",
    .move = native_window_sdl_move,
    .resize = native_window_sdl_resize,
    .get_canvas = native_window_sdl_get_canvas};

static ret_t native_window_sdl_set_prop(object_t* obj, const char* name, const value_t* v) {
  return RET_NOT_FOUND;
}

static ret_t native_window_sdl_get_prop(object_t* obj, const char* name, value_t* v) {
  return RET_NOT_FOUND;
}

static ret_t native_window_sdl_on_destroy(object_t* obj) {
  log_debug("Close native window.\n");
  native_window_sdl_close(NATIVE_WINDOW(obj));

  return RET_OK;
}

static const object_vtable_t s_native_window_sdl_vtable = {
    .type = "native_window_sdl",
    .desc = "native_window_sdl",
    .size = sizeof(native_window_sdl_t),
    .get_prop = native_window_sdl_get_prop,
    .set_prop = native_window_sdl_set_prop,
    .on_destroy = native_window_sdl_on_destroy};

static native_window_t* native_window_create_internal(const char* title, int32_t x, int32_t y,
                                                      uint32_t w, uint32_t h) {
  lcd_t* lcd = NULL;
  uint32_t flags = 0;
  object_t* obj = object_create(&s_native_window_sdl_vtable);
  native_window_t* win = NATIVE_WINDOW(obj);
  native_window_sdl_t* sdl = NATIVE_WINDOW_SDL(win);
  canvas_t* c = &(sdl->canvas);

  if (system_info()->app_type == APP_DESKTOP) {
    flags |= SDL_WINDOW_RESIZABLE;
  }

#ifndef WITH_NANOVG_SOFT
  flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
#endif /*WITH_NANOVG_SOFT*/
  sdl->window = SDL_CreateWindow(title, x, y, w, h, flags);
#ifdef WITH_NANOVG_SOFT
  sdl->render =
      SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
#endif /*WITH_NANOVG_SOFT*/

  win->x = x;
  win->y = y;
  win->w = w;
  win->h = h;
  win->handle = sdl->window;
  win->vt = &s_native_window_vtable;

#ifdef WITH_NANOVG_SOFT
  lcd = lcd_sdl2_init(sdl->render);
#else
  lcd = lcd_nanovg_init(sdl->window);
#endif /*WITH_NANOVG_SOFT*/

  canvas_init(c, lcd, font_manager());

  return win;
}

native_window_t* native_window_create(widget_t* widget) {
  int32_t x = widget->x;
  int32_t y = widget->y;
  int32_t w = widget->w;
  int32_t h = widget->h;

  if (s_shared_win != NULL) {
    object_ref(OBJECT(s_shared_win));

    return s_shared_win;
  } else {
    return native_window_create_internal("", x, y, w, h);
  }
}

#ifdef WITH_NANOVG_GL
#include "glad/glad.h"
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

static ret_t sdl_init_gl(void) {
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

#ifdef WITH_NANOVG_GL2
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(WITH_NANOVG_GL3)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

  log_debug("Init opengl done.\n");
  return RET_OK;
}
#endif /*WITH_NANOVG_GL*/

ret_t native_window_sdl_init(bool_t shared, uint32_t w, uint32_t h) {
  const char* title = system_info()->app_name;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    log_debug("Failed to initialize SDL: %s", SDL_GetError());
    exit(0);
    return RET_FAIL;
  }

#ifdef WITH_NANOVG_GL
  sdl_init_gl();
#endif /*WITH_NANOVG_GL*/

  SDL_StopTextInput();
  if (shared) {
    int32_t x = SDL_WINDOWPOS_UNDEFINED;
    int32_t y = SDL_WINDOWPOS_UNDEFINED;
    s_shared_win = native_window_create_internal(title, x, y, w, h);
  }

  return RET_OK;
}

ret_t native_window_sdl_deinit(void) {
  if (s_shared_win != NULL) {
    object_unref(OBJECT(s_shared_win));
    s_shared_win = NULL;
  }

  SDL_Quit();

  return RET_OK;
}
