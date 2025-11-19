#ifndef CLASSIC_CRYPTO_ADAPTER_H
#define CLASSIC_CRYPTO_ADAPTER_H

#include "unified_crypto_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

int classic_adapter_init(void);
int classic_adapter_cleanup(void);

int classic_rsa2048_keygen(uci_keypair_t *keypair);
int classic_rsa2048_sign(const uci_keypair_t *keypair, const uint8_t *message,
                         size_t message_len, uci_signature_t *signature);
int classic_rsa2048_verify(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, const uci_signature_t *signature);

int classic_ecdsa_p256_keygen(uci_keypair_t *keypair);
int classic_ecdsa_p256_sign(const uci_keypair_t *keypair, const uint8_t *message,
                            size_t message_len, uci_signature_t *signature);
int classic_ecdsa_p256_verify(const uci_keypair_t *keypair, const uint8_t *message,
                              size_t message_len, const uci_signature_t *signature);

int classic_sm2_keygen(uci_keypair_t *keypair);
int classic_sm2_sign(const uci_keypair_t *keypair, const uint8_t *message,
                     size_t message_len, uci_signature_t *signature);
int classic_sm2_verify(const uci_keypair_t *keypair, const uint8_t *message,
                       size_t message_len, const uci_signature_t *signature);

#ifdef __cplusplus
}
#endif

#endif
