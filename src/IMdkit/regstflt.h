
void _XRegisterFilterByMask(Display *, Window, unsigned long, Bool(*)(Display*,Window,XEvent*,XPointer), XPointer);
void _XRegisterFilterByType(Display *,Window, int, int, Bool (*)(Display*, Window, XEvent *, XPointer), XPointer);
void _XUnregisterFilter(Display *, Window, Bool (*)(Display *, Window, XEvent*, XPointer), XPointer);

