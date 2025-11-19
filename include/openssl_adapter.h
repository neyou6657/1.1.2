#ifndef OPENSSL_ADAPTER_H
#define OPENSSL_ADAPTER_H

#include "unified_crypto_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

int openssl_adapter_init(void);
int openssl_adapter_cleanup(void);

int openssl_rsa2048_keygen(uci_keypair_t *keypair);
int openssl_rsa2048_sign(const uci_keypair_t *keypair, const uint8_t *message,
                         size_t message_len, uci_signature_t *signature);
int openssl_rsa2048_verify(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, const uci_signature_t *signature);

int openssl_rsa3072_keygen(uci_keypair_t *keypair);
int openssl_rsa3072_sign(const uci_keypair_t *keypair, const uint8_t *message,
                         size_t message_len, uci_signature_t *signature);
int openssl_rsa3072_verify(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, const uci_signature_t *signature);

int openssl_rsa4096_keygen(uci_keypair_t *keypair);
int openssl_rsa4096_sign(const uci_keypair_t *keypair, const uint8_t *message,
                         size_t message_len, uci_signature_t *signature);
int openssl_rsa4096_verify(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, const uci_signature_t *signature);

int openssl_ecdsa_p256_keygen(uci_keypair_t *keypair);
int openssl_ecdsa_p256_sign(const uci_keypair_t *keypair, const uint8_t *message,
                            size_t message_len, uci_signature_t *signature);
int openssl_ecdsa_p256_verify(const uci_keypair_t *keypair, const uint8_t *message,
                              size_t message_len, const uci_signature_t *signature);

int openssl_ecdsa_p384_keygen(uci_keypair_t *keypair);
int openssl_ecdsa_p384_sign(const uci_keypair_t *keypair, const uint8_t *message,
                            size_t message_len, uci_signature_t *signature);
int openssl_ecdsa_p384_verify(const uci_keypair_t *keypair, const uint8_t *message,
                              size_t message_len, const uci_signature_t *signature);

#ifdef __cplusplus
}
#endif

#endif
