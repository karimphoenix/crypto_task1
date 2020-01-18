#ifndef MD2_H_INCLUDED
#define MD2_H_INCLUDED

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

typedef struct {
  unsigned char state[16];                                 /* state */
  unsigned char checksum[16];                           /* checksum */
  unsigned int count;                 /* number of bytes, modulo 16 */
  unsigned char buffer[16];                         /* input buffer */
} MD2_CTX;

void MD2Init (MD2_CTX *);
void MD2Update
  (MD2_CTX *, unsigned char *, unsigned int);
void MD2Final (unsigned char [16], MD2_CTX *);


#endif // MD2_H_INCLUDED
