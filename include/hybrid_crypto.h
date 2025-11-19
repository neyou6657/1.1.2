#ifndef HYBRID_CRYPTO_H
#define HYBRID_CRYPTO_H

#include "unified_crypto_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

int hybrid_adapter_init(void);
int hybrid_adapter_cleanup(void);

typedef struct {
    uci_keypair_t classic_keypair;
    uci_keypair_t pq_keypair;
} hybrid_keypair_t;

int hybrid_rsa_dilithium_keygen(uci_keypair_t *keypair);
int hybrid_rsa_dilithium_sign(const uci_keypair_t *keypair, const uint8_t *message,
                              size_t message_len, uci_signature_t *signature);
int hybrid_rsa_dilithium_verify(const uci_keypair_t *keypair, const uint8_t *message,
                                size_t message_len, const uci_signature_t *signature);

int hybrid_ecdsa_dilithium_keygen(uci_keypair_t *keypair);
int hybrid_ecdsa_dilithium_sign(const uci_keypair_t *keypair, const uint8_t *message,
                                size_t message_len, uci_signature_t *signature);
int hybrid_ecdsa_dilithium_verify(const uci_keypair_t *keypair, const uint8_t *message,
                                  size_t message_len, const uci_signature_t *signature);

#ifdef __cplusplus
}
#endif

#endif
