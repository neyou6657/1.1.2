#ifndef UNIFIED_CRYPTO_INTERFACE_H
#define UNIFIED_CRYPTO_INTERFACE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UCI_SUCCESS 0
#define UCI_ERROR_INVALID_PARAM -1
#define UCI_ERROR_NOT_SUPPORTED -2
#define UCI_ERROR_BUFFER_TOO_SMALL -3
#define UCI_ERROR_ALGORITHM_NOT_FOUND -4
#define UCI_ERROR_INTERNAL -5
#define UCI_ERROR_SIGNATURE_INVALID -6

typedef enum {
    UCI_ALG_TYPE_CLASSIC = 0,
    UCI_ALG_TYPE_POST_QUANTUM = 1,
    UCI_ALG_TYPE_HYBRID = 2
} uci_algorithm_type_t;

typedef enum {
    UCI_OP_KEYGEN = 0,
    UCI_OP_SIGN = 1,
    UCI_OP_VERIFY = 2,
    UCI_OP_ENCRYPT = 3,
    UCI_OP_DECRYPT = 4,
    UCI_OP_KEM_KEYGEN = 5,
    UCI_OP_KEM_ENCAPS = 6,
    UCI_OP_KEM_DECAPS = 7
} uci_operation_t;

typedef enum {
    UCI_ALG_RSA2048 = 100,
    UCI_ALG_RSA3072 = 101,
    UCI_ALG_RSA4096 = 102,
    UCI_ALG_ECDSA_P256 = 110,
    UCI_ALG_ECDSA_P384 = 111,
    UCI_ALG_SM2 = 120,
    UCI_ALG_SM3 = 121,
    UCI_ALG_SM4 = 122,
    
    UCI_ALG_DILITHIUM2 = 200,
    UCI_ALG_DILITHIUM3 = 201,
    UCI_ALG_DILITHIUM5 = 202,
    UCI_ALG_FALCON512 = 210,
    UCI_ALG_FALCON1024 = 211,
    UCI_ALG_SPHINCS_SHA256_128F = 220,
    UCI_ALG_SPHINCS_SHA256_192F = 221,
    UCI_ALG_SPHINCS_SHA256_256F = 222,
    
    UCI_ALG_KYBER512 = 300,
    UCI_ALG_KYBER768 = 301,
    UCI_ALG_KYBER1024 = 302,
    UCI_ALG_NTRU_HPS2048509 = 310,
    UCI_ALG_NTRU_HPS2048677 = 311,
    UCI_ALG_NTRU_HPS4096821 = 312,
    UCI_ALG_SABER_LIGHTSABER = 320,
    UCI_ALG_SABER_SABER = 321,
    UCI_ALG_SABER_FIRESABER = 322,
    
    UCI_ALG_HYBRID_RSA_DILITHIUM = 400,
    UCI_ALG_HYBRID_ECDSA_DILITHIUM = 401,
    UCI_ALG_HYBRID_RSA_KYBER = 410,
    UCI_ALG_HYBRID_ECDH_KYBER = 411
} uci_algorithm_id_t;

typedef struct {
    uci_algorithm_id_t algorithm;
    uci_algorithm_type_t type;
    uint8_t *public_key;
    size_t public_key_len;
    uint8_t *private_key;
    size_t private_key_len;
} uci_keypair_t;

typedef struct {
    uci_algorithm_id_t algorithm;
    uint8_t *data;
    size_t data_len;
} uci_signature_t;

typedef struct {
    uci_algorithm_id_t algorithm;
    uint8_t *ciphertext;
    size_t ciphertext_len;
} uci_ciphertext_t;

typedef struct {
    uint8_t *shared_secret;
    size_t shared_secret_len;
    uint8_t *ciphertext;
    size_t ciphertext_len;
} uci_kem_encaps_result_t;

typedef struct {
    const char *name;
    uci_algorithm_id_t id;
    uci_algorithm_type_t type;
    size_t public_key_len;
    size_t private_key_len;
    size_t signature_len;
    size_t ciphertext_overhead;
    uint32_t security_level;
} uci_algorithm_info_t;

int uci_init(void);
int uci_cleanup(void);

int uci_get_algorithm_info(uci_algorithm_id_t algorithm, uci_algorithm_info_t *info);
int uci_list_algorithms(uci_algorithm_type_t type, uci_algorithm_id_t *algorithms, size_t *count);

int uci_keygen(uci_algorithm_id_t algorithm, uci_keypair_t *keypair);
int uci_keypair_free(uci_keypair_t *keypair);

int uci_sign(const uci_keypair_t *keypair, const uint8_t *message, size_t message_len,
             uci_signature_t *signature);
int uci_verify(const uci_keypair_t *keypair, const uint8_t *message, size_t message_len,
               const uci_signature_t *signature);
int uci_signature_free(uci_signature_t *signature);

int uci_encrypt(const uci_keypair_t *keypair, const uint8_t *plaintext, size_t plaintext_len,
                uci_ciphertext_t *ciphertext);
int uci_decrypt(const uci_keypair_t *keypair, const uci_ciphertext_t *ciphertext,
                uint8_t *plaintext, size_t *plaintext_len);
int uci_ciphertext_free(uci_ciphertext_t *ciphertext);

int uci_kem_keygen(uci_algorithm_id_t algorithm, uci_keypair_t *keypair);
int uci_kem_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result);
int uci_kem_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext, size_t ciphertext_len,
                   uint8_t *shared_secret, size_t *shared_secret_len);
int uci_kem_encaps_result_free(uci_kem_encaps_result_t *result);

int uci_hybrid_sign(uci_algorithm_id_t algorithm, 
                    const uci_keypair_t *classic_keypair,
                    const uci_keypair_t *pq_keypair,
                    const uint8_t *message, size_t message_len,
                    uci_signature_t *signature);

int uci_hybrid_verify(uci_algorithm_id_t algorithm,
                      const uci_keypair_t *classic_keypair,
                      const uci_keypair_t *pq_keypair,
                      const uint8_t *message, size_t message_len,
                      const uci_signature_t *signature);

const char *uci_get_error_string(int error_code);

#ifdef __cplusplus
}
#endif

#endif
