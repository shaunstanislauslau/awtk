/**
 * File:   window_manager.h
 * Author: AWTK Develop Team
 * Brief:  native window manager
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

#ifndef TK_WINDOW_MANAGER_NATIVE_H
#define TK_WINDOW_MANAGER_NATIVE_H

#include "base/native_window.h"
#include "base/window_manager.h"

BEGIN_C_DECLS

/**
 * @class window_manager_native_t
 * @parent window_manager_t
 * @annotation ["scriptable"]
 * 用于管理native窗口的窗口管理器。
 *
 */
typedef struct _window_manager_native_t {
  window_manager_t window_manager;

  /*private*/
  /*一个可用的canvas，主要用于控件计算文本的大小，并不进行实际的绘制*/
  canvas_t* canvas;
  widget_t* prev_win;
  input_device_status_t input_device_status;
} window_manager_native_t;

/**
 * @method window_manager_create
 * 创建窗口管理器。
 * @annotation ["constructor"]
 *
 * @return {window_manager_t*} 返回窗口管理器对象。
 */
widget_t* window_manager_create(void);

#define WINDOW_MANAGER_NATIVE(widget) ((window_manager_native_t*)(widget))

END_C_DECLS

#endif /*TK_WINDOW_MANAGER_NATIVE_H*/
