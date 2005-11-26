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

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include "nls.h"
#include "gtk/gtkimmodule.h"
#include "gtkimcontextvn.h"
#include "keycons.h"
#include "unikey.h"

static const GtkIMContextInfo im_vn_info = {
  "unikey",		           /* ID */
  N_("Vietnamese Input Method (UniKey)"),            /* Human readable name */
  GETTEXT_PACKAGE,		   /* Translation domain */
  GTK_LOCALEDIR,		   /* Dir for bindtextdomain (not strictly needed for "gtk+") */
  "vi"		           /* Languages for which this module is the default */
};

static const GtkIMContextInfo *info_list[] = {
  &im_vn_info
};

void
im_module_init (GTypeModule *type_module)
{
  gtk_im_context_vn_register_type (type_module);
  //  UnikeySetup();
  // setUserOptions();
}

void
im_module_exit (void)
{
  //  gtk_im_context_xim_shutdown ();
}

void
im_module_list (const GtkIMContextInfo ***contexts,
		int                      *n_contexts)
{
  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS (info_list);
}

GtkIMContext *
im_module_create (const gchar *context_id)
{
  if (strcmp (context_id, "unikey") == 0)
    return gtk_im_context_vn_new ();
  else
    return NULL;
}

