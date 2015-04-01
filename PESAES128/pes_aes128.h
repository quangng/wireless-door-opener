#ifndef PES_AES128_H
#define PES_AES128_H
#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif

void aes_encrypt(uint8_t *state, uint8_t *key);
void aes_decrypt(uint8_t *state, uint8_t *key);

#ifdef __cplusplus
}
#endif
#endif
