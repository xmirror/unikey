// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* GTK Unikey Vietnamese Input Method
 * Copyright (C) 2004 Pham Kim Long
 * Contact:
 *   longcz@yahoo.com
 *   UniKey project: http://unikey.sf.net
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GTK_IM_CONTEXT_VN_H__
#define __GTK_IM_CONTEXT_VN_H__

#include <gtk/gtkimcontext.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_IM_CONTEXT_VN              gtk_type_im_context_vn
#define GTK_IM_CONTEXT_VN(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_IM_CONTEXT_VN, GtkIMContextVn))
#define GTK_IM_CONTEXT_VN_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_IM_CONTEXT_VN, GtkIMContextVnClass))
#define GTK_IS_IM_CONTEXT_VN(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_IM_CONTEXT_VN))
#define GTK_IS_IM_CONTEXT_VN_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_IM_CONTEXT_VN))
#define GTK_IM_CONTEXT_VN_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_IM_CONTEXT_VN, GtkIMContextVnClass))


typedef struct _GtkIMContextVn       GtkIMContextVn;
typedef struct _GtkIMContextVnClass  GtkIMContextVnClass;

#define GTK_MAX_COMPOSE_LEN 7
  
struct _GtkIMContextVn
{
  GtkIMContext object;
};

struct _GtkIMContextVnClass
{
  GtkIMContextClass parent_class;
};

void gtk_im_context_vn_register_type (GTypeModule *type_module);
GtkIMContext *gtk_im_context_vn_new       (void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_IM_CONTEXT_VN_H__ */
