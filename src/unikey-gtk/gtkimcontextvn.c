// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* GTK Unikey Vietnamese Input Method
 * Copyright (C) 2004 Pham Kim Long
 * Contact:
 *   longcz@yahoo.com
 *   UniKey project: http://unikey.sf.net
 * Contributor(s): Nguyen Thai Ngoc Duy (pclouds)
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

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xatom.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include "gtkimcontextvn.h"
#include "unikey.h"
#include "../xim/ukopt.h"
#include "../gui/xvnkb.h"
#include "../vnconv/vnconv.h"

#include "uksync.h"

static const guint16 gtk_compose_ignore[] = {
  GDK_Shift_L,
  GDK_Shift_R,
  GDK_Control_L,
  GDK_Control_R,
  GDK_Caps_Lock,
  GDK_Shift_Lock,
  GDK_Meta_L,
  GDK_Meta_R,
  GDK_Alt_L,
  GDK_Alt_R,
  GDK_Super_L,
  GDK_Super_R,
  GDK_Hyper_L,
  GDK_Hyper_R,
  GDK_Mode_switch
};

enum ShortcutNames {
  SC_SWITCH,
  SC_UNICODE_CHARSET,
  SC_VIQR_CHARSET,
  SC_TCVN_CHARSET,
  SC_VNI_CHARSET,
  SC_TELEX_INPUT,
  SC_VNI_INPUT,
  SC_VIQR_INPUT,
  SC_USER_INPUT,
  SC_RELOAD_CONFIG,
  SC_RESTORE_KEYS,
  SC_DISABLE_CHECK
};

typedef struct
{
  guint	keyval;
  guint	modifier;
  guint	modifier_mask;
  int scName;
} ShortcutInfo;

/* Shortcut List */
static ShortcutInfo ShortcutList[] = {
  {GDK_Shift_L, GDK_MOD1_MASK, GDK_MOD1_MASK, SC_SWITCH},
  {GDK_F1, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_UNICODE_CHARSET},
  {GDK_F2, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_VIQR_CHARSET},
  {GDK_F3, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_TCVN_CHARSET},
  {GDK_F4, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_VNI_CHARSET},
  {GDK_F5, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_TELEX_INPUT},
  {GDK_F6, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_VNI_INPUT},
  {GDK_F7, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_VIQR_INPUT},
  {GDK_F8, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_USER_INPUT},
  {GDK_F9, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_SWITCH},
  {GDK_Escape, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_RESTORE_KEYS},
  {GDK_Z, GDK_CONTROL_MASK|GDK_SHIFT_MASK, GDK_CONTROL_MASK|GDK_SHIFT_MASK, SC_DISABLE_CHECK}
  //  {GDK_Alt_L, GDK_CONTROL_MASK, GDK_CONTROL_MASK, SC_DISABLE_CHECK},
  //  {GDK_Alt_R, GDK_CONTROL_MASK, GDK_CONTROL_MASK, SC_DISABLE_CHECK}
};

static void     gtk_im_context_vn_class_init         (GtkIMContextVnClass  *class);
static void     gtk_im_context_vn_class_finalize         (GtkIMContextVnClass  *class);
static void     gtk_im_context_vn_init               (GtkIMContextVn       *im_context_vn);
static void     gtk_im_context_vn_finalize           (GObject                  *obj);
static gboolean gtk_im_context_vn_filter_key         (GtkIMContext             *context,
						      GdkEventKey              *key);
static void     gtk_im_context_vn_reset              (GtkIMContext             *context);

static void gtk_im_context_vn_focus_out (GtkIMContext   *context);
//static void     gtk_im_context_vn_set_cursor_location (GtkIMContext   *context,
//						 GdkRectangle   *area);

/*
static void     gtk_im_context_vn_get_preedit_string (GtkIMContext             *context,
							  gchar                   **str,
							  PangoAttrList           **attrs,
							  gint                     *cursor_pos);
*/
void restoreKeys(GtkIMContext *context, GdkEventKey  *event);

static gboolean ResetByKey = FALSE;
static gboolean InResetMode = FALSE;
static int PendingBuf = 0;
static int BackCount = 0;

GType gtk_type_im_context_vn = 0;

static GObjectClass *parent_class;

static void getBackspaceInfo();

static guint BackspaceKeycode;
static gint BackspaceGroup;
static gint BackspaceLevel;

static guint PauseKeycode;
static gint PauseGroup;
static gint PauseLevel;

static GdkEventKey KeyEvent;

//------------------------------------------
Atom AIMCharset, AIMUsing, AIMMethod;
Atom AGUIVisible;

static void getSyncAtoms(int xvnkbSync);
static void fixUnikeyToSyncMethod(int method);
static void fixSyncToUnikeyMethod();

static int UkGUIVisible = 0;

//------------------------------------------
UkXimOpt GlobalOpt;
char *ConfigFile = NULL;
char *MacroFile = NULL;

static void reloadConfig();
static int latinToUtf(unsigned char *dst, unsigned char *src, int inSize, int * pOutSize);

//------------------------------------------
static void getBackspaceInfo()
{
  GdkKeymapKey *keys;
  gint nkeys;

  gdk_keymap_get_entries_for_keyval (NULL, GDK_BackSpace, &keys, &nkeys);
  BackspaceKeycode = keys->keycode;
  BackspaceGroup = keys->group;
  BackspaceLevel = keys->level;

  g_free(keys);

  gdk_keymap_get_entries_for_keyval (NULL, GDK_Pause, &keys, &nkeys);
  PauseKeycode = keys->keycode;
  PauseGroup = keys->group;
  PauseLevel = keys->level;

  g_free(keys);
}

//------------------------------------------
static void createBackspaceEvent(GdkEventKey *ev, GdkWindow *window, guint32 time)
{
  ev->type = GDK_KEY_PRESS;
  ev->window = window;
  ev->send_event = 1;
  ev->time = time;
  ev->state = 0;
  ev->keyval = GDK_BackSpace;
  ev->length = 0;
  ev->string = NULL;
  ev->hardware_keycode = BackspaceKeycode;
  ev->group = BackspaceGroup;
}

//---------------------------------------
static void createPauseEvent(GdkEventKey *ev, GdkWindow *window, guint32 time)
{
  ev->type = GDK_KEY_PRESS;
  ev->window = window;
  ev->send_event = 1;
  ev->time = time;
  ev->state = 0;
  ev->keyval = GDK_Pause;
  ev->length = 0;
  ev->string = NULL;
  ev->hardware_keycode = PauseKeycode;
  ev->group = PauseGroup;
}

//---------------------------------------
void gtk_im_context_vn_register_type (GTypeModule *type_module)

{
  if (!gtk_type_im_context_vn)
    {
      static const GTypeInfo im_context_vn_info =
      {
        sizeof (GtkIMContextVnClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gtk_im_context_vn_class_init,
        (GClassFinalizeFunc) gtk_im_context_vn_class_finalize, //NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GtkIMContextVn),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gtk_im_context_vn_init,
      };

      gtk_type_im_context_vn =
	g_type_module_register_type (type_module,
				 GTK_TYPE_IM_CONTEXT,
				 "GtkIMContextVn",
				 &im_context_vn_info, 0);
    }
}

//-----------------------------------------
// based on code contributed by pclouds
//-----------------------------------------
static GdkFilterReturn
gtk_im_context_vn_event_filter (GdkXEvent *xevent,
				GdkEvent *event,
				gpointer data)
{
  XPropertyEvent *ev = (XPropertyEvent*)xevent;
  long v;
  GdkWindow *gdkroot = gdk_get_default_root_window();
  Window root = GDK_WINDOW_XID(gdkroot);

  if (!ev ||
      ev->type != PropertyNotify ||
      ev->window != root)
    return GDK_FILTER_CONTINUE;
  
  
  if (ev->atom == AIMCharset) {
    v = UkGetPropValue(ev->atom, VKC_UTF8);
    GlobalOpt.charset = SyncToUnikeyCharset(v);
    UnikeySetOutputCharset(GlobalOpt.charset);
    /*
    if (UkMacroLoaded)
      UkUpdateMacroTable(GlobalOpt.charset);
    */
    return GDK_FILTER_REMOVE;
  }
  else if (ev->atom == AIMMethod) {
    fixSyncToUnikeyMethod();
    if (GlobalOpt.enabled)
      UnikeySetInputMethod(GlobalOpt.inputMethod);
    return GDK_FILTER_REMOVE;
  }
  else if (ev->atom == AIMUsing) {
    //dont' need this
    return GDK_FILTER_REMOVE;
  }
  else if (ev->atom == AGUIVisible) {
    UkGUIVisible = UkGetPropValue(AGUIVisible, 0);
  }

  return GDK_FILTER_CONTINUE;
}

//-----------------------------------------
// contributed by pclouds
//-----------------------------------------
static void setEventFilter(int enable, GtkIMContextVnClass *class)
{
  static GdkEventMask oldMask;
  static int filterAdded = 0; 
  GdkEventMask mask;
  GdkWindow *root = gdk_get_default_root_window();

  if (enable && !filterAdded) {
    mask = gdk_window_get_events(root);
    oldMask = mask;
    mask |= GDK_PROPERTY_CHANGE_MASK;

    gdk_window_set_events(root,mask);

    //fprintf(stderr, "Registering...\n");
    /* register an event filter */
    gdk_window_add_filter(root,
			  gtk_im_context_vn_event_filter,
			  class);
    filterAdded = 1;
  }
  else if (!enable && filterAdded) {
    gdk_window_set_events(root,oldMask);
    //fprintf(stderr, "Unregistering...\n");
    ///* unregister event filter */
    gdk_window_remove_filter(root,
			  gtk_im_context_vn_event_filter,
			  class);
    filterAdded = 0;
  }

}

//-----------------------------------------
static void
gtk_im_context_vn_class_init (GtkIMContextVnClass *class)
{
  GdkWindow *gdkroot = gdk_get_default_root_window();
  Window root = GDK_WINDOW_XID(gdkroot);
  Display *display = GDK_WINDOW_XDISPLAY(gdkroot);

  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (class);
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  parent_class = g_type_class_peek_parent (class);

  im_context_class->filter_keypress = gtk_im_context_vn_filter_key;

  im_context_class->reset = gtk_im_context_vn_reset;
  im_context_class->focus_out = gtk_im_context_vn_focus_out;
  //  im_context_class->set_cursor_location = gtk_im_context_vn_set_cursor_location;

  gobject_class->finalize = gtk_im_context_vn_finalize;

  UnikeySetup();
  getBackspaceInfo();
  setEventFilter(1, class);

  UkInitSync(display, root);

  UkSetDefOptions(&GlobalOpt);
  ConfigFile = UkGetDefConfFileName();
  UkTestDefConfFile();

  /*
  ASuspend = XInternAtom(display, UKP_SUSPEND, False);
  UkSuspend = UkGetPropValue(ASuspend, 0);
  */

  AGUIVisible = XInternAtom(display, UKP_GUI_VISIBLE, False);
  UkGUIVisible = UkGetPropValue(AGUIVisible, 0);
  reloadConfig();
}

//---------------------------------------------
static void
gtk_im_context_vn_class_finalize (GtkIMContextVnClass *class)
{
  //  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (class);
  //  parent_class = g_type_class_peek_parent (class);
  //  parent_class->finalize(class);

  setEventFilter(0, class); //unregister filter
  UnikeyCleanup();
  if (GlobalOpt.macroFile) {
    free(GlobalOpt.macroFile);
    GlobalOpt.macroFile = 0;
  }
  //UkCleanupMacro();
}

//---------------------------------------------
static void
gtk_im_context_vn_init (GtkIMContextVn *im_context_vn)
{

}

//---------------------------------------------
static void
gtk_im_context_vn_finalize (GObject *obj)
{
  /* we'll do something later */
  parent_class->finalize (obj);
}

//---------------------------------------------
/**
 * gtk_im_context_simple_new:
 *
 * Creates a new #GtkIMContextSimple.
 *
 * Returns: a new #GtkIMContextSimple.
 **/
GtkIMContext *
gtk_im_context_vn_new (void)
{
  return g_object_new (GTK_TYPE_IM_CONTEXT_VN, NULL);
}

//---------------------------------------------
static void
gtk_im_context_vn_commit_char (GtkIMContext *context,
				   gunichar ch)
{
  gchar buf[10];
  gint len;

  g_return_if_fail (g_unichar_validate (ch));

  len = g_unichar_to_utf8 (ch, buf);
  buf[len] = '\0';

  g_signal_emit_by_name (context, "commit", &buf);
}

//-------------------------------------------
static void setOutputCharset(int charset)
{
  long v;
  if (UkGUIVisible) {
    v = UnikeyToSyncCharset(charset);
    UkSetPropValue(AIMCharset, v);
  }
  else {
    GlobalOpt.charset = charset;
    UnikeySetOutputCharset(charset);
  }
}

//-------------------------------------------
static void setInputMethod(UkInputMethod im)
{
  if (UkGUIVisible) {
    fixUnikeyToSyncMethod(im);
  }
  else {
    GlobalOpt.inputMethod = im;
    UnikeySetInputMethod(GlobalOpt.inputMethod);
  }
}

//-----------------------------------------------------------------
static gboolean checkSwitchKey(GtkIMContext *context, GdkEventKey *event)
{
  static int pendingSwitch = 0;
  if (event->type != GDK_KEY_PRESS && event->type != GDK_KEY_RELEASE)
    return FALSE;
  if (event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R ||
      event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) {
    if (event->type == GDK_KEY_RELEASE) {
      if (pendingSwitch) {
	pendingSwitch = 0;
	if (UkGUIVisible) {
	  if (GlobalOpt.enabled)
	    UkSetPropValue(AIMMethod, VKM_OFF);
	  else
	    fixUnikeyToSyncMethod(GlobalOpt.inputMethod);
	}
	else {
	  UnikeyResetBuf();
	  GlobalOpt.enabled = !GlobalOpt.enabled;
	}
	return TRUE;
      }
    }
    else {
      if (((event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R) && 
	   (event->state & GDK_CONTROL_MASK)) ||
	  ((event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) && 
	   (event->state & GDK_SHIFT_MASK))) {
	pendingSwitch = 1;
      }
    }
  }
  else pendingSwitch = 0;
  return FALSE;
}

//-----------------------------------------------------------------
static gboolean checkShortcuts(GtkIMContext *context, GdkEventKey *event)
{
  int i, count;
  guint modifier;
  guint modifier_mask;
  
  if (event->type != GDK_KEY_PRESS)
    return FALSE;

  count = sizeof(ShortcutList)/sizeof(ShortcutInfo);

  for (i = 0; i < count; i++) {
    modifier = ShortcutList[i].modifier;
    modifier_mask = ShortcutList[i].modifier_mask;
    if ((ShortcutList[i].keyval == event->keyval)
	&& ((event->state & modifier_mask) == modifier)){
      switch (ShortcutList[i].scName) {
      case SC_SWITCH:
	if (UkGUIVisible) {
	  if (GlobalOpt.enabled)
	    UkSetPropValue(AIMMethod, VKM_OFF);
	  else
	    fixUnikeyToSyncMethod(GlobalOpt.inputMethod);
	}
	else {
	  UnikeyResetBuf();
	  GlobalOpt.enabled = !GlobalOpt.enabled;
	}
	break;
      case SC_UNICODE_CHARSET:
	setOutputCharset(CONV_CHARSET_XUTF8);
	break;
      case SC_VIQR_CHARSET:
	setOutputCharset(CONV_CHARSET_VIQR);
	break;
      case SC_TCVN_CHARSET:
	setOutputCharset(CONV_CHARSET_TCVN3);
	break;
      case SC_VNI_CHARSET:
	setOutputCharset(CONV_CHARSET_VNIWIN);
	break;
      case SC_TELEX_INPUT:
	setInputMethod(UkTelex);
	break;
      case SC_VNI_INPUT:
	setInputMethod(UkVni);
	break;
      case SC_VIQR_INPUT:
	setInputMethod(UkViqr);
	break;
      case SC_USER_INPUT:
	setInputMethod(UkUsrIM);
	break;
      case SC_RELOAD_CONFIG:
	reloadConfig();
	break;
      case SC_RESTORE_KEYS:
	restoreKeys(context, event);
	break;
      case SC_DISABLE_CHECK:
	UnikeySetSingleMode();
	break;
      }
      return TRUE;
    }
  }
  return FALSE;
}


//---------------------------------------------
static void commitString(GtkIMContext *context)
{
  static char buf[1024];

  char *utfBuf;
  if (GlobalOpt.charset != CONV_CHARSET_XUTF8 || UnikeyOutput == UkKeyOutput) {
    int outLeft = sizeof(buf);
    if (!latinToUtf(buf, UnikeyBuf, UnikeyBufChars, &outLeft))
      return; //not enough memory
    buf[sizeof(buf)-outLeft] = 0;
    utfBuf = buf;
  }
  else {
    UnikeyBuf[UnikeyBufChars] = 0;
    utfBuf = UnikeyBuf;
  }
  g_signal_emit_by_name(context, "commit", utfBuf);
}

//---------------------------------------------
static gboolean
gtk_im_context_vn_filter_key (GtkIMContext *context,
				       GdkEventKey  *event)
{
  gunichar ch;
  unsigned char uch;

  int i;

  /*
  if (UkSuspend && !GlobalOpt.gtkImAlone) {
    printf("In suspend mode\n"); //DEBUG
    if (event->type == GDK_KEY_RELEASE)
      return FALSE;
    ch = gdk_keyval_to_unicode (event->keyval);
    if (ch != 0) {
      gtk_im_context_vn_commit_char (context, ch);
      return TRUE;
    }
    return FALSE;
  }
  */

  if (checkSwitchKey(context, event) || checkShortcuts(context, event)) {
    if (GlobalOpt.bellNotify)
      gdk_display_beep (gdk_drawable_get_display (event->window));
    return TRUE;
  }

  if (event->type == GDK_KEY_RELEASE)
    {
      return FALSE;
    }

  /* Ignore modifier key presses
   */
  for (i=0; i < G_N_ELEMENTS (gtk_compose_ignore); i++)
    if (event->keyval == gtk_compose_ignore[i])
      return FALSE;

  switch (event->keyval) {

  case GDK_Pause:
    if (PendingBuf) {
      //UnikeyAnsiBuf[UnikeyBufChars] = 0;
      //g_signal_emit_by_name(context, "commit", UnikeyAnsiBuf);
      commitString(context);
      PendingBuf = 0;
      InResetMode = FALSE;
      return TRUE;
    }
    return FALSE;

  case GDK_BackSpace:
    /*
    if (!InResetMode)
      ResetByKey = TRUE;
    */
    InResetMode = FALSE;
    ResetByKey = TRUE;
    if (BackCount > 0) {
      BackCount--;
      return FALSE;
    }

    UnikeyBackspacePress();
    if (UnikeyBackspaces > 1) {
      //      gtk_im_context_delete_surrounding(context, -UnikeyBackspaces, UnikeyBackspaces);
      BackCount = UnikeyBackspaces-1;
      createBackspaceEvent(&KeyEvent, event->window, event->time);
      for (i=0; i<BackCount; i++) {
	gdk_event_put((GdkEvent *)&KeyEvent);
	KeyEvent.time++;
      }
    }
    return FALSE;
  default:
    if (PendingBuf) {
      KeyEvent = *event;
      KeyEvent.time++;
      gdk_event_put((GdkEvent *)&KeyEvent);
      return TRUE;
    }
  }

  ch = gdk_keyval_to_unicode (event->keyval);
  if (!GlobalOpt.enabled) {
    if (ch != 0) {
      gtk_im_context_vn_commit_char (context, ch);
      return TRUE;
    }
    return FALSE;
  }

  if (ch == 0) {
    //    printf("Suspect\n");
    UnikeyResetBuf();
    return FALSE;
  }

  uch = (unsigned char)ch;
  UnikeySetCapsState(event->state & GDK_SHIFT_MASK, event->state & GDK_LOCK_MASK);

  if (event->keyval >= GDK_KP_0 && event->keyval <= GDK_KP_9) // don't convert keypad
    UnikeyPutChar(uch);
  else
    UnikeyFilter(uch);

  if (UnikeyBufChars == 0 && UnikeyBackspaces == 0) {
    InResetMode = FALSE;
    gtk_im_context_vn_commit_char (context, ch);
  }
  else {
    if (UnikeyBackspaces > 0) {
      //it would be nice if we can use gtk_im_contex_delete_surrounding
      //because in that case we don't have to generate fake backspace
      //But at the moment, it's unreliable, since it causes context to be reset
      // in an unmanageble way. So this code is commented out.

      //if (!gtk_im_context_delete_surrounding(context, -UnikeyBackspaces, UnikeyBackspaces)) {

      PendingBuf = 1;

      BackCount = UnikeyBackspaces;
      createBackspaceEvent(&KeyEvent, event->window, event->time+1);

      for (i=0; i<BackCount; i++) {
	gdk_event_put((GdkEvent *)&KeyEvent);
	KeyEvent.time++;
      }

      createPauseEvent(&KeyEvent, event->window, KeyEvent.time);
      gdk_event_put((GdkEvent *)&KeyEvent);

      return TRUE;
      //}
      //ResetByKey = TRUE;
    }
    //UnikeyAnsiBuf[UnikeyBufChars] = 0;
    //g_signal_emit_by_name(context, "commit", UnikeyAnsiBuf);
    commitString(context);
    InResetMode = FALSE;
  }
  return TRUE;
}

//-------------------------------------------------------
static void
gtk_im_context_vn_reset (GtkIMContext *context)
{
  if (!ResetByKey) {
    //    printf("reset buffer\n");
    UnikeyResetBuf();
  }
  ResetByKey = FALSE;
  InResetMode = TRUE;
}

//-------------------------------------------------------
static void gtk_im_context_vn_focus_out (GtkIMContext *context)
{
  UnikeyResetBuf();
}


//-------------------------------------------------------
static void getSyncAtoms(int xvnkbSync)
{
  GdkWindow *gdkroot = gdk_get_default_root_window();
  Display *display = GDK_WINDOW_XDISPLAY(gdkroot);

  long v;
  
  if (xvnkbSync) {
    AIMCharset = XInternAtom(display, VKP_CHARSET, False);
    AIMMethod = XInternAtom(display, VKP_METHOD, False);
    AIMUsing = XInternAtom(display, VKP_USING, False);
  }
  else {
    AIMCharset = XInternAtom(display, UKP_CHARSET, False);
    AIMMethod = XInternAtom(display, UKP_METHOD, False);
    AIMUsing = XInternAtom(display, UKP_USING, False);
  }
  
  if (UkGUIVisible) {
    v = UkGetPropValue(AIMCharset, VKC_UTF8);
    GlobalOpt.charset = SyncToUnikeyCharset((int)v);

    v = UkGetPropValue(AIMMethod, VKM_TELEX);
    GlobalOpt.enabled = (v != VKM_OFF);

    if (!GlobalOpt.enabled)
      v = UkGetPropValue(AIMUsing, VKM_TELEX);

    GlobalOpt.inputMethod = SyncToUnikeyMethod((int)v);
  }
}

//--------------------------------------------------
// this is a work-around of the difference between
// xvnkb and unikey in using viqr method
//--------------------------------------------------
static void fixUnikeyToSyncMethod(int method)
{
  long v;

  v = UnikeyToSyncMethod(method);
  UkSetPropValue(AIMMethod, v);
  UkSetPropValue(AIMUsing, v);
}

//--------------------------------------------------
// this is a work-around of the difference between
// xvnkb and unikey in using viqr method
//--------------------------------------------------
static void fixSyncToUnikeyMethod()
{
  long v;

  v = UkGetPropValue(AIMMethod, VKM_TELEX);
  GlobalOpt.enabled = (v != VKM_OFF);
  UnikeyResetBuf();

  if (GlobalOpt.enabled) {
    GlobalOpt.inputMethod = SyncToUnikeyMethod((int)v);
  }
}

//----------------------------------------------------
static void reloadConfig(int useSyncProp)
{
  char *fname;
  UnikeyGetOptions(&GlobalOpt.uk);
  if (GlobalOpt.macroFile) {
    free(GlobalOpt.macroFile);
    GlobalOpt.macroFile = 0;
  }

  if (ConfigFile != NULL) {
    UkParseOptFile(ConfigFile, &GlobalOpt);
  }

  fname = 0;
  if (MacroFile)
    fname = MacroFile;
  else if (GlobalOpt.macroFile)
    fname = GlobalOpt.macroFile;

  //UkCleanupMacro();
  if (fname) {
    if (UnikeyLoadMacroTable(fname)) {
      //fputs("\nMacro file loaded!\n", stderr);
      //UkSetupMacro();
      //UkMacroLoaded = 1;
      GlobalOpt.uk.macroEnabled = 1;
    }
    else {
      //UkMacroLoaded = 0;
      GlobalOpt.uk.macroEnabled = 0;
      fprintf(stderr, "\nFailed to load macro file: %s!\n", fname);
    }
  }
  else {
    //UkMacroLoaded = 0;
    GlobalOpt.uk.macroEnabled = 0;
    //fprintf(stderr, "No macrofile specified\n");
  }

  if (GlobalOpt.usrKeyMapFile)
    UnikeyLoadUserKeyMap(GlobalOpt.usrKeyMapFile);
  
  UnikeySetOptions(&GlobalOpt.uk);

  //set sync properties
  getSyncAtoms(GlobalOpt.xvnkbSync);
  UnikeySetOutputCharset(GlobalOpt.charset);
  UnikeySetInputMethod(GlobalOpt.inputMethod);
}

//----------------------------------------------------
int latinToUtf(unsigned char *dst, unsigned char *src, int inSize, int * pOutSize)
{
  int i;
  int outLeft;
  unsigned char ch;

  outLeft = *pOutSize;

  for (i=0; i<inSize; i++) {
    ch = *src++;
    if (ch < 0x80) {
      outLeft -= 1;
      if (outLeft >= 0)
	*dst++ = ch;
    } else {
      outLeft -= 2;
      if (outLeft >= 0) {
	*dst++ = (0xC0 | ch >> 6);
	*dst++ = (0x80 | (ch & 0x3F));
      }
    } 
  }

  *pOutSize = outLeft;
  return (outLeft >= 0);
}

/*
//---------------------------------------------
static int readContext(GtkIMContext *context)
{
  gchar *buf;
  gint *index;

  if (!gtk_im_context_get_surrounding(context, &buf, &index))
    return 0;

  
  g_free(buf);
}
*/
/*
int VnConvert(int inCharset, int outCharset, BYTE *input, BYTE *output, int & inLen, int & maxOutLen);
*/

//-------------------------------------------------
void restoreKeys(GtkIMContext *context, GdkEventKey  *event)
{
  int i;
  if (!GlobalOpt.enabled)
    return;

  UnikeyRestoreKeyStrokes();
  if (UnikeyBackspaces > 0) {
    if (!gtk_im_context_delete_surrounding(context, -UnikeyBackspaces, UnikeyBackspaces)) {
      PendingBuf = 1;
    
      BackCount = UnikeyBackspaces;
      createBackspaceEvent(&KeyEvent, event->window, event->time+1);
      
      for (i=0; i<BackCount; i++) {
	gdk_event_put((GdkEvent *)&KeyEvent);
	KeyEvent.time++;
      }
      
      createPauseEvent(&KeyEvent, event->window, KeyEvent.time);
      gdk_event_put((GdkEvent *)&KeyEvent);

      return;
    }
  }

  if (UnikeyBufChars > 0)
    commitString(context);
  InResetMode = FALSE;
}
