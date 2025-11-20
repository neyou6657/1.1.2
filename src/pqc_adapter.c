#include "pqc_adapter.h"
#include "algorithm_registry.h"
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LIBOQS
#include <oqs/oqs.h>
#endif

int pqc_adapter_init(void) {
#ifdef HAVE_LIBOQS
    uci_algorithm_impl_t impl;
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "Dilithium2";
    impl.info.id = UCI_ALG_DILITHIUM2;
    impl.info.type = UCI_ALG_TYPE_POST_QUANTUM;
    impl.info.public_key_len = 1312;
    impl.info.private_key_len = 2528;
    impl.info.signature_len = 2420;
    impl.info.security_level = 128;
    impl.keygen = pqc_dilithium2_keygen;
    impl.sign = pqc_dilithium2_sign;
    impl.verify = pqc_dilithium2_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "Dilithium3";
    impl.info.id = UCI_ALG_DILITHIUM3;
    impl.info.type = UCI_ALG_TYPE_POST_QUANTUM;
    impl.info.public_key_len = 1952;
    impl.info.private_key_len = 4000;
    impl.info.signature_len = 3293;
    impl.info.security_level = 192;
    impl.keygen = pqc_dilithium3_keygen;
    impl.sign = pqc_dilithium3_sign;
    impl.verify = pqc_dilithium3_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "Dilithium5";
    impl.info.id = UCI_ALG_DILITHIUM5;
    impl.info.type = UCI_ALG_TYPE_POST_QUANTUM;
    impl.info.public_key_len = 2592;
    impl.info.private_key_len = 4864;
    impl.info.signature_len = 4595;
    impl.info.security_level = 256;
    impl.keygen = pqc_dilithium5_keygen;
    impl.sign = pqc_dilithium5_sign;
    impl.verify = pqc_dilithium5_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "Falcon-512";
    impl.info.id = UCI_ALG_FALCON512;
    impl.info.type = UCI_ALG_TYPE_POST_QUANTUM;
    impl.info.public_key_len = 897;
    impl.info.private_key_len = 1281;
    impl.info.signature_len = 666;
    impl.info.security_level = 128;
    impl.keygen = pqc_falcon512_keygen;
    impl.sign = pqc_falcon512_sign;
    impl.verify = pqc_falcon512_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "Kyber512";
    impl.info.id = UCI_ALG_KYBER512;
    impl.info.type = UCI_ALG_TYPE_POST_QUANTUM;
    impl.info.public_key_len = 800;
    impl.info.private_key_len = 1632;
    impl.info.security_level = 128;
    impl.kem_keygen = pqc_kyber512_keygen;
    impl.kem_encaps = pqc_kyber512_encaps;
    impl.kem_decaps = pqc_kyber512_decaps;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "Kyber768";
    impl.info.id = UCI_ALG_KYBER768;
    impl.info.type = UCI_ALG_TYPE_POST_QUANTUM;
    impl.info.public_key_len = 1184;
    impl.info.private_key_len = 2400;
    impl.info.security_level = 192;
    impl.kem_keygen = pqc_kyber768_keygen;
    impl.kem_encaps = pqc_kyber768_encaps;
    impl.kem_decaps = pqc_kyber768_decaps;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "Kyber1024";
    impl.info.id = UCI_ALG_KYBER1024;
    impl.info.type = UCI_ALG_TYPE_POST_QUANTUM;
    impl.info.public_key_len = 1568;
    impl.info.private_key_len = 3168;
    impl.info.security_level = 256;
    impl.kem_keygen = pqc_kyber1024_keygen;
    impl.kem_encaps = pqc_kyber1024_encaps;
    impl.kem_decaps = pqc_kyber1024_decaps;
    registry_register_algorithm(&impl);
#endif
    
    return UCI_SUCCESS;
}

int pqc_adapter_cleanup(void) {
    return UCI_SUCCESS;
}

#ifdef HAVE_LIBOQS

static int pqc_sig_keygen(const char *alg_name, uci_keypair_t *keypair) {
    OQS_SIG *sig = OQS_SIG_new(alg_name);
    if (!sig) {
        return UCI_ERROR_INTERNAL;
    }
    
    keypair->public_key = (uint8_t *)malloc(sig->length_public_key);
    keypair->private_key = (uint8_t *)malloc(sig->length_secret_key);
    
    if (!keypair->public_key || !keypair->private_key) {
        free(keypair->public_key);
        free(keypair->private_key);
        OQS_SIG_free(sig);
        return UCI_ERROR_INTERNAL;
    }
    
    if (OQS_SIG_keypair(sig, keypair->public_key, keypair->private_key) != OQS_SUCCESS) {
        free(keypair->public_key);
        free(keypair->private_key);
        OQS_SIG_free(sig);
        return UCI_ERROR_INTERNAL;
    }
    
    keypair->public_key_len = sig->length_public_key;
    keypair->private_key_len = sig->length_secret_key;
    
    OQS_SIG_free(sig);
    return UCI_SUCCESS;
}

static int pqc_sig_sign(const char *alg_name, const uci_keypair_t *keypair,
                        const uint8_t *message, size_t message_len,
                        uci_signature_t *signature) {
    OQS_SIG *sig = OQS_SIG_new(alg_name);
    if (!sig) {
        return UCI_ERROR_INTERNAL;
    }
    
    signature->data = (uint8_t *)malloc(sig->length_signature);
    if (!signature->data) {
        OQS_SIG_free(sig);
        return UCI_ERROR_INTERNAL;
    }
    
    size_t sig_len;
    if (OQS_SIG_sign(sig, signature->data, &sig_len, message, message_len,
                     keypair->private_key) != OQS_SUCCESS) {
        free(signature->data);
        OQS_SIG_free(sig);
        return UCI_ERROR_INTERNAL;
    }
    
    signature->data_len = sig_len;
    OQS_SIG_free(sig);
    return UCI_SUCCESS;
}

static int pqc_sig_verify(const char *alg_name, const uci_keypair_t *keypair,
                          const uint8_t *message, size_t message_len,
                          const uci_signature_t *signature) {
    OQS_SIG *sig = OQS_SIG_new(alg_name);
    if (!sig) {
        return UCI_ERROR_INTERNAL;
    }
    
    OQS_STATUS status = OQS_SIG_verify(sig, message, message_len,
                                       signature->data, signature->data_len,
                                       keypair->public_key);
    
    OQS_SIG_free(sig);
    
    if (status != OQS_SUCCESS) {
        return UCI_ERROR_SIGNATURE_INVALID;
    }
    
    return UCI_SUCCESS;
}

static int pqc_kem_keygen(const char *alg_name, uci_keypair_t *keypair) {
    OQS_KEM *kem = OQS_KEM_new(alg_name);
    if (!kem) {
        return UCI_ERROR_INTERNAL;
    }
    
    keypair->public_key = (uint8_t *)malloc(kem->length_public_key);
    keypair->private_key = (uint8_t *)malloc(kem->length_secret_key);
    
    if (!keypair->public_key || !keypair->private_key) {
        free(keypair->public_key);
        free(keypair->private_key);
        OQS_KEM_free(kem);
        return UCI_ERROR_INTERNAL;
    }
    
    if (OQS_KEM_keypair(kem, keypair->public_key, keypair->private_key) != OQS_SUCCESS) {
        free(keypair->public_key);
        free(keypair->private_key);
        OQS_KEM_free(kem);
        return UCI_ERROR_INTERNAL;
    }
    
    keypair->public_key_len = kem->length_public_key;
    keypair->private_key_len = kem->length_secret_key;
    
    OQS_KEM_free(kem);
    return UCI_SUCCESS;
}

static int pqc_kem_encaps(const char *alg_name, const uci_keypair_t *keypair,
                          uci_kem_encaps_result_t *result) {
    OQS_KEM *kem = OQS_KEM_new(alg_name);
    if (!kem) {
        return UCI_ERROR_INTERNAL;
    }
    
    result->shared_secret = (uint8_t *)malloc(kem->length_shared_secret);
    result->ciphertext = (uint8_t *)malloc(kem->length_ciphertext);
    
    if (!result->shared_secret || !result->ciphertext) {
        free(result->shared_secret);
        free(result->ciphertext);
        OQS_KEM_free(kem);
        return UCI_ERROR_INTERNAL;
    }
    
    if (OQS_KEM_encaps(kem, result->ciphertext, result->shared_secret,
                       keypair->public_key) != OQS_SUCCESS) {
        free(result->shared_secret);
        free(result->ciphertext);
        OQS_KEM_free(kem);
        return UCI_ERROR_INTERNAL;
    }
    
    result->shared_secret_len = kem->length_shared_secret;
    result->ciphertext_len = kem->length_ciphertext;
    
    OQS_KEM_free(kem);
    return UCI_SUCCESS;
}

static int pqc_kem_decaps(const char *alg_name, const uci_keypair_t *keypair,
                          const uint8_t *ciphertext, size_t ciphertext_len,
                          uint8_t *shared_secret, size_t *shared_secret_len) {
    OQS_KEM *kem = OQS_KEM_new(alg_name);
    if (!kem) {
        return UCI_ERROR_INTERNAL;
    }
    
    if (*shared_secret_len < kem->length_shared_secret) {
        *shared_secret_len = kem->length_shared_secret;
        OQS_KEM_free(kem);
        return UCI_ERROR_BUFFER_TOO_SMALL;
    }
    
    if (OQS_KEM_decaps(kem, shared_secret, ciphertext, keypair->private_key) != OQS_SUCCESS) {
        OQS_KEM_free(kem);
        return UCI_ERROR_INTERNAL;
    }
    
    *shared_secret_len = kem->length_shared_secret;
    OQS_KEM_free(kem);
    return UCI_SUCCESS;
}

int pqc_dilithium2_keygen(uci_keypair_t *keypair) {
    return pqc_sig_keygen(OQS_SIG_alg_dilithium_2, keypair);
}

int pqc_dilithium2_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return pqc_sig_sign(OQS_SIG_alg_dilithium_2, keypair, message, message_len, signature);
}

int pqc_dilithium2_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return pqc_sig_verify(OQS_SIG_alg_dilithium_2, keypair, message, message_len, signature);
}

int pqc_dilithium3_keygen(uci_keypair_t *keypair) {
    return pqc_sig_keygen(OQS_SIG_alg_dilithium_3, keypair);
}

int pqc_dilithium3_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return pqc_sig_sign(OQS_SIG_alg_dilithium_3, keypair, message, message_len, signature);
}

int pqc_dilithium3_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return pqc_sig_verify(OQS_SIG_alg_dilithium_3, keypair, message, message_len, signature);
}

int pqc_dilithium5_keygen(uci_keypair_t *keypair) {
    return pqc_sig_keygen(OQS_SIG_alg_dilithium_5, keypair);
}

int pqc_dilithium5_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return pqc_sig_sign(OQS_SIG_alg_dilithium_5, keypair, message, message_len, signature);
}

int pqc_dilithium5_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return pqc_sig_verify(OQS_SIG_alg_dilithium_5, keypair, message, message_len, signature);
}

int pqc_falcon512_keygen(uci_keypair_t *keypair) {
    return pqc_sig_keygen(OQS_SIG_alg_falcon_512, keypair);
}

int pqc_falcon512_sign(const uci_keypair_t *keypair, const uint8_t *message,
                       size_t message_len, uci_signature_t *signature) {
    return pqc_sig_sign(OQS_SIG_alg_falcon_512, keypair, message, message_len, signature);
}

int pqc_falcon512_verify(const uci_keypair_t *keypair, const uint8_t *message,
                         size_t message_len, const uci_signature_t *signature) {
    return pqc_sig_verify(OQS_SIG_alg_falcon_512, keypair, message, message_len, signature);
}

int pqc_kyber512_keygen(uci_keypair_t *keypair) {
    return pqc_kem_keygen(OQS_KEM_alg_kyber_512, keypair);
}

int pqc_kyber512_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result) {
    return pqc_kem_encaps(OQS_KEM_alg_kyber_512, keypair, result);
}

int pqc_kyber512_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                        size_t ciphertext_len, uint8_t *shared_secret,
                        size_t *shared_secret_len) {
    return pqc_kem_decaps(OQS_KEM_alg_kyber_512, keypair, ciphertext, ciphertext_len,
                          shared_secret, shared_secret_len);
}

int pqc_kyber768_keygen(uci_keypair_t *keypair) {
    return pqc_kem_keygen(OQS_KEM_alg_kyber_768, keypair);
}

int pqc_kyber768_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result) {
    return pqc_kem_encaps(OQS_KEM_alg_kyber_768, keypair, result);
}

int pqc_kyber768_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                        size_t ciphertext_len, uint8_t *shared_secret,
                        size_t *shared_secret_len) {
    return pqc_kem_decaps(OQS_KEM_alg_kyber_768, keypair, ciphertext, ciphertext_len,
                          shared_secret, shared_secret_len);
}

int pqc_kyber1024_keygen(uci_keypair_t *keypair) {
    return pqc_kem_keygen(OQS_KEM_alg_kyber_1024, keypair);
}

int pqc_kyber1024_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result) {
    return pqc_kem_encaps(OQS_KEM_alg_kyber_1024, keypair, result);
}

int pqc_kyber1024_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                         size_t ciphertext_len, uint8_t *shared_secret,
                         size_t *shared_secret_len) {
    return pqc_kem_decaps(OQS_KEM_alg_kyber_1024, keypair, ciphertext, ciphertext_len,
                          shared_secret, shared_secret_len);
}

#else

int pqc_dilithium2_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_dilithium2_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_dilithium2_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_dilithium3_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_dilithium3_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_dilithium3_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_dilithium5_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_dilithium5_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_dilithium5_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_falcon512_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_falcon512_sign(const uci_keypair_t *keypair, const uint8_t *message,
                       size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_falcon512_verify(const uci_keypair_t *keypair, const uint8_t *message,
                         size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_kyber512_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_kyber512_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_kyber512_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                        size_t ciphertext_len, uint8_t *shared_secret,
                        size_t *shared_secret_len) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_kyber768_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_kyber768_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_kyber768_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                        size_t ciphertext_len, uint8_t *shared_secret,
                        size_t *shared_secret_len) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_kyber1024_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_kyber1024_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int pqc_kyber1024_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext,
                         size_t ciphertext_len, uint8_t *shared_secret,
                         size_t *shared_secret_len) {
    return UCI_ERROR_NOT_SUPPORTED;
}

#endif
