int _Xi18nNeedSwap(Xi18n i18n_core, CARD16 connect_id);
Xi18nClient * _Xi18nNewClient(Xi18n i18n_core);
Xi18nClient * _Xi18nFindClient(Xi18n i18n_core, CARD16 connect_id);
void _Xi18nDeleteClient(Xi18n i18n_core, CARD16 connect_id);
void _Xi18nSendMessage(XIMS ims, CARD16 connect_id,
		  CARD8 major_opcode, CARD8 minor_opcode,
		  unsigned char *data, long length);
void _Xi18nSendTriggerKey(XIMS ims, CARD16 connect_id);
void _Xi18nSetEventMask(XIMS ims, CARD16 connect_id,
		   CARD16 im_id, CARD16 ic_id,
		   CARD32 forward_mask, CARD32 sync_mask);
