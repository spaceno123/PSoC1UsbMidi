/*
	Program	circularbuffer.c
	Date	2012/04/30 .. 2012/04/30	(generic)
	Copyright (C) 2012 by AKIYA
	--- up date ---
*/
#include "circularbuffer.h"

/* --- put circular buffer --- */
short putbuf(SCIRCULARBUFFER *psBuf, unsigned char data)
{
	unsigned char wNext = psBuf->wp == psBuf->sz ? 0 : psBuf->wp+1;
	short ret;

	if (wNext != psBuf->rp) {
		(psBuf->buf)[psBuf->wp] = data;
		psBuf->wp = wNext;
		ret = 0;
	}
	else {
		ret = -1;	// buffer full !
	}
	return ret;
}

/* --- get circular buffer --- */
short getbuf(SCIRCULARBUFFER *psBuf)
{
	short ret;

	if (psBuf->rp != psBuf->wp) {
		unsigned char rNext = psBuf->rp == psBuf->sz ? 0 : psBuf->rp+1;

		ret = (psBuf->buf)[psBuf->rp];
		psBuf->rp = rNext;
	}
	else {
		ret = -1;	// buffer empty !
	}
	return ret;
}
