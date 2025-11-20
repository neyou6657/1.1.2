#ifndef ALGORITHM_REGISTRY_H
#define ALGORITHM_REGISTRY_H

#include "unified_crypto_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*uci_keygen_func_t)(uci_keypair_t *keypair);
typedef int (*uci_sign_func_t)(const uci_keypair_t *keypair, const uint8_t *message, 
                               size_t message_len, uci_signature_t *signature);
typedef int (*uci_verify_func_t)(const uci_keypair_t *keypair, const uint8_t *message,
                                 size_t message_len, const uci_signature_t *signature);
typedef int (*uci_encrypt_func_t)(const uci_keypair_t *keypair, const uint8_t *plaintext,
                                  size_t plaintext_len, uci_ciphertext_t *ciphertext);
typedef int (*uci_decrypt_func_t)(const uci_keypair_t *keypair, const uci_ciphertext_t *ciphertext,
                                  uint8_t *plaintext, size_t *plaintext_len);
typedef int (*uci_kem_keygen_func_t)(uci_keypair_t *keypair);
typedef int (*uci_kem_encaps_func_t)(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result);
typedef int (*uci_kem_decaps_func_t)(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                                     size_t ciphertext_len, uint8_t *shared_secret,
                                     size_t *shared_secret_len);

typedef struct {
    uci_algorithm_info_t info;
    uci_keygen_func_t keygen;
    uci_sign_func_t sign;
    uci_verify_func_t verify;
    uci_encrypt_func_t encrypt;
    uci_decrypt_func_t decrypt;
    uci_kem_keygen_func_t kem_keygen;
    uci_kem_encaps_func_t kem_encaps;
    uci_kem_decaps_func_t kem_decaps;
} uci_algorithm_impl_t;

int registry_init(void);
int registry_cleanup(void);

int registry_register_algorithm(const uci_algorithm_impl_t *impl);
const uci_algorithm_impl_t *registry_get_algorithm(uci_algorithm_id_t algorithm);
int registry_list_algorithms(uci_algorithm_type_t type, uci_algorithm_id_t *algorithms, size_t *count);

#ifdef __cplusplus
}
#endif

#endif
