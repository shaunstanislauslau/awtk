/**
 * File:   window_manager.c
 * Author: AWTK Develop Team
 * Brief:  window manager
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

#include "base/widget.h"
#include "base/canvas.h"
#include "base/dialog_highlighter.h"
#include "base/input_device_status.h"
#include "base/window_manager.h"
#include "base/window_animator_factory.h"

static widget_t* s_window_manager = NULL;

widget_t* window_manager(void) {
  return s_window_manager;
}

ret_t window_manager_set(widget_t* widget) {
  s_window_manager = widget;

  return RET_OK;
}

widget_t* window_manager_cast(widget_t* widget) {
  return_value_if_fail(widget != NULL, NULL);

  return widget;
}

widget_t* window_manager_get_top_main_window(widget_t* widget) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, NULL);
  return_value_if_fail(wm->vt->get_top_main_window != NULL, NULL);

  return wm->vt->get_top_main_window(widget);
}

widget_t* window_manager_get_top_window(widget_t* widget) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, NULL);
  return_value_if_fail(wm->vt->get_top_window != NULL, NULL);

  return wm->vt->get_top_window(widget);
}

widget_t* window_manager_get_prev_window(widget_t* widget) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, NULL);
  return_value_if_fail(wm->vt->get_prev_window != NULL, NULL);

  return wm->vt->get_prev_window(widget);
}

ret_t window_manager_resize(widget_t* widget, wh_t w, wh_t h) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->resize != NULL, RET_BAD_PARAMS);

  return wm->vt->resize(widget, w, h);
}

ret_t window_manager_open_window(widget_t* widget, widget_t* window) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(window != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->open_window != NULL, RET_BAD_PARAMS);

  return wm->vt->open_window(widget, window);
}

ret_t window_manager_close_window(widget_t* widget, widget_t* window) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(window != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->close_window != NULL, RET_BAD_PARAMS);

  return wm->vt->close_window(widget, window);
}

ret_t window_manager_close_window_force(widget_t* widget, widget_t* window) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(window != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->close_window_force != NULL, RET_BAD_PARAMS);

  return wm->vt->close_window_force(widget, window);
}

ret_t window_manager_paint(widget_t* widget, canvas_t* c) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->paint != NULL, RET_BAD_PARAMS);

  return wm->vt->paint(widget, c);
}

ret_t window_manager_dispatch_input_event(widget_t* widget, event_t* e) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(e != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->dispatch_input_event != NULL, RET_BAD_PARAMS);

  return wm->vt->dispatch_input_event(widget, e);
}

ret_t window_manager_set_show_fps(widget_t* widget, bool_t show_fps) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->set_show_fps != NULL, RET_BAD_PARAMS);

  wm->show_fps = show_fps;
  return wm->vt->set_show_fps(widget, show_fps);
}

ret_t window_manager_set_screen_saver_time(widget_t* widget, uint32_t time) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->set_screen_saver_time != NULL, RET_BAD_PARAMS);

  return wm->vt->set_screen_saver_time(widget, time);
}

ret_t window_manager_set_cursor(widget_t* widget, const char* cursor) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->set_cursor != NULL, RET_BAD_PARAMS);

  return wm->vt->set_cursor(widget, cursor);
}

ret_t window_manager_back(widget_t* widget) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->back != NULL, RET_BAD_PARAMS);

  return wm->vt->back(widget);
}
ret_t window_manager_back_to_home(widget_t* widget) {
  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, RET_BAD_PARAMS);
  return_value_if_fail(wm->vt->back_to_home != NULL, RET_BAD_PARAMS);

  return wm->vt->back_to_home(widget);
}

xy_t window_manager_get_pointer_x(widget_t* widget) {
  xy_t x = 0;
  xy_t y = 0;
  bool_t pressed = TRUE;

  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, 0);
  return_value_if_fail(wm->vt->get_pointer != NULL, 0);

  wm->vt->get_pointer(widget, &x, &y, &pressed);

  return x;
}

xy_t window_manager_get_pointer_y(widget_t* widget) {
  xy_t x = 0;
  xy_t y = 0;
  bool_t pressed = TRUE;

  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, 0);
  return_value_if_fail(wm->vt->get_pointer != NULL, 0);

  wm->vt->get_pointer(widget, &x, &y, &pressed);

  return y;
}

bool_t window_manager_get_pointer_pressed(widget_t* widget) {
  xy_t x = 0;
  xy_t y = 0;
  bool_t pressed = TRUE;

  window_manager_t* wm = WINDOW_MANAGER(widget);
  return_value_if_fail(wm != NULL && wm->vt != NULL, 0);
  return_value_if_fail(wm->vt->get_pointer != NULL, 0);

  wm->vt->get_pointer(widget, &x, &y, &pressed);

  return pressed;
}
