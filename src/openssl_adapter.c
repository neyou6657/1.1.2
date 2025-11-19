#include "openssl_adapter.h"
#include "algorithm_registry.h"
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_OPENSSL
#include <openssl/rsa.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#endif

int openssl_adapter_init(void) {
#ifdef HAVE_OPENSSL
    uci_algorithm_impl_t impl;
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "RSA-2048";
    impl.info.id = UCI_ALG_RSA2048;
    impl.info.type = UCI_ALG_TYPE_CLASSIC;
    impl.info.public_key_len = 294;
    impl.info.private_key_len = 1192;
    impl.info.signature_len = 256;
    impl.info.security_level = 112;
    impl.keygen = openssl_rsa2048_keygen;
    impl.sign = openssl_rsa2048_sign;
    impl.verify = openssl_rsa2048_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "RSA-3072";
    impl.info.id = UCI_ALG_RSA3072;
    impl.info.type = UCI_ALG_TYPE_CLASSIC;
    impl.info.public_key_len = 422;
    impl.info.private_key_len = 1776;
    impl.info.signature_len = 384;
    impl.info.security_level = 128;
    impl.keygen = openssl_rsa3072_keygen;
    impl.sign = openssl_rsa3072_sign;
    impl.verify = openssl_rsa3072_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "RSA-4096";
    impl.info.id = UCI_ALG_RSA4096;
    impl.info.type = UCI_ALG_TYPE_CLASSIC;
    impl.info.public_key_len = 550;
    impl.info.private_key_len = 2364;
    impl.info.signature_len = 512;
    impl.info.security_level = 128;
    impl.keygen = openssl_rsa4096_keygen;
    impl.sign = openssl_rsa4096_sign;
    impl.verify = openssl_rsa4096_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "ECDSA-P256";
    impl.info.id = UCI_ALG_ECDSA_P256;
    impl.info.type = UCI_ALG_TYPE_CLASSIC;
    impl.info.public_key_len = 91;
    impl.info.private_key_len = 121;
    impl.info.signature_len = 72;
    impl.info.security_level = 128;
    impl.keygen = openssl_ecdsa_p256_keygen;
    impl.sign = openssl_ecdsa_p256_sign;
    impl.verify = openssl_ecdsa_p256_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "ECDSA-P384";
    impl.info.id = UCI_ALG_ECDSA_P384;
    impl.info.type = UCI_ALG_TYPE_CLASSIC;
    impl.info.public_key_len = 120;
    impl.info.private_key_len = 167;
    impl.info.signature_len = 104;
    impl.info.security_level = 192;
    impl.keygen = openssl_ecdsa_p384_keygen;
    impl.sign = openssl_ecdsa_p384_sign;
    impl.verify = openssl_ecdsa_p384_verify;
    registry_register_algorithm(&impl);
#endif
    
    return UCI_SUCCESS;
}

int openssl_adapter_cleanup(void) {
    return UCI_SUCCESS;
}

#ifdef HAVE_OPENSSL

static int openssl_rsa_keygen(int bits, uci_keypair_t *keypair) {
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    
    if (!ctx) {
        return UCI_ERROR_INTERNAL;
    }
    
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return UCI_ERROR_INTERNAL;
    }
    
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return UCI_ERROR_INTERNAL;
    }
    
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return UCI_ERROR_INTERNAL;
    }
    
    EVP_PKEY_CTX_free(ctx);
    
    unsigned char *pub_der = NULL;
    int pub_len = i2d_PUBKEY(pkey, &pub_der);
    if (pub_len < 0) {
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    unsigned char *priv_der = NULL;
    int priv_len = i2d_PrivateKey(pkey, &priv_der);
    if (priv_len < 0) {
        OPENSSL_free(pub_der);
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    keypair->public_key = (uint8_t *)malloc(pub_len);
    keypair->private_key = (uint8_t *)malloc(priv_len);
    
    if (!keypair->public_key || !keypair->private_key) {
        free(keypair->public_key);
        free(keypair->private_key);
        OPENSSL_free(pub_der);
        OPENSSL_free(priv_der);
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    memcpy(keypair->public_key, pub_der, pub_len);
    memcpy(keypair->private_key, priv_der, priv_len);
    keypair->public_key_len = pub_len;
    keypair->private_key_len = priv_len;
    
    OPENSSL_free(pub_der);
    OPENSSL_free(priv_der);
    EVP_PKEY_free(pkey);
    
    return UCI_SUCCESS;
}

static int openssl_rsa_sign(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, uci_signature_t *signature) {
    const unsigned char *p = keypair->private_key;
    EVP_PKEY *pkey = d2i_PrivateKey(EVP_PKEY_RSA, NULL, &p, keypair->private_key_len);
    
    if (!pkey) {
        return UCI_ERROR_INTERNAL;
    }
    
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) {
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    if (EVP_DigestSignInit(md_ctx, NULL, EVP_sha256(), NULL, pkey) <= 0) {
        EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    size_t sig_len;
    if (EVP_DigestSign(md_ctx, NULL, &sig_len, message, message_len) <= 0) {
        EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    signature->data = (uint8_t *)malloc(sig_len);
    if (!signature->data) {
        EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    if (EVP_DigestSign(md_ctx, signature->data, &sig_len, message, message_len) <= 0) {
        free(signature->data);
        EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    signature->data_len = sig_len;
    
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(pkey);
    
    return UCI_SUCCESS;
}

static int openssl_rsa_verify(const uci_keypair_t *keypair, const uint8_t *message,
                              size_t message_len, const uci_signature_t *signature) {
    const unsigned char *p = keypair->public_key;
    EVP_PKEY *pkey = d2i_PUBKEY(NULL, &p, keypair->public_key_len);
    
    if (!pkey) {
        return UCI_ERROR_INTERNAL;
    }
    
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) {
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    if (EVP_DigestVerifyInit(md_ctx, NULL, EVP_sha256(), NULL, pkey) <= 0) {
        EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    int result = EVP_DigestVerify(md_ctx, signature->data, signature->data_len, 
                                  message, message_len);
    
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(pkey);
    
    if (result != 1) {
        return UCI_ERROR_SIGNATURE_INVALID;
    }
    
    return UCI_SUCCESS;
}

static int openssl_ecdsa_keygen(int nid, uci_keypair_t *keypair) {
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    
    if (!ctx) {
        return UCI_ERROR_INTERNAL;
    }
    
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return UCI_ERROR_INTERNAL;
    }
    
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, nid) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return UCI_ERROR_INTERNAL;
    }
    
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return UCI_ERROR_INTERNAL;
    }
    
    EVP_PKEY_CTX_free(ctx);
    
    unsigned char *pub_der = NULL;
    int pub_len = i2d_PUBKEY(pkey, &pub_der);
    if (pub_len < 0) {
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    unsigned char *priv_der = NULL;
    int priv_len = i2d_PrivateKey(pkey, &priv_der);
    if (priv_len < 0) {
        OPENSSL_free(pub_der);
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    keypair->public_key = (uint8_t *)malloc(pub_len);
    keypair->private_key = (uint8_t *)malloc(priv_len);
    
    if (!keypair->public_key || !keypair->private_key) {
        free(keypair->public_key);
        free(keypair->private_key);
        OPENSSL_free(pub_der);
        OPENSSL_free(priv_der);
        EVP_PKEY_free(pkey);
        return UCI_ERROR_INTERNAL;
    }
    
    memcpy(keypair->public_key, pub_der, pub_len);
    memcpy(keypair->private_key, priv_der, priv_len);
    keypair->public_key_len = pub_len;
    keypair->private_key_len = priv_len;
    
    OPENSSL_free(pub_der);
    OPENSSL_free(priv_der);
    EVP_PKEY_free(pkey);
    
    return UCI_SUCCESS;
}

static int openssl_ecdsa_sign(const uci_keypair_t *keypair, const uint8_t *message,
                             size_t message_len, uci_signature_t *signature) {
    return openssl_rsa_sign(keypair, message, message_len, signature);
}

static int openssl_ecdsa_verify(const uci_keypair_t *keypair, const uint8_t *message,
                               size_t message_len, const uci_signature_t *signature) {
    return openssl_rsa_verify(keypair, message, message_len, signature);
}

int openssl_rsa2048_keygen(uci_keypair_t *keypair) {
    return openssl_rsa_keygen(2048, keypair);
}

int openssl_rsa2048_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return openssl_rsa_sign(keypair, message, message_len, signature);
}

int openssl_rsa2048_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return openssl_rsa_verify(keypair, message, message_len, signature);
}

int openssl_rsa3072_keygen(uci_keypair_t *keypair) {
    return openssl_rsa_keygen(3072, keypair);
}

int openssl_rsa3072_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return openssl_rsa_sign(keypair, message, message_len, signature);
}

int openssl_rsa3072_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return openssl_rsa_verify(keypair, message, message_len, signature);
}

int openssl_rsa4096_keygen(uci_keypair_t *keypair) {
    return openssl_rsa_keygen(4096, keypair);
}

int openssl_rsa4096_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return openssl_rsa_sign(keypair, message, message_len, signature);
}

int openssl_rsa4096_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return openssl_rsa_verify(keypair, message, message_len, signature);
}

int openssl_ecdsa_p256_keygen(uci_keypair_t *keypair) {
    return openssl_ecdsa_keygen(NID_X9_62_prime256v1, keypair);
}

int openssl_ecdsa_p256_sign(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, uci_signature_t *signature) {
    return openssl_ecdsa_sign(keypair, message, message_len, signature);
}

int openssl_ecdsa_p256_verify(const uci_keypair_t *keypair, const uint8_t *message,
                             size_t message_len, const uci_signature_t *signature) {
    return openssl_ecdsa_verify(keypair, message, message_len, signature);
}

int openssl_ecdsa_p384_keygen(uci_keypair_t *keypair) {
    return openssl_ecdsa_keygen(NID_secp384r1, keypair);
}

int openssl_ecdsa_p384_sign(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, uci_signature_t *signature) {
    return openssl_ecdsa_sign(keypair, message, message_len, signature);
}

int openssl_ecdsa_p384_verify(const uci_keypair_t *keypair, const uint8_t *message,
                             size_t message_len, const uci_signature_t *signature) {
    return openssl_ecdsa_verify(keypair, message, message_len, signature);
}

#else

int openssl_rsa2048_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_rsa2048_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_rsa2048_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_rsa3072_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_rsa3072_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_rsa3072_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_rsa4096_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_rsa4096_sign(const uci_keypair_t *keypair, const uint8_t *message,
                        size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_rsa4096_verify(const uci_keypair_t *keypair, const uint8_t *message,
                          size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_ecdsa_p256_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_ecdsa_p256_sign(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_ecdsa_p256_verify(const uci_keypair_t *keypair, const uint8_t *message,
                             size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_ecdsa_p384_keygen(uci_keypair_t *keypair) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_ecdsa_p384_sign(const uci_keypair_t *keypair, const uint8_t *message,
                           size_t message_len, uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

int openssl_ecdsa_p384_verify(const uci_keypair_t *keypair, const uint8_t *message,
                             size_t message_len, const uci_signature_t *signature) {
    return UCI_ERROR_NOT_SUPPORTED;
}

#endif
