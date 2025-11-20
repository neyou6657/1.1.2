#include "classic_crypto_adapter.h"
#include "algorithm_registry.h"
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_GMSSL
#include <gmssl/sm2.h>
#include <gmssl/sm3.h>
#include <gmssl/rand.h>
#endif

int classic_adapter_init(void) {
    uci_algorithm_impl_t impl;
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "RSA-2048";
    impl.info.id = UCI_ALG_RSA2048;
    impl.info.type = UCI_ALG_TYPE_CLASSIC;
    impl.info.public_key_len = 270;
    impl.info.private_key_len = 1190;
    impl.info.signature_len = 256;
    impl.info.security_level = 112;
    impl.keygen = classic_rsa2048_keygen;
    impl.sign = classic_rsa2048_sign;
    impl.verify = classic_rsa2048_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "ECDSA-P256";
    impl.info.id = UCI_ALG_ECDSA_P256;
    impl.info.type = UCI_ALG_TYPE_CLASSIC;
    impl.info.public_key_len = 65;
    impl.info.private_key_len = 32;
    impl.info.signature_len = 72;
    impl.info.security_level = 128;
    impl.keygen = classic_ecdsa_p256_keygen;
    impl.sign = classic_ecdsa_p256_sign;
    impl.verify = classic_ecdsa_p256_verify;
    registry_register_algorithm(&impl);
    
#ifdef HAVE_GMSSL
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "SM2";
    impl.info.id = UCI_ALG_SM2;
    impl.info.type = UCI_ALG_TYPE_CLASSIC;
    impl.info.public_key_len = 65;
    impl.info.private_key_len = 32;
    impl.info.signature_len = 72;
    impl.info.security_level = 128;
    impl.keygen = classic_sm2_keygen;
    impl.sign = classic_sm2_sign;
    impl.verify = classic_sm2_verify;
    registry_register_algorithm(&impl);
#endif
    
    return UCI_SUCCESS;
}

int classic_adapter_cleanup(void) {
    return UCI_SUCCESS;
}

int classic_rsa2048_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int classic_rsa2048_sign(const uci_keypair_t *keypair, const uint8_t *message,
                         size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int classic_rsa2048_verify(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int classic_ecdsa_p256_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int classic_ecdsa_p256_sign(const uci_keypair_t *keypair, const uint8_t *message,
                            size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int classic_ecdsa_p256_verify(const uci_keypair_t *keypair, const uint8_t *message,
                              size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

#ifdef HAVE_GMSSL

int classic_sm2_keygen(uci_keypair_t *keypair) {
    SM2_KEY sm2_key;
    
    if (sm2_key_generate(&sm2_key) != 1) {
        return UCI_ERROR_INTERNAL;
    }
    
    keypair->private_key = (uint8_t *)malloc(32);
    keypair->public_key = (uint8_t *)malloc(65);
    
    if (!keypair->private_key || !keypair->public_key) {
        free(keypair->private_key);
        free(keypair->public_key);
        return UCI_ERROR_INTERNAL;
    }
    
    SM2_POINT public_point;
    sm2_key_get_public_key(&sm2_key, &public_point);
    
    uint8_t private_key_bytes[32];
    sm2_key_get_private_key(&sm2_key, private_key_bytes);
    
    keypair->public_key[0] = 0x04;
    memcpy(keypair->public_key + 1, &public_point, 64);
    memcpy(keypair->private_key, private_key_bytes, 32);
    
    keypair->public_key_len = 65;
    keypair->private_key_len = 32;
    
    return UCI_SUCCESS;
}

int classic_sm2_sign(const uci_keypair_t *keypair, const uint8_t *message,
                     size_t message_len, uci_signature_t *signature) {
    SM2_KEY sm2_key;
    SM2_SIGNATURE sig;
    SM3_CTX sm3_ctx;
    uint8_t dgst[32];
    
    sm2_key_set_private_key(&sm2_key, keypair->private_key);
    
    SM2_POINT public_point;
    memcpy(&public_point, keypair->public_key + 1, 64);
    sm2_key_set_public_key(&sm2_key, &public_point);
    
    sm3_init(&sm3_ctx);
    sm3_update(&sm3_ctx, message, message_len);
    sm3_finish(&sm3_ctx, dgst);
    
    if (sm2_sign(&sm2_key, dgst, &sig) != 1) {
        return UCI_ERROR_INTERNAL;
    }
    
    signature->data = (uint8_t *)malloc(SM2_signature_typical_size);
    if (!signature->data) {
        return UCI_ERROR_INTERNAL;
    }
    
    uint8_t *p = signature->data;
    if (sm2_signature_to_der(&sig, &p) <= 0) {
        free(signature->data);
        return UCI_ERROR_INTERNAL;
    }
    
    signature->data_len = p - signature->data;
    
    return UCI_SUCCESS;
}

int classic_sm2_verify(const uci_keypair_t *keypair, const uint8_t *message,
                       size_t message_len, const uci_signature_t *signature) {
    SM2_KEY sm2_key;
    SM2_SIGNATURE sig;
    SM3_CTX sm3_ctx;
    uint8_t dgst[32];
    
    SM2_POINT public_point;
    memcpy(&public_point, keypair->public_key + 1, 64);
    sm2_key_set_public_key(&sm2_key, &public_point);
    
    sm3_init(&sm3_ctx);
    sm3_update(&sm3_ctx, message, message_len);
    sm3_finish(&sm3_ctx, dgst);
    
    const uint8_t *p = signature->data;
    if (sm2_signature_from_der(&sig, &p, signature->data_len) != 1) {
        return UCI_ERROR_SIGNATURE_INVALID;
    }
    
    if (sm2_verify(&sm2_key, dgst, &sig) != 1) {
        return UCI_ERROR_SIGNATURE_INVALID;
    }
    
    return UCI_SUCCESS;
}

#else

int classic_sm2_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int classic_sm2_sign(const uci_keypair_t *keypair, const uint8_t *message,
                     size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int classic_sm2_verify(const uci_keypair_t *keypair, const uint8_t *message,
                       size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

#endif
