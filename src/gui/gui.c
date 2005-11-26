// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2000-2005 Pham Kim Long
 * Contact:
 *   unikey@gmail.com
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <string.h>
#include <time.h>
#include "xvnkb.h"
#include "uksync.h"
#include "guiopt.h"

#define PROG_NAME "Unikey XIM Simple GUI"

/* using global variables is a very bad way of communication. Must do something better!!! */
Display *display;
Window MainWindow, RootWindow;
Atom ProgAtom;
Atom _NET_WM_WINDOW_TYPE, _NET_WM_WINDOW_TYPE_DOCK;
Atom ARaiseWindow;

int UkLoopContinue = 1;
int UkGUIVisible = 1;

static int ScreenNum;
static char *progname; /* name this program was invoked by */
static char XimPath[256];
static char *MacroFile = 0;
static char *ConfigFile = 0;
static char *XimLocales = 0;
static char *DisplayName = NULL;
static time_t StartTime;

GC MainGC;
XFontStruct *FontInfo;
unsigned int MainWinWidth = 80;
unsigned int MainWinHeight = 24;
unsigned int displayWidth, displayHeight;
int MainWinX, MainWinY;

unsigned long RedXColor, BlueXColor;

Colormap MainColormap;

//char *InfoText;

void getGC(Window win, GC *gc, XFontStruct *font_info);
void loadFont(XFontStruct ** font_info);
void drawText(Window win, GC gc, XFontStruct *fontInfo, 
	      unsigned int winWith, unsigned int winHeight);

void drawMainWindow(Window win, GC gc, XFontStruct *fontInfo, 
	      unsigned int winWidth, unsigned int winHeight);


//-----------------------------------------------------------
void freeXResources();
void allocXResources();
void propertyChanged();
void getSyncAtoms();
void getInitSettings();

//long atom_get_value(Atom key);
void setRootPropMask();
void fixUnikeyToSyncMethod(int method);
void fixSyncToUnikeyMethod();

// sync atoms
Atom AIMCharset, AIMUsing, AIMMethod;
Atom AGUIPosX, AGUIPosY;
Atom AGUIVisible;

UkGuiOpt GlobalOpt;

//-----------------------------------------------------------
void getInitPos();

//-----------------------------------------------------------
// process management
Bool singleLaunch();
void signalSetup();
pid_t childProcess();
void ximTerminatedHandler(int signum);
void ximLaunchHandler(int signum);
void terminatedHandler(int signum);
void getXimPath();
void cleanup();
void reloadXimConfig();

int XimLaunchOk = False;
pid_t ChildPid = -1;


//-----------------------------------------------------------
char * UkModeList[] = {
  "UTF8",
  "VIQR",
  "TCVN",
  "VNI",
  "BK2"
};

char * UkMethodList[] = {
  "TX",
  "VI",
  "VR",
  "UD"
};


char *UkOffText = "OFF";

int UnikeyOn = 0;
int UkActiveMode = UNIKEY_UTF8;
int UkActiveMethod = UNIKEY_TELEX_INPUT;


//------------------------------------------------------
void rotateUkMode()
{
  int v;
  int mod = sizeof(UkModeList)/sizeof(UkModeList[0]);
  int newActiveMode, newUnikeyOn;

  newActiveMode = (UkActiveMode+1) % mod;
  newUnikeyOn = 1;

  //synchronize charset
  v = UnikeyToSyncCharset(newActiveMode);
  UkSetPropValue(AIMCharset, v);

  //synchronize on/off flag
  fixUnikeyToSyncMethod(UkActiveMethod);
}

//------------------------------------------------------
void rotateUkMethod()
{
  int mod = sizeof(UkMethodList)/sizeof(UkMethodList[0]);
  fixUnikeyToSyncMethod((UkActiveMethod+1) % mod);
}

//------------------------------------------------------
void switchUnikey()
{
  if (UnikeyOn)
    UkSetPropValue(AIMMethod, VKM_OFF);
  else {
    fixUnikeyToSyncMethod(UkActiveMethod);
    /*
    long v = UnikeyToSyncMethod(UkActiveMethod);
    UkSetPropValue(AIMMethod, v);
    UkSetPropValue(AIMUsing, v);
    */
  }
}

//------------------------------------------------------
void handlePropertyChanged(Window win, XEvent *event)
{
  long v;
  int redraw = 0;

  XPropertyEvent *pev = (XPropertyEvent *)event;

  if (pev->atom == AIMCharset) {
    v = UkGetPropValue(pev->atom, VKC_UTF8);
    UkActiveMode = SyncToUnikeyCharset(v);
    redraw = 1;
  }
  else if (pev->atom == AIMMethod) {
    fixSyncToUnikeyMethod();
    XRaiseWindow(display, win);
    redraw = 1;
  }
  else if (pev->atom == AIMUsing) {
    //don't need this
  }
  else if (pev->atom == AGUIVisible) {
    v = UkGetPropValue(pev->atom, 0);
    if (v) {
      UkGUIVisible = 1;
      XMapWindow(display, MainWindow);
    }
    else {
      UkGUIVisible = 0;
      XUnmapWindow(display, MainWindow);
    }
  }
  else if (pev->atom == ARaiseWindow) {
    //printf("Raise window\n"); //DEBUG
    XRaiseWindow(display, win);    
  }

  if (redraw)
    drawMainWindow(MainWindow, MainGC, FontInfo, MainWinWidth, MainWinHeight);
}

//------------------------------------------------------
void MyXEventHandler(Window im_window, XEvent *event)
{
  static int dragX, dragY;
  static int dragging = 0;
  static int moved = 0;

  switch (event->type) {
  case DestroyNotify:
    break;
  case ButtonPress:
    switch (event->xbutton.button) {
    case Button1:
      {
	XButtonEvent *bev = (XButtonEvent *)event;
	if ((bev->state & ControlMask) && (bev->state & Mod1Mask)) {
	  UkSetPropValue(AGUIVisible, 0);
	  break;
	}
	if ((bev->state & ShiftMask) && (bev->state & ControlMask)) {
	  reloadXimConfig();
	  break;
	}
	dragging = 1;
	dragX = bev->x_root - MainWinX;
	dragY = bev->y_root - MainWinY;
      }
      break;
    case Button3:
      if (event->xbutton.state & ControlMask)
	rotateUkMethod();
      else
	rotateUkMode();
      break;
    }
    break;
  case ButtonRelease:
    if (event->xbutton.button == Button1) {
      if (dragging) {
	if (!moved) {
	  switchUnikey();
	}
      }
      dragging = 0;
      moved = 0;
    }
    break;

  case Expose:
    if (event->xexpose.count == 0)
      drawMainWindow(MainWindow, MainGC, FontInfo, MainWinWidth, MainWinHeight);
    break;

  case MotionNotify:
    if (dragging) {
      XMotionEvent *mev = (XMotionEvent *)event;
      moved = 1;
      MainWinX = mev->x_root - dragX;
      MainWinY = mev->y_root - dragY;

      if (MainWinX < 0)
	MainWinX = 0;
      else if (MainWinX + MainWinWidth > displayWidth)
	MainWinX = displayWidth - MainWinWidth;

      if (MainWinY < 0)
	MainWinY = 0;
      else if (MainWinY + MainWinHeight > displayHeight)
	MainWinY = displayHeight - MainWinHeight;

      XMoveWindow(display, im_window, MainWinX, MainWinY);

      dragX = mev->x_root - MainWinX;
      dragY = mev->y_root - MainWinY;

      UkSetPropValue(AGUIPosX, MainWinX);
      UkSetPropValue(AGUIPosY, MainWinY);

    }
    break;

  case PropertyNotify:
    if (event->xproperty.window == RootWindow)
      handlePropertyChanged(im_window, event);
    break;

  case VisibilityNotify:
    {
      if (event->xvisibility.state != VisibilityUnobscured) {
	time_t current = time(0);
	if (current - StartTime < 3*60)
	  XRaiseWindow(display, im_window);
      }

      /*
      static int count = 10;
      if (event->xvisibility.state != VisibilityUnobscured && count > 0) {
	count--;
	//	fprintf(stderr, "Visibility count: %d\n", count);
	XRaiseWindow(display, im_window);
      }
      */
    }
    break;
  default:
    break;
  }
  return;
}

//------------------------------------------------------
void usage()
{
  char *usageText = 
    "\nUnikey - Vietnamese input method for X Window. Version "
#ifdef PACKAGE_VERSION
    PACKAGE_VERSION
#else
    "0.9.1"
#endif
    "\nUniKey project: http://unikey.org\n"
    "Copyright (C) 2000-2005 Pham Kim Long\n"
    "---------------------------------------------------------------\n"
    "Command line: \n"
    "  unikey [OPTIONS]\n\n"
    "Options:\n"
    "  -h, -help           Print this help and exit\n"
    "  -v, -version        Show version and exit\n"
    "  -display <name>     Display name to connect to\n"
    "  -xim <ukxim>        Path to Unikey XIM server (ukxim)\n"
    "  -config <file>      Specify configuration file (default: ~/.unikey/option)\n"
    "  -macro <file>       Load macro file\n"
    "  -l, -locales <list> Locales accepted by unikey.\n"
    "                      Default: \"C,en_US,vi_VN,fr_FR,fr_BE,\n"
    "                                fr_CA,de_DE,ja_JP,cs_CZ,ru_RU\")\n"
    "\nExamples:\n"
    "  $ unikey\n"
    "      Unikey will search for ukxim in the default search path\n"
    "  $ unikey -xim /usr/local/bin/ukxim -macro ~/ukmacro\n"
    "      Explicitly specifies ukxim, and loads ukmacro\n";

  puts(usageText);
}

//------------------------------------------------------
void showVersion()
{
  char *versionText = 
    "\nUnikey - Vietnamese input method for X Window\n"
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
void getOptions()
{
  UkGuiSetDefOptions(&GlobalOpt);
  if (ConfigFile)
    UkGuiParseOptFile(ConfigFile, &GlobalOpt);
  else
    UkGuiParseOptFile(UkGuiGetDefConfFileName(), &GlobalOpt);
}

//------------------------------------------------------
int main(int argc, char **argv)
{
  XSizeHints *sizeHints;
  XWMHints *wmHints;
  XClassHint *classHints;
  XTextProperty windowNameP, iconNameP;
  char *windowName = "UniKey GUI";
  char *iconName = "UniKey";
  XSetWindowAttributes attrs;
  Bool setXim = False;
  int i;
  pid_t pid;

  progname = argv[0];

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-display")) {
      DisplayName = argv[++i];
    }
    else if (!strcmp(argv[i], "-xim")) {
      setXim = True;
      strcpy(XimPath, argv[++i]);
    }
    else if (!strcmp(argv[i], "-macro")) {
      MacroFile = argv[++i];
    }
    else if (!strcmp(argv[i], "-config")) {
      ConfigFile = argv[++i];
    }
    else if (!strcmp(argv[i], "-locales") || !strcmp(argv[i], "-l")) {
      XimLocales = argv[++i];
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

  if (!setXim)
    getXimPath();
  
  pid = fork();
  if (pid > 0)
    exit(0);
  else if (pid < 0) {
    fputs("failed to launch new child process\n", stderr);
    exit(-1);
  }

  getOptions();

  if (!(sizeHints = XAllocSizeHints())) {
    fprintf(stderr, "%s: failure allocating memory\n", progname);
    exit(0);
  }

  if (!(wmHints = XAllocWMHints())) {
    fprintf(stderr, "%s: failure allocating memory\n", progname);
    exit(0);
  }
  
  if (!(classHints = XAllocClassHint())) {
    fprintf(stderr, "%s: failure allocating memory\n", progname);
    exit(0);
  }

  if ((display = XOpenDisplay(DisplayName)) == NULL) {
    fprintf(stderr, "Can't Open Display: %s\n", XDisplayName(DisplayName));
    XCloseDisplay(display);
    exit(1);
  }

  StartTime = time(0);
  RootWindow = DefaultRootWindow(display);
  setRootPropMask();
  getSyncAtoms();

  if (!singleLaunch()) {
    //check if previous instance is hidden
    if (!UkGetPropValue(AGUIVisible, 0)) {
      //wake up
      UkSetPropValue(AGUIVisible, 1);
      XFlush(display);
    }
    else {
      UkSetPropValue(ARaiseWindow, 1);
      XFlush(display);
      fputs("An instance of unikey has already started!\n", stderr);
    }
    exit(1);
  }

  UkSetPropValue(AGUIVisible, 1);
  UkGUIVisible = 1;

  /* get screen size from display structure macro */
  ScreenNum = DefaultScreen(display);
  displayWidth = DisplayWidth(display, ScreenNum);
  displayHeight = DisplayHeight(display, ScreenNum);
  MainColormap = DefaultColormap(display, ScreenNum);

  // MainWinX = displayWidth-MainWinWidth-50;
  // MainWinY = displayHeight-MainWinHeight-50;
  getInitPos();

  MainWindow = XCreateSimpleWindow(display, DefaultRootWindow(display),
				   MainWinX,
				   MainWinY,
				   MainWinWidth, MainWinHeight,
				   0, WhitePixel(display, ScreenNum),
				   WhitePixel(display, ScreenNum));
  attrs.override_redirect = True;
  XChangeWindowAttributes(display, MainWindow, CWOverrideRedirect, &attrs);

  if (MainWindow == (Window)NULL) {
    fprintf(stderr, "Can't Create Window\n");
    exit(1);
  }

  // Setup window properties
  sizeHints->flags = PPosition | PSize | PMinSize;
  sizeHints->min_width = MainWinWidth;
  sizeHints->min_height = MainWinHeight;

  if (XStringListToTextProperty(&windowName, 1, &windowNameP) == 0) {
    fprintf( stderr, "Structure allocation for windowName failed.\n"); 
    exit(-1);
  }

  if (XStringListToTextProperty(&iconName, 1, &iconNameP) == 0) {
    fprintf( stderr, "Structure allocation for iconName failed.\n"); 
    exit(-1);
  }

  wmHints->initial_state = NormalState;
  wmHints->input = True;
  wmHints->flags = StateHint | InputHint;

  classHints->res_name = progname;
  classHints->res_class = "UnikeyWindow";

  XSetWMProperties(display, MainWindow, &windowNameP, &iconNameP, 
		   argv, argc, sizeHints, wmHints, 
		   classHints);

  XChangeProperty(display, MainWindow, 
		  _NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, 
		  (unsigned char *)&_NET_WM_WINDOW_TYPE_DOCK, 1);

  //  XStoreName(display, im_window, "Unikey");
  XSetTransientForHint(display, MainWindow, MainWindow);
  XSelectInput(display, MainWindow, 
     StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask |
     VisibilityChangeMask | PointerMotionMask | PropertyChangeMask);
  
  allocXResources();
  getInitSettings();

  XMapWindow(display, MainWindow);
  UkSetPropValue(AGUIPosX, MainWinX);
  UkSetPropValue(AGUIPosY, MainWinY);

  signalSetup();
  ChildPid = childProcess();

  UkLoopContinue = 1;
  while (UkLoopContinue) {
    XEvent event;
    XNextEvent(display, &event);
    /*
      if (XFilterEvent(&event, None) == True) {
      continue;
      }
    */
    MyXEventHandler(MainWindow, &event);
  }
  cleanup();
  return 0;
}

//------------------------------------------------------
void getGC(Window win, GC *gc, XFontStruct *font)
{
  unsigned long valuemask = 0; /* ignore XGCvalues and use defaults */
  XGCValues values;
	
  /* Create default Graphics Context */
  *gc = XCreateGC(display, win, valuemask, &values);

  /* specify font */
  XSetFont(display, *gc, font->fid);

  /* specify black foreground since default window background is 
   * white and default foreground is undefined. */
  XSetForeground(display, *gc, RedXColor); //BlackPixel(display,ScreenNum));
}

//--------------------------------------------------------------
void loadFont(XFontStruct ** font)
{
  char *fontname = "-*-helvetica-bold-r-normal-*-12-120-*-*-*-*-*-*";
  char *failsafeFont = "7x13";
  /* Load font and get font information structure. */
  if ((*font = XLoadQueryFont(display,fontname)) == NULL) {
    //load failsafe font
    if ((*font = XLoadQueryFont(display,failsafeFont)) == NULL) {
      fprintf( stderr, "%s: Cannot open font\n", progname);
      exit( -1 );
    }
  }
}

//--------------------------------------------
void drawMainWindow(Window win, GC gc, XFontStruct *fontInfo, 
	      unsigned int winWidth, unsigned int winHeight)
{
  char text[100] ;
  int len;
  int textWidth;
  int fontHeight;
  
  if (UnikeyOn)
    sprintf(text, "%s: %s", UkMethodList[UkActiveMethod], UkModeList[UkActiveMode]);
  else
    strcpy(text, UkOffText);

  XSetForeground(display, gc, WhitePixel(display, ScreenNum));
  XFillRectangle(display, win, gc, 0, 0, winWidth, winHeight);

  //  XSetLineAttributes(display, gc, 2, LineSolid, CapNotLast, JoinMiter);
  XSetForeground(display, gc, BlueXColor); //BlackPixel(display, ScreenNum));
  XDrawRectangle(display, win, gc, 0, 0, winWidth-1, winHeight-1);
  XDrawRectangle(display, win, gc, 3, 3, winWidth-7, winHeight-7);

  XSetForeground(display, gc, RedXColor);
  len = strlen(text);
  textWidth = XTextWidth(fontInfo, text, len);

  fontHeight = fontInfo->ascent + fontInfo->descent;

  /* output text, centered on each line */
  XDrawString(display, win, gc, (winWidth - textWidth)/2, 
	      (winHeight+fontHeight)/2-2, text, len);
}


//--------------------------------------------
void allocXResources()
{
  XColor exactColor, screenColor;
  XAllocNamedColor(display, MainColormap, "red", &screenColor, &exactColor);
  RedXColor = screenColor.pixel;
  XAllocNamedColor(display, MainColormap, "blue", &screenColor, &exactColor);  
  BlueXColor = screenColor.pixel;

  loadFont(&FontInfo);
  getGC(MainWindow, &MainGC, FontInfo);
}

//--------------------------------------------
void freeXResources()
{
  XUnloadFont(display, FontInfo->fid);
  XFreeGC(display, MainGC);
}

//--------------------------------------------------
// this is a work-around of the difference between
// xvnkb and unikey in using viqr method
//--------------------------------------------------
void fixSyncToUnikeyMethod()
{
  long v;
  v = UkGetPropValue(AIMMethod, VKM_TELEX);
  UnikeyOn = (v != VKM_OFF);

  if (!UnikeyOn) {
    v = UkGetPropValue(AIMUsing, VKM_TELEX);
  }
  UkActiveMethod = SyncToUnikeyMethod((int)v);
}

//--------------------------------------------------
// this is a work-around of the difference between
// xvnkb and unikey in using viqr method
//--------------------------------------------------
void fixUnikeyToSyncMethod(int method)
{
  long v;
  v = UnikeyToSyncMethod(method);
  UkSetPropValue(AIMUsing, v);
  UkSetPropValue(AIMMethod, v);
}

//--------------------------------------------
void getSyncAtoms()
{
  AGUIVisible = XInternAtom(display, UKP_GUI_VISIBLE, False);

  AIMCharset = XInternAtom(display, UKP_CHARSET, False);
  AIMMethod = XInternAtom(display, UKP_METHOD, False);
  AIMUsing = XInternAtom(display, UKP_USING, False);

  AGUIPosX = XInternAtom(display, UKP_GUI_POS_X, False);
  AGUIPosY = XInternAtom(display, UKP_GUI_POS_Y, False);

  _NET_WM_WINDOW_TYPE = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  _NET_WM_WINDOW_TYPE_DOCK = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
  ARaiseWindow = XInternAtom(display, "_UNIKEY_RAISE_WINDOW", False);
}

//--------------------------------------------
void getInitSettings()
{
  long v;

  v = UkGetPropValue(AIMCharset, VKC_UTF8);
  UkActiveMode = SyncToUnikeyCharset((int)v);

  v = UkGetPropValue(AIMMethod, VKM_TELEX);
  UnikeyOn = (v != VKM_OFF);

  if (!UnikeyOn)
    v = UkGetPropValue(AIMUsing, VKM_TELEX);

  UkActiveMethod = SyncToUnikeyMethod((int)v);

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

//----------------------------------------------------
pid_t childProcess()
{
  pid_t pid;
  int ret;
  char *argv[10];
  int argc;

  pid = fork();

  if (pid == 0) {
    argc = 0;
    argv[argc++] = "ukxim";
    argv[argc++] = "-report";
    argv[argc++] = "-watch-gui";

     if (MacroFile) {
      argv[argc++] = "-macro";
      argv[argc++] = MacroFile;
    }

    if (ConfigFile) {
      argv[argc++] = "-config";
      argv[argc++] = ConfigFile;
    }

    if (XimLocales) {
      argv[argc++] = "-locales";
      argv[argc++] = XimLocales;
    }

    if (DisplayName) {
      argv[argc++] = "-display";
      argv[argc++] = DisplayName;
    }
    argv[argc++] = 0;

    ret = execvp(XimPath, argv);

    if (ret == -1) {
      fprintf(stderr, "Failed to launch %s\n", XimPath);
      exit(-1);
    }
    return 0;
  }
  else if (pid > 0)
    return pid;
  else
    fputs("Failed to lauch child process\n", stderr);
  return -1;
}

//----------------------------------------------------
void ximTerminatedHandler(int signum)
{
  sigset_t sigs;

  waitpid(WAIT_ANY, NULL, WNOHANG); //to kill zombies

  sigemptyset(&sigs);
  sigaddset(&sigs, SIGCHLD);
  sigprocmask(SIG_BLOCK, &sigs, NULL);

  if (XimLaunchOk) {
    XimLaunchOk = False;
    ChildPid = childProcess();
    if (ChildPid > 0)
      fputs("Unikey XIM server reloaded!\n", stderr);
  }
}

//----------------------------------------------------
void ximLaunchHandler(int signum)
{
  sigset_t sigs;
  if (signum == SIGUSR1)
    XimLaunchOk = True;
  else
    XimLaunchOk = False;
  sigemptyset(&sigs);
  sigaddset(&sigs, SIGCHLD);
  sigprocmask(SIG_UNBLOCK, &sigs, NULL);
}

//----------------------------------------------------
void terminatedHandler(int signum)
{
  //  fprintf(stderr, "Killing child process: %d\n", ChildPid);
  sigset_t sigs;
  sigemptyset(&sigs);
  sigaddset(&sigs, SIGCHLD);
  sigprocmask(SIG_BLOCK, &sigs, NULL);
  
  cleanup();
  exit(0);
}

//----------------------------------------------------
void signalSetup()
{
  signal(SIGCHLD, ximTerminatedHandler);
  signal(SIGUSR1, ximLaunchHandler);
  signal(SIGUSR2, ximLaunchHandler);
  signal(SIGTERM, terminatedHandler);
  signal(SIGKILL, terminatedHandler);
  signal(SIGINT, terminatedHandler);
  signal(SIGQUIT, terminatedHandler);
  signal(SIGHUP, terminatedHandler);
}

//----------------------------------------------------
void getXimPath()
{
  //just use search path, don't rely on the path that 
  // this GUI program was started in
  strcpy(XimPath, "ukxim"); 

  /*
  // get path from the command line
  char *p;
  strcpy(XimPath, progname);
  p = strrchr(XimPath, '/');
  if (p == NULL)
    p = XimPath;
  else
    p++;
  strcpy(p, "ukxim");
  */
}


//------------------------------------------------------
void getInitPos()
{
  if (GlobalOpt.posX < 0)
    MainWinX = displayWidth-MainWinWidth-50;
  else {
    MainWinX = GlobalOpt.posX;
    if (MainWinX + MainWinWidth > displayWidth)
      MainWinX = displayWidth - MainWinWidth;
  }

  if (GlobalOpt.posY < 0)
    MainWinY = displayHeight-MainWinHeight-50;
  else {
    MainWinY = GlobalOpt.posY;
    if (MainWinY + MainWinHeight > displayHeight)
      MainWinY = displayHeight - MainWinHeight;
  }
}

//----------------------------------------------------
void cleanup()
{
  XDestroyWindow(display, MainWindow);
  freeXResources();
  XCloseDisplay(display);
  if (ChildPid > 0) {
    kill(ChildPid, SIGTERM);
  }
}

//----------------------------------------------------
void reloadXimConfig()
{
  if (ChildPid > 0) {
    kill(ChildPid, SIGUSR1);
    XBell(display, 0);
  }
}
