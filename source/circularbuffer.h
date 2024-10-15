/*
	Circular Buffer Header File
*/
#ifndef CIRCULARBUFFER_H
#define	CIRCULARBUFFER_H

typedef struct {
	unsigned char rp;
	unsigned char wp;
	unsigned char sz;	// set size-1
	unsigned char *buf;	// length max 256
} SCIRCULARBUFFER;

short putbuf(SCIRCULARBUFFER *psBuf, unsigned char data);
short getbuf(SCIRCULARBUFFER *psBuf);

#endif	/* CIRCULARBUFFER_H */
