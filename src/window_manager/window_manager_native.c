/**
 * File:   window_manager_default.c
 * Author: AWTK Develop Team
 * Brief:  default window manager
 *
 * Copyright (c) 2018 - 2019  Guangzhou ZHIYUAN Electronics Co.,Ltd.
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
 * 2018-01-13 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/mem.h"
#include "base/keys.h"
#include "base/idle.h"
#include "tkc/utils.h"
#include "base/timer.h"
#include "base/layout.h"
#include "tkc/time_now.h"
#include "widgets/dialog.h"
#include "base/locale_info.h"
#include "base/system_info.h"
#include "base/image_manager.h"
#include "base/dialog_highlighter_factory.h"
#include "window_manager/window_manager_native.h"

static native_window_t* window_get_native_window(widget_t* win) {
  return_value_if_fail(win != NULL, NULL);

  return (native_window_t*)widget_get_prop_pointer(win, WIDGET_PROP_NATIVE_WINDOW);
}

static ret_t window_manager_dispatch_window_event(widget_t* window, event_type_t type) {
  window_event_t evt;
  event_t e = event_init(type, window);
  widget_dispatch(window, &e);

  evt.window = window;
  evt.e = event_init(type, window->parent);

  return widget_dispatch(window->parent, (event_t*)&(evt));
}

static ret_t window_manager_dispatch_window_open(widget_t* curr_win) {
  window_manager_dispatch_window_event(curr_win, EVT_WINDOW_WILL_OPEN);
  window_manager_dispatch_window_event(curr_win, EVT_WINDOW_OPEN);

  return RET_OK;
}

static ret_t window_manager_create_native_window(widget_t* widget) {
  native_window_t* nw = NULL;

  if(tk_str_eq(widget_get_type(widget), WIDGET_TYPE_NORMAL_WINDOW)) {
    nw = native_window_create(widget);
  } else {
    widget_t* prev =  window_manager_get_prev_window(widget->parent);
    nw = (native_window_t*)widget_get_prop_pointer(prev, WIDGET_PROP_NATIVE_WINDOW);
    object_ref(OBJECT(nw));
  }
  return_value_if_fail(nw != NULL, RET_BAD_PARAMS);

  widget_set_prop_pointer(widget, WIDGET_PROP_NATIVE_WINDOW, nw);

  return RET_OK;
}

static ret_t window_manager_idle_dispatch_window_open(const idle_info_t* info) {
  widget_t* window = WIDGET(info->ctx);
  return_value_if_fail(window != NULL, RET_REMOVE);

  if(window->w <= 0) {
    window->w = window->parent->w;
  }
  
  if(window->h <= 0) {
    window->h = window->parent->h;
  }

  widget_layout(window);
  window_manager_create_native_window(window);
  window_manager_dispatch_window_open(window);

  return RET_REMOVE;
}

static ret_t window_manager_native_open_window(widget_t* widget, widget_t* window) {
  return_value_if_fail(widget != NULL && window != NULL, RET_BAD_PARAMS);

  widget_add_child(widget, window);
  widget_add_idle(window, (idle_func_t)window_manager_idle_dispatch_window_open);

  return RET_OK;
}

static ret_t window_manager_idle_destroy_window(const idle_info_t* info) {
  native_window_t* nw = NULL; 
  widget_t* win = WIDGET(info->ctx);
  
  nw = (native_window_t*)widget_get_prop_pointer(win, WIDGET_PROP_NATIVE_WINDOW);
  if(nw != NULL) {
    object_unref(OBJECT(nw));
  }

  widget_destroy(win);

  return RET_OK;
}

static ret_t window_manager_native_close_window_force(widget_t* widget, widget_t* window) {
  widget_remove_child(widget, window);
  idle_add(window_manager_idle_destroy_window, window);
  return RET_OK;
}

static ret_t window_manager_native_close_window(widget_t* widget, widget_t* window) {

  return window_manager_close_window_force(widget, window);
}

static widget_t* window_manager_find_target_by_win(widget_t* widget, void* win) {
  native_window_t* nw = NULL; 
  return_value_if_fail(widget != NULL, NULL);

  WIDGET_FOR_EACH_CHILD_BEGIN_R(widget, iter, i)
  nw = (native_window_t*)widget_get_prop_pointer(iter, WIDGET_PROP_NATIVE_WINDOW);
  if(nw->handle == win) {
    return iter;
  }
  WIDGET_FOR_EACH_CHILD_END()

  return NULL;
}

static widget_t* window_manager_find_target(widget_t* widget, void* win, xy_t x, xy_t y) {
  point_t p = {x, y};
  native_window_t* nw = NULL; 
  return_value_if_fail(widget != NULL, NULL);

  if (widget->grab_widget != NULL) {
    return widget->grab_widget;
  }

  widget_to_local(widget, &p);
  WIDGET_FOR_EACH_CHILD_BEGIN_R(widget, iter, i)
  xy_t r = iter->x + iter->w;
  xy_t b = iter->y + iter->h;

  nw = (native_window_t*)widget_get_prop_pointer(iter, WIDGET_PROP_NATIVE_WINDOW);
  if(nw == NULL || nw->handle != win) {
    continue;
  }

  if (iter->visible && iter->sensitive && iter->enable && p.x >= iter->x && p.y >= iter->y &&
      p.x <= r && p.y <= b) {
    return iter;
  }

  if (widget_is_dialog(iter) || widget_is_popup(iter)) {
    return iter;
  }
  WIDGET_FOR_EACH_CHILD_END()

  return NULL;
}


static widget_t* window_manager_native_get_prev_window(widget_t* widget) {
  window_manager_native_t* wm = WINDOW_MANAGER_NATIVE(widget);

  return wm->prev_win;
}

static ret_t window_manager_paint_child(widget_t* widget, widget_t* child) {
  rect_t* dr = NULL;
  canvas_t* canvas = NULL; 
  native_window_t* nw = NULL; 
  window_manager_native_t* wm = WINDOW_MANAGER_NATIVE(widget);
  nw = (native_window_t*)widget_get_prop_pointer(child, WIDGET_PROP_NATIVE_WINDOW);
  
  dr = &(nw->dirty_rect);
  if(dr->w > 0 && dr->h) {
    rect_t r = native_window_calc_dirty_rect(nw);
    if(r.w > 0 && r.h > 0) {
      canvas_t* c = native_window_get_canvas(nw);
      widget_paint(child, c);
      native_window_update_last_dirty_rect(nw);
    }
  }

  return RET_OK;
}

static ret_t window_manager_native_paint(widget_t* widget) {
  rect_t r;
  native_window_t* nw = NULL;
  window_manager_native_t* wm = WINDOW_MANAGER_NATIVE(widget);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);
  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  if (iter->visible) {
    nw = (native_window_t*)widget_get_prop_pointer(iter, WIDGET_PROP_NATIVE_WINDOW);
    
    r = native_window_calc_dirty_rect(nw);
    canvas_begin_frame(native_window_get_canvas(nw), &r, LCD_DRAW_NORMAL);
  }
  WIDGET_FOR_EACH_CHILD_END()

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  if (iter->visible) {
    window_manager_paint_child(widget, iter);
  }
  WIDGET_FOR_EACH_CHILD_END()
  
  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  if (iter->visible) {
    nw = (native_window_t*)widget_get_prop_pointer(iter, WIDGET_PROP_NATIVE_WINDOW);
    canvas_end_frame(native_window_get_canvas(nw));
  }
  WIDGET_FOR_EACH_CHILD_END()

  return RET_OK;
}

static ret_t window_manager_on_remove_child(widget_t* widget, widget_t* window) {
  return RET_FAIL;
}

static ret_t window_manager_get_prop(widget_t* widget, const char* name, value_t* v) {
  return RET_NOT_FOUND;
}

static ret_t window_manager_set_prop(widget_t* widget, const char* name, const value_t* v) {
  return RET_NOT_FOUND;
}

static ret_t window_manager_on_destroy(widget_t* widget) {
  return RET_OK;
}

static ret_t window_manager_on_layout_children(widget_t* widget) {
  return RET_OK;
}

static ret_t window_manager_native_resize(widget_t* widget, wh_t w, wh_t h);
static ret_t window_manager_native_post_init(widget_t* widget, wh_t w, wh_t h);
static ret_t window_manager_native_set_cursor(widget_t* widget, const char* cursor);
static ret_t window_manager_native_set_show_fps(widget_t* widget, bool_t show_fps);
static ret_t window_manager_native_dispatch_input_event(widget_t* widget, event_t* e);
static ret_t window_manager_native_set_screen_saver_time(widget_t* widget,
                                                          uint32_t screen_saver_time);

static ret_t window_manager_native_get_pointer(widget_t* widget, xy_t* x, xy_t* y,
                                                bool_t* pressed) {
  window_manager_native_t* wm = WINDOW_MANAGER_NATIVE(widget);
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (x != NULL) {
    *x = wm->input_device_status.last_x;
  }
  if (y != NULL) {
    *y = wm->input_device_status.last_y;
  }
  if (pressed != NULL) {
    *pressed = wm->input_device_status.pressed;
  }

  return RET_OK;
}

static ret_t window_manager_on_event(widget_t* widget, event_t* e) {
  return RET_OK;
}

static ret_t window_manager_native_resize(widget_t* widget, wh_t w, wh_t h) {
  window_manager_native_t* wm = WINDOW_MANAGER_NATIVE(widget);
  return_value_if_fail(wm != NULL, RET_BAD_PARAMS);

  widget_move_resize(widget, 0, 0, w, h);

  return widget_layout_children(widget);
}

static ret_t window_manager_native_post_init(widget_t* widget, wh_t w, wh_t h) {
  window_manager_native_t* wm = WINDOW_MANAGER_NATIVE(widget);
  return_value_if_fail(wm != NULL, RET_BAD_PARAMS);

  window_manager_native_resize(widget, w, h);

  return RET_OK;
}

static ret_t window_manager_native_dispatch_input_event(widget_t* widget, event_t* e) {
  widget_t* target = NULL;
  input_device_status_t* ids = NULL;
  window_manager_native_t* wm = WINDOW_MANAGER_NATIVE(widget);
  return_value_if_fail(wm != NULL && e != NULL, RET_BAD_PARAMS);

  ids = &(wm->input_device_status);
  if (wm->ignore_user_input) {
    if (ids->pressed && e->type == EVT_POINTER_UP) {
      log_debug("animating ignore input, but it is last pointer_up\n");
    } else {
      log_debug("animating ignore input\n");
      return RET_OK;
    }
  }

  if(e->type == EVT_POINTER_DOWN
      || e->type == EVT_POINTER_MOVE
      || e->type == EVT_POINTER_UP) {
    pointer_event_t* evt = pointer_event_cast(e);
    target = window_manager_find_target(widget, e->native_window_handle, evt->x, evt->y);
  } else {
    target = window_manager_find_target_by_win(widget, e->native_window_handle);
  }
  input_device_status_on_input_event(ids, target, e);

  if(e->type == EVT_POINTER_DOWN) {
    wm->prev_win = target;
  }

  return RET_OK;
}

static ret_t window_manager_native_set_show_fps(widget_t* widget, bool_t show_fps) {
  return RET_OK;
}

static ret_t window_manager_native_set_screen_saver_time(widget_t* widget,
                                                          uint32_t screen_saver_time) {
  return RET_OK;
}

static ret_t window_manager_native_set_cursor(widget_t* widget, const char* cursor) {
  return RET_OK;
}

static window_manager_vtable_t s_window_manager_self_vtable = {
    .paint = window_manager_native_paint,
    .resize = window_manager_native_resize,
    .post_init = window_manager_native_post_init,
    .set_cursor = window_manager_native_set_cursor,
    .open_window = window_manager_native_open_window,
    .get_pointer = window_manager_native_get_pointer,
    .close_window = window_manager_native_close_window,
    .set_show_fps = window_manager_native_set_show_fps,
    .get_prev_window = window_manager_native_get_prev_window,
    .close_window_force = window_manager_native_close_window_force,
    .dispatch_input_event = window_manager_native_dispatch_input_event,
    .set_screen_saver_time = window_manager_native_set_screen_saver_time};


static const widget_vtable_t s_window_manager_vtable = {
    .size = sizeof(window_manager_t),
    .is_window_manager = TRUE,
    .type = WIDGET_TYPE_WINDOW_MANAGER,
    .set_prop = window_manager_set_prop,
    .get_prop = window_manager_get_prop,
    .on_event = window_manager_on_event,
    .on_layout_children = window_manager_on_layout_children,
    .on_remove_child = window_manager_on_remove_child,
    .on_destroy = window_manager_on_destroy};

widget_t* window_manager_create(void) {
  window_manager_native_t* wm = TKMEM_ZALLOC(window_manager_native_t);
  return_value_if_fail(wm != NULL, NULL);

  return window_manager_init(WINDOW_MANAGER(wm),
      &s_window_manager_vtable, &s_window_manager_self_vtable);
}

