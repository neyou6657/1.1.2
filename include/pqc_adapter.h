#ifndef PQC_ADAPTER_H
#define PQC_ADAPTER_H

#include "unified_crypto_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

int pqc_adapter_init(void);
int pqc_adapter_cleanup(void);

int pqc_dilithium2_keygen(uci_keypair_t *keypair);
int pqc_dilithium2_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature);
int pqc_dilithium2_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature);

int pqc_dilithium3_keygen(uci_keypair_t *keypair);
int pqc_dilithium3_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature);
int pqc_dilithium3_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature);

int pqc_dilithium5_keygen(uci_keypair_t *keypair);
int pqc_dilithium5_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature);
int pqc_dilithium5_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature);

int pqc_falcon512_keygen(uci_keypair_t *keypair);
int pqc_falcon512_sign(const uci_keypair_t *keypair, const uint8_t *message,
                       size_t message_len, uci_signature_t *signature);
int pqc_falcon512_verify(const uci_keypair_t *keypair, const uint8_t *message,
                         size_t message_len, const uci_signature_t *signature);

int pqc_kyber512_keygen(uci_keypair_t *keypair);
int pqc_kyber512_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result);
int pqc_kyber512_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                        size_t ciphertext_len, uint8_t *shared_secret,
                        size_t *shared_secret_len);

int pqc_kyber768_keygen(uci_keypair_t *keypair);
int pqc_kyber768_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result);
int pqc_kyber768_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                        size_t ciphertext_len, uint8_t *shared_secret,
                        size_t *shared_secret_len);

int pqc_kyber1024_keygen(uci_keypair_t *keypair);
int pqc_kyber1024_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result);
int pqc_kyber1024_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                         size_t ciphertext_len, uint8_t *shared_secret,
                         size_t *shared_secret_len);

#ifdef __cplusplus
}
#endif

#endif
