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

    This version tidied and debugged by Steve Underwood May 1999
 
******************************************************************/

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <stdlib.h>
#include <stdio.h>

#include "Xtrans.h"
#include "FrameMgr.h"
#include "IMdkit.h"
#include "Xi18n.h"
#include "Xi18nTr.h"

extern Xi18nClient *_Xi18nFindClient (Xi18n, CARD16);
extern Xi18nClient *_Xi18nNewClient (Xi18n);
extern void _Xi18nDeleteClient (Xi18n, CARD16);
static Bool TransRead (XtransConnInfo, char *, int, int *);
static Bool TransWrite (XtransConnInfo, char *, int);
static void Xi18nWaitTransListen (Display *, int, XPointer);
static void Xi18nWaitTransAccept (Display *, int, XPointer);

static unsigned char *ReadTrIMMessage (XIMS ims, int fd, int *connect_id)
{
    Xi18n i18n_core = ims->protocol;
    Xi18nClient *client = i18n_core->address.clients;
    TransClient *tr_client;

    FrameMgr fm;
    extern XimFrameRec packet_header_fr[];
    register int total_size;
    unsigned char *p = NULL;
    unsigned char *pp;
    int read_length;
    XimProtoHdr	*hdr;
    Bool isConnect = False;
    CARD8 major_opcode, minor_opcode;
    CARD16 length;

    while (client != NULL)
    {
        tr_client = (TransClient *) client->trans_rec;
        if (tr_client->accept_fd == fd)
        {
            *connect_id = client->connect_id;
            break;
        }
        /*endif*/
        client = client->next;
    }
    /*endwhile*/

    if ((hdr = (XimProtoHdr *) malloc (sizeof (hdr))) == NULL)
        return (unsigned char *) NULL;
    /*endif*/
    
    if (!TransRead (tr_client->accept_conn,
                    (char *) hdr,
                    sizeof (hdr),
                    &read_length)
        ||
        read_length != sizeof (hdr))
    {
        goto read_error;
    }
    else
    {
        if (client->byte_order == '?')
        {
            if (hdr->major_opcode == XIM_CONNECT)
            {
                CARD8 byte_order;
                
                if (!TransRead (tr_client->accept_conn,
                                (char *) &byte_order,
                                sizeof (CARD8),
                                &read_length)
                    ||
                    read_length != sizeof (CARD8))
                {
                    goto read_error;
                }
                /*endif*/
                isConnect = True;
                client->byte_order = (CARD8) byte_order;
            }
            else
            {
                return (unsigned char *) NULL; 	/* can do nothing */
            }
            /*endif*/
        }
        /*endif*/
        fm = FrameMgrInit (packet_header_fr,
                           (char *) hdr,
                           _Xi18nNeedSwap (i18n_core, *connect_id));
        total_size = FrameMgrGetTotalSize (fm);
        /* get data */
        FrameMgrGetToken (fm, major_opcode);
        FrameMgrGetToken (fm, minor_opcode);
        FrameMgrGetToken (fm, length);
        FrameMgrFree (fm);

        if ((p = (unsigned char *) malloc (total_size + length*4)) == NULL)
            return (unsigned char *) NULL;
        /*endif*/
        pp = p;
        memmove(pp, &major_opcode, sizeof (CARD8));
        pp += sizeof (CARD8);
        memmove(pp, &minor_opcode, sizeof (CARD8));
        pp += sizeof (CARD8);
        memmove(pp, &length, sizeof (CARD16));
        pp += sizeof (CARD16);
        XFree (hdr);
        if (!isConnect)
        {
            if (length > 0)
            {
                if (!TransRead (tr_client->accept_conn,
                                (char *) pp,
                                length*4,
                                &read_length)
                    ||
                    read_length != length * 4)
                {
                    goto read_error;
                }
                /*endif*/
            }
            /*endif*/
        }
        else
        {
            memmove (pp, &client->byte_order, sizeof (CARD8));
            pp += sizeof (CARD8);
            if (!TransRead (tr_client->accept_conn,
                            (char *) pp,
                            length*4 - sizeof (CARD8),
                            &read_length)
                ||
                read_length != length*4 - sizeof (CARD8))
            {
                goto read_error;
            }
            /*endif*/
        }
        /*endif*/
    }
    /*endif*/
    return (unsigned char *) p;

    /* TODO: Get rid of this label, and the goto's  */
read_error:
    _XUnregisterInternalConnection (i18n_core->address.dpy, fd);
    _XimXTransDisconnect (tr_client->accept_conn);
    _XimXTransClose (tr_client->accept_conn);
    return (unsigned char *) NULL;
}

static Bool Xi18nTransBegin(XIMS ims)
{
    Xi18n i18n_core = ims->protocol;
    char *address = i18n_core->address.im_addr;
    TransSpecRec *spec = (TransSpecRec *) i18n_core->address.connect_addr;
    int fd;

    if (((spec->trans_conn = (struct _XtransConnInfo *)
                             _XimdXTransOpenCOTSServer(address)) == NULL)
        ||
        (_XimdXTransCreateListener(spec->trans_conn, spec->port) != 0))
    {
        return False;
    }
    /*endif*/
    fd = _XimXTransGetConnectionNumber(spec->trans_conn);
    return _XRegisterInternalConnection(i18n_core->address.dpy, fd,
                                        (_XInternalConnectionProc)Xi18nWaitTransListen,
                                        (XPointer)ims);
}

static Bool Xi18nTransEnd(XIMS ims)
{
    Xi18n i18n_core = ims->protocol;
    TransSpecRec *spec = (TransSpecRec *) i18n_core->address.connect_addr;
    int fd;

    fd = _XimXTransGetConnectionNumber (spec->trans_conn);
    if (fd == 0)
        return False;
    /*endif*/
    _XUnregisterInternalConnection (i18n_core->address.dpy, fd);
    _XimXTransDisconnect (spec->trans_conn);
    _XimXTransClose (spec->trans_conn);

    XFree (spec->port);
    XFree (spec);
    return True;
}

static Bool Xi18nTransSend (XIMS ims,
                            CARD16 connect_id,
                            unsigned char *reply,
                            long length)
{
    Xi18n i18n_core = ims->protocol;
    Xi18nClient *client = _Xi18nFindClient (i18n_core, connect_id);
    TransClient *tr_client = (TransClient *) client->trans_rec;

    if (length > 0)
    {
        if (TransWrite (tr_client->accept_conn, (char *) reply, length)
            != length)
        {
            return False;
        }
        /*endif*/
    }
    /*endif*/
    return True;
}

static Bool Xi18nTransWait (XIMS ims,
                            CARD16 connect_id,
                            CARD8 major_opcode,
                            CARD8 minor_opcode)
{
    Xi18n i18n_core = ims->protocol;
    Xi18nClient *client = _Xi18nFindClient(i18n_core, connect_id);
    TransClient *tr_client = (TransClient *)client->trans_rec;
    int fd = _XimXTransGetConnectionNumber(tr_client->accept_conn);

    for (;;)
    {
        unsigned char *packet;
        XimProtoHdr *hdr;
        int connect_id_ret;

        if ((packet = ReadTrIMMessage (ims, fd, &connect_id_ret))
            == (unsigned char *) NULL)
        {
            return False;
        }
        /*endif*/
        hdr = (XimProtoHdr *) packet;

        if ((hdr->major_opcode == major_opcode)
            &&
            (hdr->minor_opcode == minor_opcode))
        {
            return True;
        }
        else if (hdr->major_opcode == XIM_ERROR)
        {
            return False;
        }
        /*endif*/
    }
    /*endfor*/
}

static Bool Xi18nTransDisconnect(XIMS ims, CARD16 connect_id)
{
    Xi18n i18n_core = ims->protocol;
    Xi18nClient *client = _Xi18nFindClient (i18n_core, connect_id);
    TransClient *tr_client = (TransClient *) client->trans_rec;

    _XUnregisterInternalConnection (i18n_core->address.dpy,
                                    tr_client->accept_fd);
    _XimXTransDisconnect (tr_client->accept_conn);
    _XimXTransClose (tr_client->accept_conn);
    XFree (tr_client);
    _Xi18nDeleteClient (i18n_core, connect_id);
    return True;
}

static Bool TransRead (XtransConnInfo accept_conn,
                       char *buf,
                       int buf_len,
                       int *ret_len)
{
    int len;

    if ((len = _XimXTransRead (accept_conn, buf, buf_len)) <= 0)
        return False;
    /*endif*/
    *ret_len = len;
    return True;
}

static Bool TransWrite (XtransConnInfo accept_conn, char *buf, int len)
{
    register int nbyte;

    while (len > 0)
    {
        if ((nbyte = _XimXTransWrite (accept_conn, buf, len)) <= 0)
            return False;
        /*endif*/
        len -= nbyte;
        buf += nbyte;
    }
    /*endwhile*/
    return True;
}

Bool _Xi18nCheckTransAddress (Xi18n i18n_core,
                              TransportSW *transSW,
                              char *address)
{
    TransSpecRec *spec;
    char *p;
    char *hostname;

    if (!(spec = (TransSpecRec *) malloc (sizeof (TransSpecRec))))
        return False;
    /*endif*/
    if (!(hostname = (char *) malloc (strlen (address) + 1)))
    {
        XFree (spec);
        return False;
    }
    /*endif*/
    strcpy (hostname, address);

    if (p = index (hostname, ':'))
    {
        p++;
        if (!(spec->port = (char *) malloc (strlen (p) + 1)))
        {
            XFree (spec);
            XFree (hostname);
            return False;
        }
        /*endif*/
        strcpy (spec->port, p);
        XFree (hostname);
    }
    else
    {
        XFree (spec);
        XFree (hostname);
        return False;
    }
    /*endif*/
    i18n_core->address.connect_addr = (TransSpecRec *) spec;
    i18n_core->methods.begin = Xi18nTransBegin;
    i18n_core->methods.end = Xi18nTransEnd;
    i18n_core->methods.send = Xi18nTransSend;
    i18n_core->methods.wait = Xi18nTransWait;
    i18n_core->methods.disconnect = Xi18nTransDisconnect;
    return True;
}

static TransClient *NewTrClient (Xi18n i18n_core, XtransConnInfo accept_conn)
{
    Xi18nClient *client = _Xi18nNewClient (i18n_core);
    TransClient *tr_client;

    tr_client = (TransClient *) malloc (sizeof (TransClient));

    tr_client->accept_conn = accept_conn;
    tr_client->accept_fd = _XimXTransGetConnectionNumber (accept_conn);
    client->trans_rec = tr_client;

    return ((TransClient *) tr_client);
}

static void Xi18nWaitTransListen (Display *d, int fd, XPointer arg)
{
    XIMS ims = (XIMS)arg;
    Xi18n i18n_core = ims->protocol;
    TransSpecRec *spec = (TransSpecRec *) i18n_core->address.connect_addr;
    XtransConnInfo new_client;
    TransClient *client;
    int status;

    if ((new_client = (struct _XtransConnInfo *)
                      _XimdXTransAccept (spec->trans_conn, &status)) != NULL)
    {
        client = NewTrClient (i18n_core, new_client);
        _XRegisterInternalConnection (i18n_core->address.dpy,
                                      client->accept_fd,
                                      (_XInternalConnectionProc) Xi18nWaitTransAccept,
                                      (XPointer) ims);
    }
    /*endif*/
    return;
}

static void Xi18nWaitTransAccept (Display *d, int fd, XPointer arg)
{
    XIMS ims = (XIMS) arg;
    extern void _Xi18nMessageHandler (XIMS, CARD16, unsigned char *, Bool *);
    Bool delete = True;
    unsigned char *packet;
    int connect_id;

    if ((packet = ReadTrIMMessage (ims, fd, &connect_id))
        == (unsigned char *) NULL)
    {
        return;
    }
    /*endif*/
    _Xi18nMessageHandler (ims, connect_id, packet, &delete);
    if (delete == True)
        XFree (packet);
    /*endif*/
    return;
}
