#ifndef _TI_AES_H
#define _TI_AES_H

#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif

void aes_encrypt(unsigned char *state, unsigned char *key);
void aes_decrypt(unsigned char *state, unsigned char *key);

#ifdef __cplusplus
}
#endif

#endif  //_TI_AES_H
