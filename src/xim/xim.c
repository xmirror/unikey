// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/******************************************************************
 
         Copyright 1994, 1995 by Sun Microsystems, Inc.
         Copyright 1993, 1994 by Hewlett-Packard Company
 
Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and Hewlett-Packard not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and Hewlett-Packard make no representations about
the suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
 
SUN MICROSYSTEMS INC. AND HEWLETT-PACKARD COMPANY DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
SUN MICROSYSTEMS, INC. AND HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 
  Author: Hidetoshi Tajima(tajima@Eng.Sun.COM) Sun Microsystems, Inc.
 
******************************************************************/

/* Unikey Vietnamese Input Method
 * Copyright (C) 2004-2005 Pham Kim Long
 * Contact:
 *   longcz@yahoo.com
 *   UniKey project: http://unikey.sf.net
 *
 * This program is based on Sample program included in IMdkit
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

#include <stdio.h>
//#include <unistd.h>
#include <signal.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "IMdkit.h"
#include "Xi18n.h"
#include "IC.h"
#include "vnconv.h"
#include "unikey.h"
#include "uksync.h"
#include "../gui/xvnkb.h"
//#include "macro.h"
#include "ukopt.h"

#define DEFAULT_IMNAME "unikey"
#define PROG_NAME "Unikey XIM"

//#define DEBUG

#define COMMIT_KEYSYM XK_F12

//-----------------------------------------------------------------
Display *display;
Window RootWindow;
Atom ProgAtom;
XIMS UnikeyIms;

Window MainWindow = None;
Atom AIMCharset, AIMUsing, AIMMethod;
Atom AGUIVisible;
Atom AGUIPosX, AGUIPosY;

static unsigned int BsKeycode;
//static unsigned int PauseKeycode;
static unsigned int CommitKeycode;
static unsigned int ReturnKeycode;

static char ReturnChar;

static Bool PendingCommit = False;
static Bool PostponeKeyEv = False;
static int PostponeCount = 0;
static Bool DataCommit = False;
static Bool UkTriggering = False;

void getSyncAtoms(Bool xvnkbSync);
void setRootPropMask();
void resetState();
Bool singleLaunch();
void fixUnikeyToSyncMethod(UkInputMethod method);
void fixSyncToUnikeyMethod();

//-----------------------------------------------------------------
UkXimOpt GlobalOpt;

int UkLoopContinue = 1;
int UkWatchGUI = 0;
int UkGUIVisible = 0;
char *ConfigFile = NULL;
char *MacroFile = NULL;
char *XimLocales = NULL; //locales option from command line
char *DefaultXimLocales = "C,en_US,vi_VN,fr_FR,fr_BE,fr_CA,de_DE,ja_JP,cs_CZ,ru_RU";

//int AllocMacforFile = 0;

void reloadConfig(int signum);
void restoreKeys(XIMS ims, IMForwardEventStruct *call_data);
void signalSetup();
int saveOptFile();
void cleanup();

//-----------------------------------------------------------------
int myXErrorHandler(Display *dpy, XErrorEvent *event);

/* flags for debugging */
Bool use_tcp = False;		/* Using TCP/IP Transport or not */
Bool use_local = False;		/* Using Unix domain Tranport or not */
long filter_mask = KeyPressMask | KeyReleaseMask;


/* Supported Inputstyles */
static XIMStyle Styles[] = {
    //  XIMPreeditCallbacks|XIMStatusCallbacks,
    //XIMPreeditPosition|XIMStatusArea,
    //XIMPreeditPosition|XIMStatusNothing,
    //XIMPreeditArea|XIMStatusArea,
    XIMPreeditNothing|XIMStatusNothing,
    0
};

/* Trigger Keys List */
static XIMTriggerKey Trigger_Keys[] = {
    {XK_A, 0, ControlMask | Mod1Mask},
    {XK_a, 0, ControlMask | Mod1Mask},
    {XK_B, 0, ControlMask | Mod1Mask},
    {XK_b, 0, ControlMask | Mod1Mask},
    {XK_C, 0, ControlMask | Mod1Mask},
    {XK_c, 0, ControlMask | Mod1Mask},
    {XK_D, 0, ControlMask | Mod1Mask},
    {XK_d, 0, ControlMask | Mod1Mask},
    {XK_E, 0, ControlMask | Mod1Mask},
    {XK_e, 0, ControlMask | Mod1Mask},
    {XK_F, 0, ControlMask | Mod1Mask},
    {XK_f, 0, ControlMask | Mod1Mask},
    {XK_G, 0, ControlMask | Mod1Mask},
    {XK_g, 0, ControlMask | Mod1Mask},
    {XK_H, 0, ControlMask | Mod1Mask},
    {XK_h, 0, ControlMask | Mod1Mask},
    {XK_I, 0, ControlMask | Mod1Mask},
    {XK_i, 0, ControlMask | Mod1Mask},
    {XK_J, 0, ControlMask | Mod1Mask},
    {XK_j, 0, ControlMask | Mod1Mask},
    {XK_K, 0, ControlMask | Mod1Mask},
    {XK_k, 0, ControlMask | Mod1Mask},
    {XK_L, 0, ControlMask | Mod1Mask},
    {XK_l, 0, ControlMask | Mod1Mask},
    {XK_M, 0, ControlMask | Mod1Mask},
    {XK_m, 0, ControlMask | Mod1Mask},
    {XK_N, 0, ControlMask | Mod1Mask},
    {XK_n, 0, ControlMask | Mod1Mask},
    {XK_O, 0, ControlMask | Mod1Mask},
    {XK_o, 0, ControlMask | Mod1Mask},
    {XK_P, 0, ControlMask | Mod1Mask},
    {XK_p, 0, ControlMask | Mod1Mask},
    {XK_Q, 0, ControlMask | Mod1Mask},
    {XK_q, 0, ControlMask | Mod1Mask},
    {XK_R, 0, ControlMask | Mod1Mask},
    {XK_r, 0, ControlMask | Mod1Mask},
    {XK_S, 0, ControlMask | Mod1Mask},
    {XK_s, 0, ControlMask | Mod1Mask},
    {XK_T, 0, ControlMask | Mod1Mask},
    {XK_t, 0, ControlMask | Mod1Mask},
    {XK_U, 0, ControlMask | Mod1Mask},
    {XK_u, 0, ControlMask | Mod1Mask},
    {XK_V, 0, ControlMask | Mod1Mask},
    {XK_v, 0, ControlMask | Mod1Mask},
    {XK_W, 0, ControlMask | Mod1Mask},
    {XK_w, 0, ControlMask | Mod1Mask},
    {XK_X, 0, ControlMask | Mod1Mask},
    {XK_x, 0, ControlMask | Mod1Mask},
    {XK_Y, 0, ControlMask | Mod1Mask},
    {XK_y, 0, ControlMask | Mod1Mask},
    {XK_Z, 0, ControlMask | Mod1Mask},
    {XK_z, 0, ControlMask | Mod1Mask},
    //  {XK_Shift_L, 0, ControlMask},
    //  {XK_Shift_R, 0, ControlMask},
    {XK_Shift_L, ControlMask, ControlMask},
    {XK_Shift_R, ControlMask, ControlMask},
    //  {XK_Shift_L, 0, Mod1Mask},
    //  {XK_Shift_R, 0, Mod1Mask},
    {XK_Shift_L, Mod1Mask, Mod1Mask},
    {XK_Shift_R, Mod1Mask, Mod1Mask},
    {0L, 0L, 0L}
};

/* Forward Keys List */
static XIMTriggerKey Forward_Keys[] = {
    {XK_Return, 0, 0},
    {XK_Tab, 0, 0},
    {XK_Delete, 0, 0},
    {0L, 0L, 0L}
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
    SC_RELOAD_CONFIG,
    SC_RESTORE_KEYS,
    SC_DISABLE_CHECK,
    SC_USER_INPUT
};

typedef struct
{
    CARD32	keysym;
    CARD32	modifier;
    CARD32	modifier_mask;
    int scName;
} ShortcutInfo;

/* Shortcut List */
static ShortcutInfo ShortcutList[] = {
    //{XK_Shift_L, Mod1Mask, Mod1Mask, SC_SWITCH},
    //{XK_Shift_R, Mod1Mask, Mod1Mask, SC_SWITCH},
    {XK_F1, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_UNICODE_CHARSET},
    {XK_F2, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_VIQR_CHARSET},
    {XK_F3, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_TCVN_CHARSET},
    {XK_F4, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_VNI_CHARSET},
    {XK_F5, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_TELEX_INPUT},
    {XK_F6, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_VNI_INPUT},
    {XK_F7, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_VIQR_INPUT},
    {XK_F8, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_USER_INPUT},
    {XK_F9, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_SWITCH},
    {XK_Escape, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_RESTORE_KEYS},
    {XK_Z, ControlMask|ShiftMask, ControlMask|ShiftMask, SC_DISABLE_CHECK}
};

/*
static ShortcutInfo ShortcutList[] = {
  {XK_Shift_L, ControlMask, ControlMask, SC_SWITCH},
  {XK_F1, Mod1Mask|ShiftMask, Mod1Mask|ShiftMask, SC_UNICODE_CHARSET},
  {XK_F2, Mod1Mask|ShiftMask, Mod1Mask|ShiftMask, SC_VIQR_CHARSET},
  {XK_F3, Mod1Mask|ShiftMask, Mod1Mask|ShiftMask, SC_TCVN_CHARSET},
  {XK_F4, Mod1Mask|ShiftMask, Mod1Mask|ShiftMask, SC_VNI_CHARSET},
  {XK_F5, Mod1Mask|ShiftMask, Mod1Mask|ShiftMask, SC_TELEX_INPUT},
  {XK_F6, Mod1Mask|ShiftMask, Mod1Mask|ShiftMask, SC_VNI_INPUT},
  {XK_F7, Mod1Mask|ShiftMask, Mod1Mask|ShiftMask, SC_VIQR_INPUT}
  //  {XK_F9, Mod1Mask|ShiftMask, Mod1Mask|ShiftMask, SC_RELOAD_CONFIG}
};
*/

static XIMEncoding SupportedEncodings[] = {
    "COMPOUND_TEXT",
    NULL
};
//-----------------------------------------------------------------
// helper functions
#define STRBUFLEN 64

int latinToUtf(char *dst, char *src, int inSize, int * pOutSize);

//-----------------------------------------------------------------
Bool MyGetICValuesHandler(XIMS ims, IMChangeICStruct *call_data)
{
    GetIC(call_data);
    return True;
}

//-----------------------------------------------------------------
Bool MySetICValuesHandler(XIMS ims, IMChangeICStruct *call_data)
{
#ifdef DEBUG
    printf("Set IC Values\n");
#endif
    SetIC(call_data);
    return True;
}

//-----------------------------------------------------------------
Bool MyOpenHandler(XIMS ims, IMOpenStruct *call_data)
{
#ifdef DEBUG
    printf("new_client lang = %s\n", call_data->lang.name);
    printf("     connect_id = 0x%x\n", (int)call_data->connect_id);
#endif
    return True;
}

//-----------------------------------------------------------------
Bool MyCloseHandler(XIMS ims, IMOpenStruct *call_data)
{
#ifdef DEBUG
    printf("closing connect_id 0x%x\n", (int)call_data->connect_id);
#endif
    return True;
}

//-----------------------------------------------------------------
Bool MyCreateICHandler(XIMS ims, IMChangeICStruct *call_data)
{
#ifdef DEBUG
    printf("Create IC\n");
#endif
    CreateIC(call_data);
    return True;
}

//-----------------------------------------------------------------
Bool MyDestroyICHandler(XIMS ims, IMChangeICStruct *call_data)
{
#ifdef DEBUG
    printf("Destroy IC\n");
#endif
    DestroyIC(call_data);
    return True;
}

//-----------------------------------------------------------------
Bool IsKey(XIMS ims, IMForwardEventStruct *call_data, XIMTriggerKey *trigger)
{
    char strbuf[STRBUFLEN];
    KeySym keysym;
    int i;
    int modifier;
    int modifier_mask;
    XKeyEvent *kev;

    memset(strbuf, 0, STRBUFLEN);
    kev = (XKeyEvent*)&call_data->event;
    XLookupString(kev, strbuf, STRBUFLEN, &keysym, NULL);

    for (i = 0; trigger[i].keysym != 0; i++) {
	modifier      = trigger[i].modifier;
	modifier_mask = trigger[i].modifier_mask;
	if (((KeySym)trigger[i].keysym == keysym)
	    && ((kev->state & modifier_mask) == modifier))
        return True;
    }
    return False;
}

//-----------------------------------------------------------------
Bool isSwitchKey(XIMS ims, IMForwardEventStruct *call_data)
{
    char strbuf[STRBUFLEN];
    KeySym keysym;
    static int pendingSwitch = 0;
    
    XKeyEvent *kev = (XKeyEvent*)&call_data->event;
    if (kev->type != KeyPress && kev->type != KeyRelease)
        return False;
    
    memset(strbuf, 0, STRBUFLEN);

    XLookupString(kev, strbuf, STRBUFLEN, &keysym, NULL);

    if (keysym == XK_Shift_L || keysym == XK_Shift_R ||
        keysym == XK_Control_L || keysym == XK_Control_R) {
        if (kev->type == KeyRelease ) {
            if (pendingSwitch) {
                pendingSwitch = 0;
                if (GlobalOpt.enabled)
                    UkSetPropValue(AIMMethod, VKM_OFF);
                else
                    fixUnikeyToSyncMethod(GlobalOpt.inputMethod);
                return True;
            }
        }
        else {
            if (((keysym == XK_Shift_L || keysym == XK_Shift_R) && (kev->state & ControlMask)) ||
                ((keysym == XK_Control_L || keysym == XK_Control_R) && (kev->state & ShiftMask))) {
                pendingSwitch = 1;
            }
        }
    }
    else pendingSwitch = 0;
    return False;
}

//-----------------------------------------------------------------
Bool isShortcut(XIMS ims, IMForwardEventStruct *call_data)
{
    char strbuf[STRBUFLEN];
    KeySym keysym;
    int i, count;
    int modifier;
    int modifier_mask;
    long v;
    int bellAction = 1;
  
    XKeyEvent *kev;

    if (call_data->event.type != KeyPress)
        return False;
    
    memset(strbuf, 0, STRBUFLEN);
    kev = (XKeyEvent*)&call_data->event;
    XLookupString(kev, strbuf, STRBUFLEN, &keysym, NULL);
    
    count = sizeof(ShortcutList)/sizeof(ShortcutInfo);
    
    for (i = 0; i < count; i++) {
        modifier = ShortcutList[i].modifier;
        modifier_mask = ShortcutList[i].modifier_mask;
        if (((KeySym)ShortcutList[i].keysym == keysym)
            && ((kev->state & modifier_mask) == modifier)){
            switch (ShortcutList[i].scName) {
            case SC_SWITCH:
                if (GlobalOpt.enabled)
                    UkSetPropValue(AIMMethod, VKM_OFF);
                else
                    fixUnikeyToSyncMethod(GlobalOpt.inputMethod);
                break;
            case SC_UNICODE_CHARSET:
                v = UnikeyToSyncCharset(CONV_CHARSET_XUTF8);
                UkSetPropValue(AIMCharset, v);
                break;
            case SC_VIQR_CHARSET:
                v = UnikeyToSyncCharset(CONV_CHARSET_VIQR);
                UkSetPropValue(AIMCharset, v);
                break;
            case SC_TCVN_CHARSET:
                v = UnikeyToSyncCharset(CONV_CHARSET_TCVN3);
                UkSetPropValue(AIMCharset, v);
                break;
            case SC_VNI_CHARSET:
                v = UnikeyToSyncCharset(CONV_CHARSET_VNIWIN);
                UkSetPropValue(AIMCharset, v);
                break;
            case SC_TELEX_INPUT:
                fixUnikeyToSyncMethod(UkTelex);
                break;
            case SC_VNI_INPUT:
                fixUnikeyToSyncMethod(UkVni);
                break;
            case SC_VIQR_INPUT:
                fixUnikeyToSyncMethod(UkViqr);
                break;
      case SC_USER_INPUT:
          fixUnikeyToSyncMethod(UkUsrIM);
          break;
            case SC_RELOAD_CONFIG:
                reloadConfig(SIGUSR1);
                break;
            case SC_RESTORE_KEYS:
                restoreKeys(ims, call_data);
                bellAction = 0;
                break;
            case SC_DISABLE_CHECK:
                UnikeySetSingleMode();
                bellAction = 0;
                break;
            }

            if (bellAction && GlobalOpt.bellNotify)
                XBell(ims->core.display, 0);
            
            return True;
        }
    }
    return False;
}

//-----------------------------------------------
void forwardBackspaces(XIMS ims, IMForwardEventStruct *call_data, int times) 
{
    XKeyEvent *fwdKev;
    IMForwardEventStruct fwdEv;
    int i;
    
    fwdEv = *((IMForwardEventStruct *)call_data);
    fwdKev = (XKeyEvent *)&fwdEv.event;
    
    fwdKev->keycode = BsKeycode;
    
    for (i=0; i<UnikeyBackspaces; i++) {
        fwdKev->serial++;
        fwdKev->time++;
        fwdEv.serial_number++;
        IMForwardEvent(ims, (XPointer)&fwdEv);
    }
}

//-----------------------------------------------
void sendBackspaces(XIMS ims, IMForwardEventStruct *call_data, int times) 
{
    XKeyEvent *fwdKev;
    IMForwardEventStruct fwdEv;
    int i;
    
    fwdEv = *((IMForwardEventStruct *)call_data);
    fwdKev = (XKeyEvent *)&fwdEv.event;
    
    fwdKev->keycode = BsKeycode;
    for (i=0; i<UnikeyBackspaces; i++) {
        fwdKev->serial++;
        fwdKev->time++;
        fwdKev->type = KeyPress;
        XSendEvent(display, fwdKev->window, False, KeyPressMask, (XEvent *)fwdKev);
        /*
          fwdKev->serial++;
          fwdKev->time++;
          fwdKev->type = KeyRelease;
          XSendEvent(display, fwdKev->window, False, KeyReleaseMask, (XEvent *)fwdKev);
        */
  }

}

//-----------------------------------------------
void sendCommit(XEvent *srcEv) 
{
    XKeyEvent ev;
    ev = *((XKeyEvent *)srcEv);
    
    ev.serial++;
    ev.time++;
    ev.keycode = CommitKeycode;
    ev.type = KeyPress;
    XSendEvent(display, ev.window, False, KeyPressMask, (XEvent *)&ev);
    
    ev.type = KeyRelease;
    ev.serial++;
    ev.time++;
    XSendEvent(display, ev.window, False, KeyReleaseMask, (XEvent *)&ev);
    
    //  XSync(display, False);
}

//-----------------------------------------------
void commitString(XIMS ims, IMForwardEventStruct *call_data, const char *str)
{
    IMCommitStruct commitInfo;
    
    *((IMAnyStruct *)&commitInfo) = *((IMAnyStruct *)call_data);
    commitInfo.icid = call_data->icid;
    commitInfo.flag = XimLookupChars;
    commitInfo.commit_string = (char *)str;
    IMCommitString(ims, (XPointer)&commitInfo);
}

//-----------------------------------------------
void commitBuf(XIMS ims, IMForwardEventStruct *call_data)
{
    XTextProperty tp;
    Display *dpy;
    
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
    
    dpy = ims->core.display;
    Xutf8TextListToTextProperty(dpy, 
                                (char **)&utfBuf, 1,
                                XCompoundTextStyle, 
                                &tp);
    commitString(ims, call_data, (const char *)tp.value);
    
    /*
     *((IMAnyStruct *)&commitInfo) = *((IMAnyStruct *)call_data);
     commitInfo.icid = call_data->icid;
     commitInfo.flag = XimLookupChars;
     commitInfo.commit_string = (char *)tp.value;
     IMCommitString(ims, (XPointer)&commitInfo);
    */

    XFree(tp.value); 
}

//-----------------------------------------------
void ProcessKey(XIMS ims, IMForwardEventStruct *call_data)
{
    char strbuf[STRBUFLEN];
    KeySym keysym;
    XKeyEvent *kev;
    IMForwardEventStruct forward_ev;
    int count;

    forward_ev = *((IMForwardEventStruct *)call_data);
    memset(strbuf, 0, STRBUFLEN);
    kev = (XKeyEvent*)&call_data->event;
    count = XLookupString(kev, strbuf, STRBUFLEN, &keysym, NULL);

    switch (keysym) {
    case XK_BackSpace:
#ifdef DEBUG
        fprintf(stderr, "Backspace received\n");
#endif
        if (!PendingCommit) {
            UnikeyBackspacePress();
            if (UnikeyBackspaces > 0) {
                if (GlobalOpt.commitMethod == UkForwardCommit) {
                    forwardBackspaces(ims, call_data, UnikeyBackspaces);
                    if (UnikeyBufChars > 0)
                        commitBuf(ims, call_data);
                }
                else {
                    sendBackspaces(ims, call_data, UnikeyBackspaces);
                    sendCommit(&call_data->event);
                    PendingCommit = True;
                    PostponeKeyEv = True;
                    PostponeCount = 0;
                    DataCommit = (UnikeyBufChars > 0);
                }
                XSync(display, False);
                return;
            }
            if (UnikeyBufChars > 0) {
                commitBuf(ims, call_data);
                XSync(display, False);
                return;
            }
        }
        else {
#ifdef DEBUG
            fprintf(stderr, "Fake backspace\n");
#endif
        }
        IMForwardEvent(ims, (XPointer)&forward_ev);
        return;

    case COMMIT_KEYSYM:
#ifdef DEBUG
        fprintf(stderr, "pause received\n");
#endif
        if (PendingCommit) {
            PendingCommit = False;
            if (DataCommit) {
                commitBuf(ims, call_data);
                sendCommit(&call_data->event);
                //XSync(display, False);
                DataCommit = False;
            }
            else {
                sendCommit(&call_data->event);
                //XSync(display, False);
            }
#ifdef DEBUG
            fprintf(stderr, "commited\n");
#endif
            return;
        }
        PostponeKeyEv = False;
        return; //break;
    }
    
    if (count > 0 && (kev->state & (ControlMask | Mod1Mask)) == 0) {
        if (PostponeKeyEv && PostponeCount < 50) {
            //postpone this event
            PostponeCount++;
            //printf("OOPS OUT OF ORDER\n");//DEBUG
            XSendEvent(display, kev->window, False, KeyPressMask, (XEvent *)kev);
            return;
        }
        if (count == 1) {
            //      fprintf(stderr, "ch: %c %d\n", strbuf[0], strbuf[0]);
            UnikeySetCapsState(kev->state & ShiftMask, kev->state & LockMask);
            
            //if (keysym >= XK_KP_0 && keysym <= XK_KP_9)
            if (IsKeypadKey(keysym))  // don't convert keypad
                UnikeyPutChar((unsigned char)strbuf[0]);
            else {
                UnikeyFilter((unsigned char)strbuf[0]);
                if (UnikeyBackspaces > 0) {
                    if (GlobalOpt.commitMethod == UkForwardCommit) {
                        forwardBackspaces(ims, call_data, UnikeyBackspaces);
                        if (UnikeyBufChars > 0)
                            commitBuf(ims, call_data);
                    }
                    else {
                        sendBackspaces(ims, call_data, UnikeyBackspaces);
                        sendCommit(&call_data->event);
                        PendingCommit = True;
                        PostponeKeyEv = True;
                        PostponeCount = 0;
                        DataCommit = (UnikeyBufChars > 0);
                    }
                    //XSync(display, False);
                    return;
                }

                if (UnikeyBufChars > 0) {
                    commitBuf(ims, call_data);
                    //XSync(display, False);
                    return;
                }  
            }
            if (UkTriggering) {
                strbuf[1] = 0;
                commitString(ims, call_data, strbuf);
                //XSync(display, False);
                return;
            }
        }
        else {
            //fprintf(stderr, "count != 1. About to reset state\n");
            resetState();
        }
    }
    else {
        if (keysym != XK_Shift_L && keysym != XK_Shift_R &&
            keysym != XK_Control_L && keysym != XK_Control_R &&
            keysym != XK_Meta_L && keysym != XK_Meta_R && 
            keysym != XK_Alt_L && keysym != XK_Alt_R &&
            // I don't know why with some GTK application we receive keysym == 0
            // after commiting buffer. So we need to avoid reseting state for this case
            keysym != 0) 
        {
            //fprintf(stderr, "Key sym checking: %d. About to reset state\n", keysym);
            resetState();
        }
    }
    IMForwardEvent(ims, (XPointer)&forward_ev);
}

//-----------------------------------------------
Bool MyForwardEventHandler(XIMS ims, IMForwardEventStruct *call_data)
{
    IMForwardEventStruct forward_ev;

    if (UkWatchGUI && !UkGUIVisible)
        goto forwardEv;

    if (isSwitchKey(ims, call_data) || isShortcut(ims, call_data))
        return True;

    /* Lookup KeyPress Events only */
    if (call_data->event.type != KeyPress)
        goto forwardEv;
  
    if (!GlobalOpt.enabled)
        goto forwardEv;

    if (IsKey(ims, call_data, Forward_Keys)) {
        //fprintf(stderr, "IsKey. About to reset state\n");
        resetState();
        goto forwardEv;
    } else {
        ProcessKey(ims, call_data);
    }
    return True;
 forwardEv:
    forward_ev = *((IMForwardEventStruct *)call_data);
    IMForwardEvent(ims, (XPointer)&forward_ev);
    return True;
}

//-----------------------------------------------
Bool MyTriggerNotifyHandler(XIMS ims, IMTriggerNotifyStruct *call_data)
{
    //  GlobalOpt.enabled = !GlobalOpt.enabled;
    // XBell(ims->core.display, 0);
    IC *ic = FindIC(call_data->icid);
    int keyIndex = call_data->key_index / 3;

    if (ic) {
        IMForwardEventStruct imEv;
        imEv.major_code = call_data->major_code;
        imEv.minor_code = call_data->minor_code;
        imEv.connect_id = call_data->connect_id;
        imEv.icid = call_data->icid;
        imEv.sync_bit = 1;
        //set x event
        imEv.event.type = KeyPress;
        imEv.event.xkey.display = display;
        imEv.event.xkey.window = (ic->focus_win)? ic->focus_win : ic->client_win;
        imEv.event.xkey.root = RootWindow;
        imEv.event.xkey.subwindow = None;
        imEv.event.xkey.time = CurrentTime;
        if (keyIndex < 52) //if trigger key is a letter key
            imEv.event.xkey.state = ((keyIndex % 2) == 0) ? ShiftMask : 0;
        else {
            imEv.event.xkey.state = Trigger_Keys[keyIndex].modifier;
        }
        imEv.event.xkey.keycode = XKeysymToKeycode(display, Trigger_Keys[keyIndex].keysym);
        imEv.event.xkey.same_screen = True;

        if ((!UkWatchGUI || UkGUIVisible) && GlobalOpt.enabled) {
            //let this key go through normal processing
            UkTriggering = True;
            MyForwardEventHandler(ims, &imEv);
            UkTriggering = False;
        }
        else {
            char strbuf[STRBUFLEN];
            KeySym keysym;
            int count;

            if (!UkWatchGUI || UkGUIVisible) {
                if (isShortcut(ims, &imEv))
                    return True;
            }
            
            //commit this key
            memset(strbuf, 0, STRBUFLEN);
            count = XLookupString(&imEv.event.xkey, strbuf, STRBUFLEN, &keysym, NULL);
            if (count > 0) {
                strbuf[count] = 0;
                commitString(ims, &imEv, strbuf);
            }
        }
    }
    return True;
}

//-----------------------------------------------
Bool MyPreeditStartReplyHandler(XIMS ims, IMPreeditCBStruct *call_data)
{
    return True;
}

//-----------------------------------------------
Bool MyPreeditCaretReplyHandler(XIMS ims, IMPreeditCBStruct *call_data)
{
    return True;
}

//-----------------------------------------------
Bool MyProtoHandler(XIMS ims, IMProtocol *call_data)
{
    switch (call_data->major_code) {
    case XIM_OPEN:
        //fprintf(stderr, "XIM_OPEN:\n");
        return MyOpenHandler(ims, (IMOpenStruct *)call_data);
    case XIM_CLOSE:
        //fprintf(stderr, "XIM_CLOSE:\n");
        return MyCloseHandler(ims, (IMOpenStruct *)call_data);
    case XIM_CREATE_IC:
        //fprintf(stderr, "XIM_CREATE_IC:\n");
        return MyCreateICHandler(ims, (IMChangeICStruct *)call_data);
    case XIM_DESTROY_IC:
        //fprintf(stderr, "XIM_DESTROY_IC.\n");
        return MyDestroyICHandler(ims, (IMChangeICStruct *)call_data);
    case XIM_SET_IC_VALUES:
        //fprintf(stderr, "XIM_SET_IC_VALUES:\n");
        return MySetICValuesHandler(ims, (IMChangeICStruct *)call_data);
    case XIM_GET_IC_VALUES:
        //fprintf(stderr, "XIM_GET_IC_VALUES:\n");
        return MyGetICValuesHandler(ims, (IMChangeICStruct *)call_data);
    case XIM_FORWARD_EVENT:
        //fprintf(stderr, "XIM_FORWARD_EVENT\n");
        return MyForwardEventHandler(ims, (IMForwardEventStruct *)call_data);
    case XIM_SET_IC_FOCUS:
        //fprintf(stderr, "XIM_SET_IC_FOCUS()\n");
        return True;
    case XIM_UNSET_IC_FOCUS:
        //fprintf(stderr, "XIM_UNSET_IC_FOCUS:\n");
        resetState();
        return True;
    case XIM_RESET_IC:
        //fprintf(stderr, "XIM_RESET_IC_FOCUS:\n");
        return True;
    case XIM_TRIGGER_NOTIFY:
        //fprintf(stderr, "XIM_TRIGGER_NOTIFY:\n");
        return MyTriggerNotifyHandler(ims, (IMTriggerNotifyStruct *)call_data);
    case XIM_PREEDIT_START_REPLY:
        //fprintf(stderr, "XIM_PREEDIT_START_REPLY:\n");
        return MyPreeditStartReplyHandler(ims, (IMPreeditCBStruct *)call_data);
    case XIM_PREEDIT_CARET_REPLY:
        //fprintf(stderr, "XIM_PREEDIT_CARET_REPLY:\n");
        return MyPreeditCaretReplyHandler(ims, (IMPreeditCBStruct *)call_data);
    default:
        //fprintf(stderr, "Unknown IMDKit Protocol message type\n");
        break;
    }
    return False;
}


//------------------------------------------------------
void handlePropertyChanged(Window win, XEvent *event)
{
    long v;

    XPropertyEvent *pev = (XPropertyEvent *)event;

    if (pev->atom == AIMCharset) {
        v = UkGetPropValue(pev->atom, VKC_UTF8);
        GlobalOpt.charset = SyncToUnikeyCharset(v);
        UnikeySetOutputCharset(GlobalOpt.charset);
        //    if (UkMacroLoaded)
        //      UkUpdateMacroTable(GlobalOpt.charset);
    }
    else if (pev->atom == AIMMethod) {
        fixSyncToUnikeyMethod();
        if (GlobalOpt.enabled)
            UnikeySetInputMethod(GlobalOpt.inputMethod);
    }
    else if (pev->atom == AIMUsing) {
        //dont' need this
    }
    else if (pev->atom == AGUIPosX) {
        GlobalOpt.posX = UkGetPropValue(AGUIPosX, -1);
    }
    else if (pev->atom == AGUIPosY) {
        GlobalOpt.posY = UkGetPropValue(AGUIPosY, -1);
    }
    else if (pev->atom == AGUIVisible) {
        UkGUIVisible = UkGetPropValue(AGUIVisible, 0);
        //fprintf(stderr, "Property. About to reset state\n");
        resetState();
        if (!UkGUIVisible) {
            if (GlobalOpt.autoSave)
                saveOptFile();
        }
        else {
            reloadConfig(SIGUSR1);
        }
    }
}

//------------------------------------------------------
void MyXEventHandler(Window im_window, XEvent *event)
{
    switch (event->type) {
    case DestroyNotify:
        break;
    case ButtonPress:
        switch (event->xbutton.button) {
        case Button1:
            {
                XButtonEvent *bev = (XButtonEvent *)event;
                if ((bev->state & ControlMask) && (bev->state & Mod1Mask))
                    UkLoopContinue = 0;
            }
            break;
        }
        break;

    case PropertyNotify:
        if (event->xproperty.window == RootWindow)
            handlePropertyChanged(im_window, event);
        break;
        /*
          case ButtonRelease: //this event occurs in IC focus window
          fprintf(stderr, "Button Release event...\n");
          resetState();
          break;
        */
    default:
        break;
    }
}

//------------------------------------------------------
void initUnikey()
{
    UnikeySetup();
    BsKeycode = XKeysymToKeycode(display, XK_BackSpace);
    CommitKeycode = XKeysymToKeycode(display, COMMIT_KEYSYM);
    ReturnKeycode = XKeysymToKeycode(display, XK_Return);
    
    ReturnChar = 13; //*XKeysymToString(XK_Return);

    //memset(&GlobalOpt, 0, sizeof(GlobalOpt));
    UkSetDefOptions(&GlobalOpt);
    
    //apply initial settings
    //UnikeySetOutputCharset(GlobalOpt.charset);
    //UnikeySetInputMethod(GlobalOpt.inputMethod);
    
    //UkSetupMacro();
}

//------------------------------------------------------
void initXIM(Window im_window, const char *imname)
{
    XIMStyles *input_styles;
    XIMTriggerKeys *on_keys;
    XIMEncodings *encodings;
    char transport[80];		/* enough */

    if ((input_styles = (XIMStyles *)malloc(sizeof(XIMStyles))) == NULL) {
        fprintf(stderr, "Can't allocate\n");
        exit(1);
    }
    input_styles->count_styles = sizeof(Styles)/sizeof(XIMStyle) - 1;
    input_styles->supported_styles = Styles;

    if ((on_keys = (XIMTriggerKeys *)
         malloc(sizeof(XIMTriggerKeys))) == NULL) {
        fprintf(stderr, "Can't allocate\n");
        exit(1);
    }
    on_keys->count_keys = sizeof(Trigger_Keys)/sizeof(XIMTriggerKey) - 1;
    on_keys->keylist = Trigger_Keys;

    if ((encodings = (XIMEncodings *)malloc(sizeof(XIMEncodings))) == NULL) {
        fprintf(stderr, "Can't allocate\n");
        exit(1);
    }
    encodings->count_encodings = sizeof(SupportedEncodings)/sizeof(XIMEncoding) - 1;
    encodings->supported_encodings = SupportedEncodings;

    if (use_local) {
        char hostname[64];
        char *address = "/tmp/.ximsock";

        gethostname(hostname, 64);
        sprintf(transport, "local/%s:%s", hostname, address);
    } else if (use_tcp) {
        char hostname[64];
        int port_number = 9010;

        gethostname(hostname, 64);
        sprintf(transport, "tcp/%s:%d", hostname, port_number);
    } else {
        strcpy(transport, "X/");
    }

    UnikeyIms = IMOpenIM(display,
                         IMModifiers, "Xi18n",
                         IMServerWindow, im_window,
                         IMServerName, imname,
                         IMLocale, 
                         GlobalOpt.ximLocales,
                         IMServerTransport, transport,
                         IMInputStyles, input_styles,
                         NULL);

    if (UnikeyIms == (XIMS)NULL) {
        fprintf(stderr, "Can't Open Input Method Service:\n");
        fprintf(stderr, "\tInput Method Name :%s\n", imname);
        fprintf(stderr, "\tTranport Address:%s\n", transport);
        exit(1);
    }

    if (GlobalOpt.ximFlow == UkXimDynamic) {
        IMSetIMValues(UnikeyIms,
                      IMOnKeysList, on_keys,
                      NULL);
    }

    IMSetIMValues(UnikeyIms,
                  IMEncodingList, encodings,
                  IMProtocolHandler, MyProtoHandler,
                  IMFilterEventMask, filter_mask,
                  NULL);
}

//------------------------------------------------------
void usage()
{
    char *usageText = 
        "\nUnikey XIM - Vietnamese input method for X Window. Version "
#ifdef PACKAGE_VERSION
        PACKAGE_VERSION
#else
        "0.9.1"
#endif
        "\nUniKey project: http://unikey.sourceforge.net\n"
        "Copyright (C) 1998-2004 Pham Kim Long\n"
        "---------------------------------------------------------------\n"
        "Command line: \n"
        "  ukxim [OPTIONS]\n\n"
        "Options:\n"
        "  -h, -help        Print this help and exit\n"
        "  -v, -version     Show version and exit\n"
        "  -display <name>  Display name to connect to\n"
        "  -xvnkb-sync      Enable synchronization with xvnkb GUI\n"
        "  -config <file>   Specify configuration file (default: ~/.unikeyrc)\n"
        "  -macro <file>    Load macro file\n"
        "  -l, -locales <list> Locales accepted by unikey.\n"
        "                      Default: \"C,en_US,vi_VN,fr_FR,fr_BE,\n"
        "                                fr_CA,de_DE,ja_JP,cs_CZ,ru_RU\")\n"

        "\nExamples:\n"
        "  $ ukxim &\n"
        "      Runs ukxim with default options\n"
        "  $ unikey -macro ~/ukmacro &\n"
        "      Runs ukxim with ukmacro file loaded from home directory\n";

    puts(usageText);
}

//------------------------------------------------------
void showVersion()
{
    char *versionText = 
        "\nUnikey XIM - Vietnamese input method for X Window\n"
        "Version "
#ifdef PACKAGE_VERSION
        PACKAGE_VERSION
#else
        "0.9.1"
#endif
        "\n";
    puts(versionText);
}

//------------------------------------------------------
int main(int argc, char **argv)
{
    char *display_name = NULL;
    char *imname = NULL;
    Bool xvnkbSync = False;

    //    Window im_window;
    register int i;
    Bool report = False;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-display")) {
            display_name = argv[++i];
        }
        else if (!strcmp(argv[i], "-report")) {
            report = True;
        }
        else if (!strcmp(argv[i], "-watch-gui")) {
            UkWatchGUI = 1;
        }
        else if (!strcmp(argv[i], "-macro")) {
            //GlobalOpt.macroFile = strdup(argv[++i]);
            MacroFile = argv[++i];
        }
        else if (!strcmp(argv[i], "-config")) {
            ConfigFile = argv[++i];
        }
        else if (!strcmp(argv[i], "-locales")) {
            XimLocales = argv[++i];
        }
        else if (!strcmp(argv[i], "-xvnkb-sync")) {
            xvnkbSync = True;
        }
        else if (!strcmp(argv[i], "-help") || !strcmp(argv[i], "-h")) {
            usage();
            exit(0);
        }
        else if (!strcmp(argv[i], "-version") || !strcmp(argv[i], "-v")) {
            showVersion();
            exit(0);
        }
        else {
            puts("Wrong options! Run \"unikey -h\" for help\n");
            exit(-1);
        }
    }

    if (!imname) imname = DEFAULT_IMNAME;

    UkTestDefConfFile();

    if (ConfigFile == NULL) {
        ConfigFile = UkGetDefConfFileName();
    }

    if (!setlocale(LC_CTYPE, "en_US.UTF-8") && !setlocale(LC_CTYPE, "vi_VN.UTF-8")) {
        fputs("Cannot load either en_US.UTF-8 or vi_VN.UTF-8 locale\n"
              "To use this program you must have one of these locales installed\n", stderr);
        exit(1);
    }

    if ((display = XOpenDisplay(display_name)) == NULL) {
        fprintf(stderr, "Can't Open Display: %s\n", display_name);
        exit(1);
    }
    RootWindow = DefaultRootWindow(display);

    if (!singleLaunch()) {
        XCloseDisplay(display);
        fputs("An instance of ukxim has already started!\n", stderr);
        if (report)
            kill(getppid(), SIGUSR2); //failed to launch, don't try anymore
        exit(1);
    }

    XSetErrorHandler(myXErrorHandler);
    setRootPropMask();

    MainWindow = XCreateSimpleWindow(display, DefaultRootWindow(display),
                                     0, 700, 400, 800-700,
                                     0, WhitePixel(display, DefaultScreen(display)),
                                     WhitePixel(display, DefaultScreen(display)));

    if (MainWindow == (Window)NULL) {
        fprintf(stderr, "Can't Create Window\n");
        exit(1);
    }

    XStoreName(display, MainWindow, "Unikey");
    XSetTransientForHint(display, MainWindow, MainWindow);

    getSyncAtoms(xvnkbSync);
    UkGUIVisible = UkGetPropValue(AGUIVisible, 0);

    initUnikey();
    reloadConfig(SIGUSR1);

    initXIM(MainWindow, imname);

    XSelectInput(display, MainWindow, 
	   StructureNotifyMask | ButtonPressMask | PropertyChangeMask);

    //XMapWindow(display, im_window);
    XFlush(display);		/* necessary flush for tcp/ip connection */

    if (report)
        kill(getppid(), SIGUSR1); //lauched successfully

    signalSetup();

    UkLoopContinue = 1;
    while (UkLoopContinue) {
        XEvent event;
        XNextEvent(display, &event);
        if (XFilterEvent(&event, None) == True) {
            //		fprintf(stderr, "window %ld\n",event.xany.window);
            continue;
        }
        MyXEventHandler(MainWindow, &event);
    }
    cleanup();
    return 0;
}

//--------------------------------------------
void getSyncAtoms(Bool xvnkbSync)
{
    long v;

    AGUIVisible = XInternAtom(display, UKP_GUI_VISIBLE, False);

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


    AGUIPosX = XInternAtom(display, UKP_GUI_POS_X, False);
    AGUIPosY = XInternAtom(display, UKP_GUI_POS_Y, False);


    v = UkGetPropValue(AIMCharset, VKC_UTF8);
    GlobalOpt.charset = SyncToUnikeyCharset((int)v);

    v = UkGetPropValue(AIMMethod, VKM_TELEX);
    GlobalOpt.enabled = (v != VKM_OFF);

    if (!GlobalOpt.enabled)
        v = UkGetPropValue(AIMUsing, VKM_TELEX);

    GlobalOpt.inputMethod = SyncToUnikeyMethod((int)v);
}

//--------------------------------------------------
// this is copied from xvnkb
//--------------------------------------------------
void setRootPropMask()
{
    XWindowAttributes attr;

    XGetWindowAttributes(display, RootWindow, &attr);
    if (!(attr.your_event_mask  & PropertyChangeMask))
        XSelectInput(display, RootWindow, attr.your_event_mask | PropertyChangeMask);
}

//----------------------------------------------------
void resetState()
{
    UnikeyResetBuf();
    PendingCommit = False;
    PostponeKeyEv = False;
    PostponeCount = 0;
    DataCommit = False;
}

//----------------------------------------------------
// Single launch
// using technique of John Cwikla
//----------------------------------------------------
Bool singleLaunch()
{
    ProgAtom = XInternAtom(display, PROG_NAME, False);
    if (XGetSelectionOwner(display, ProgAtom) == None) {
        XSetSelectionOwner(display, ProgAtom, RootWindow, CurrentTime);
        return True;
    }
    return False;
}

//--------------------------------------------------
// this is a work-around of the difference between
// xvnkb and unikey in using viqr method
//--------------------------------------------------
void fixSyncToUnikeyMethod()
{
    long v;

    v = UkGetPropValue(AIMMethod, VKM_TELEX);
    GlobalOpt.enabled = (v != VKM_OFF);
    //fprintf(stderr, "fixSyncToUnikeyMethod. About to reset state\n");
    resetState();

    if (GlobalOpt.enabled) {
        GlobalOpt.inputMethod = SyncToUnikeyMethod((int)v);
    }
}

//--------------------------------------------------
// this is a work-around of the difference between
// xvnkb and unikey in using viqr method
//--------------------------------------------------
void fixUnikeyToSyncMethod(UkInputMethod method)
{
    long v;

    v = UnikeyToSyncMethod(method);
    UkSetPropValue(AIMUsing, v);
    UkSetPropValue(AIMMethod, v);
}

//----------------------------------------------------
int latinToUtf(char *dst, char *src, int inSize, int * pOutSize)
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

//------------------------------------------------------------
int myXErrorHandler(Display *dpy, XErrorEvent *error) 
{
    char buf[64];
    XGetErrorText(dpy, error->error_code, buf, sizeof(buf));
    fprintf (stderr, "\nUnexpected X error: %s\n serial %ld\n error_code %d\n"
             " request_code %d\n minor_code %d\n",
             buf,
             error->serial, 
             error->error_code, 
             error->request_code,
             error->minor_code);
    //fprintf(stderr, "X Error: %s\n", buf);
    return 0; //no problem!
}

//----------------------------------------------------
void reloadConfig(int signum)
{
    char *fname;
    long v;

    UnikeyGetOptions(&GlobalOpt.uk);

    if (GlobalOpt.macroFile) {
        free(GlobalOpt.macroFile);
        GlobalOpt.macroFile = 0;
    }

    if (GlobalOpt.ximLocales) {
        free(GlobalOpt.ximLocales);
        GlobalOpt.ximLocales = 0;
    }

    if (ConfigFile != NULL) {
        if (UkParseOptFile(ConfigFile, &GlobalOpt)) {
            // fprintf(stderr, "Config loaded\n");
        }
    }

    //  UkCleanupMacro();
    fname = 0;
    if (MacroFile)
        fname = MacroFile;
    else if (GlobalOpt.macroFile)
        fname = GlobalOpt.macroFile;

    if (fname) {
        if (UnikeyLoadMacroTable(fname)) {
            //      fputs("\nMacro file loaded!\n", stderr);
            //      UkSetupMacro();
            //UkMacroLoaded = True;
            GlobalOpt.uk.macroEnabled = 1;
        }
        else {
            //UkMacroLoaded = False;
            GlobalOpt.uk.macroEnabled = 0;
            fprintf(stderr, "\nFailed to load macro file: %s!\n", fname);
        }
    }

    if (GlobalOpt.usrKeyMapFile)
        UnikeyLoadUserKeyMap(GlobalOpt.usrKeyMapFile);
  
    UnikeySetOptions(&GlobalOpt.uk);

    //set sync properties
    v = UnikeyToSyncCharset(GlobalOpt.charset);
    UkSetPropValue(AIMCharset, v);
    fixUnikeyToSyncMethod(GlobalOpt.inputMethod);
    if (!GlobalOpt.enabled) {
        UkSetPropValue(AIMMethod, VKM_OFF);
    }

    if (XimLocales) {
        if (GlobalOpt.ximLocales)
            free(GlobalOpt.ximLocales);
        GlobalOpt.ximLocales = strdup(XimLocales);
    }
    else if (GlobalOpt.ximLocales == NULL)
        GlobalOpt.ximLocales = strdup(DefaultXimLocales);

    XFlush(display);
}

//----------------------------------------------------
int saveOptFile()
{
    if (!ConfigFile)
        return 0;
    //  GlobalOpt.posX = UkGetPropValue(AGUIPosX, -1);
    // GlobalOpt.posY = UkGetPropValue(AGUIPosY, -1);
    //  fprintf(stderr, "Macrofile before: %s\n", GlobalOpt.macroFile);
    UnikeyGetOptions(&GlobalOpt.uk);
    UkWriteOptFile(ConfigFile, &GlobalOpt);
    //fprintf(stderr, "Macrofile after: %s\n", GlobalOpt.macroFile);
    return 1;
}

//----------------------------------------------------
void terminatedHandler(int signum)
{
    cleanup();
    exit(0);
}

//----------------------------------------------------
void signalSetup()
{
    signal(SIGTERM, terminatedHandler);
    signal(SIGHUP, terminatedHandler);
    signal(SIGINT, terminatedHandler);
    signal(SIGQUIT, terminatedHandler);
    signal(SIGUSR1, reloadConfig);
}

//----------------------------------------------------
void cleanup()
{
    if (GlobalOpt.autoSave)
        saveOptFile();
    IMCloseIM(UnikeyIms);
    XDestroyWindow(display, MainWindow);
    XFlush(display);
    XCloseDisplay(display);
}

//----------------------------------------------------
void restoreKeys(XIMS ims, IMForwardEventStruct *call_data)
{
    XKeyEvent *kev = (XKeyEvent*)&call_data->event;
    UnikeyRestoreKeyStrokes();
    kev->state = kev->state & ~ControlMask; //clear Control-key mask

    if (UnikeyBackspaces > 0) {
        if (GlobalOpt.commitMethod == UkForwardCommit) {
            forwardBackspaces(ims, call_data, UnikeyBackspaces);
            if (UnikeyBufChars > 0)
                commitBuf(ims, call_data);
        }
        else {
            sendBackspaces(ims, call_data, UnikeyBackspaces);
            sendCommit(&call_data->event);
            PendingCommit = True;
            PostponeKeyEv = True;
            PostponeCount = 0;
            DataCommit = (UnikeyBufChars > 0);
        }
        //XSync(display, False);
        return;
    }

    if (UnikeyBufChars > 0)
        commitBuf(ims, call_data);
}
