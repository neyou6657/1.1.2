#include "hybrid_crypto.h"
#include "algorithm_registry.h"
#include "classic_crypto_adapter.h"
#include "pqc_adapter.h"
#include <stdlib.h>
#include <string.h>

int hybrid_adapter_init(void) {
    uci_algorithm_impl_t impl;
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "Hybrid-RSA-Dilithium";
    impl.info.id = UCI_ALG_HYBRID_RSA_DILITHIUM;
    impl.info.type = UCI_ALG_TYPE_HYBRID;
    impl.info.public_key_len = 1582;
    impl.info.private_key_len = 3718;
    impl.info.signature_len = 2676;
    impl.info.security_level = 128;
    impl.keygen = hybrid_rsa_dilithium_keygen;
    impl.sign = hybrid_rsa_dilithium_sign;
    impl.verify = hybrid_rsa_dilithium_verify;
    registry_register_algorithm(&impl);
    
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "Hybrid-ECDSA-Dilithium";
    impl.info.id = UCI_ALG_HYBRID_ECDSA_DILITHIUM;
    impl.info.type = UCI_ALG_TYPE_HYBRID;
    impl.info.public_key_len = 1377;
    impl.info.private_key_len = 2560;
    impl.info.signature_len = 2492;
    impl.info.security_level = 128;
    impl.keygen = hybrid_ecdsa_dilithium_keygen;
    impl.sign = hybrid_ecdsa_dilithium_sign;
    impl.verify = hybrid_ecdsa_dilithium_verify;
    registry_register_algorithm(&impl);
    
    return UCI_SUCCESS;
}

int hybrid_adapter_cleanup(void) {
    return UCI_SUCCESS;
}

int hybrid_rsa_dilithium_keygen(uci_keypair_t *keypair) {
    uci_keypair_t rsa_keypair, dilithium_keypair;
    int ret;
    
    ret = classic_rsa2048_keygen(&rsa_keypair);
    if (ret != UCI_SUCCESS) {
        return ret;
    }
    
    ret = pqc_dilithium2_keygen(&dilithium_keypair);
    if (ret != UCI_SUCCESS) {
        uci_keypair_free(&rsa_keypair);
        return ret;
    }
    
    size_t total_public_len = sizeof(size_t) * 2 + rsa_keypair.public_key_len + dilithium_keypair.public_key_len;
    size_t total_private_len = sizeof(size_t) * 2 + rsa_keypair.private_key_len + dilithium_keypair.private_key_len;
    
    keypair->public_key = (uint8_t *)malloc(total_public_len);
    keypair->private_key = (uint8_t *)malloc(total_private_len);
    
    if (!keypair->public_key || !keypair->private_key) {
        free(keypair->public_key);
        free(keypair->private_key);
        uci_keypair_free(&rsa_keypair);
        uci_keypair_free(&dilithium_keypair);
        return UCI_ERROR_INTERNAL;
    }
    
    uint8_t *p = keypair->public_key;
    memcpy(p, &rsa_keypair.public_key_len, sizeof(size_t));
    p += sizeof(size_t);
    memcpy(p, rsa_keypair.public_key, rsa_keypair.public_key_len);
    p += rsa_keypair.public_key_len;
    memcpy(p, &dilithium_keypair.public_key_len, sizeof(size_t));
    p += sizeof(size_t);
    memcpy(p, dilithium_keypair.public_key, dilithium_keypair.public_key_len);
    
    p = keypair->private_key;
    memcpy(p, &rsa_keypair.private_key_len, sizeof(size_t));
    p += sizeof(size_t);
    memcpy(p, rsa_keypair.private_key, rsa_keypair.private_key_len);
    p += rsa_keypair.private_key_len;
    memcpy(p, &dilithium_keypair.private_key_len, sizeof(size_t));
    p += sizeof(size_t);
    memcpy(p, dilithium_keypair.private_key, dilithium_keypair.private_key_len);
    
    keypair->public_key_len = total_public_len;
    keypair->private_key_len = total_private_len;
    
    uci_keypair_free(&rsa_keypair);
    uci_keypair_free(&dilithium_keypair);
    
    return UCI_SUCCESS;
}

int hybrid_rsa_dilithium_sign(const uci_keypair_t *keypair, const uint8_t *message,
                              size_t message_len, uci_signature_t *signature) {
    uci_keypair_t rsa_keypair, dilithium_keypair;
    uci_signature_t rsa_sig, dilithium_sig;
    int ret;
    
    const uint8_t *p = keypair->public_key;
    memcpy(&rsa_keypair.public_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    rsa_keypair.public_key = (uint8_t *)p;
    p += rsa_keypair.public_key_len;
    memcpy(&dilithium_keypair.public_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    dilithium_keypair.public_key = (uint8_t *)p;
    
    p = keypair->private_key;
    memcpy(&rsa_keypair.private_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    rsa_keypair.private_key = (uint8_t *)p;
    p += rsa_keypair.private_key_len;
    memcpy(&dilithium_keypair.private_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    dilithium_keypair.private_key = (uint8_t *)p;
    
    rsa_keypair.algorithm = UCI_ALG_RSA2048;
    dilithium_keypair.algorithm = UCI_ALG_DILITHIUM2;
    
    ret = classic_rsa2048_sign(&rsa_keypair, message, message_len, &rsa_sig);
    if (ret != UCI_SUCCESS) {
        return ret;
    }
    
    ret = pqc_dilithium2_sign(&dilithium_keypair, message, message_len, &dilithium_sig);
    if (ret != UCI_SUCCESS) {
        uci_signature_free(&rsa_sig);
        return ret;
    }
    
    size_t total_sig_len = sizeof(size_t) * 2 + rsa_sig.data_len + dilithium_sig.data_len;
    signature->data = (uint8_t *)malloc(total_sig_len);
    
    if (!signature->data) {
        uci_signature_free(&rsa_sig);
        uci_signature_free(&dilithium_sig);
        return UCI_ERROR_INTERNAL;
    }
    
    uint8_t *sig_p = signature->data;
    memcpy(sig_p, &rsa_sig.data_len, sizeof(size_t));
    sig_p += sizeof(size_t);
    memcpy(sig_p, rsa_sig.data, rsa_sig.data_len);
    sig_p += rsa_sig.data_len;
    memcpy(sig_p, &dilithium_sig.data_len, sizeof(size_t));
    sig_p += sizeof(size_t);
    memcpy(sig_p, dilithium_sig.data, dilithium_sig.data_len);
    
    signature->data_len = total_sig_len;
    
    uci_signature_free(&rsa_sig);
    uci_signature_free(&dilithium_sig);
    
    return UCI_SUCCESS;
}

int hybrid_rsa_dilithium_verify(const uci_keypair_t *keypair, const uint8_t *message,
                                size_t message_len, const uci_signature_t *signature) {
    uci_keypair_t rsa_keypair, dilithium_keypair;
    uci_signature_t rsa_sig, dilithium_sig;
    int ret;
    
    const uint8_t *p = keypair->public_key;
    memcpy(&rsa_keypair.public_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    rsa_keypair.public_key = (uint8_t *)p;
    p += rsa_keypair.public_key_len;
    memcpy(&dilithium_keypair.public_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    dilithium_keypair.public_key = (uint8_t *)p;
    
    p = keypair->private_key;
    memcpy(&rsa_keypair.private_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    rsa_keypair.private_key = (uint8_t *)p;
    p += rsa_keypair.private_key_len;
    memcpy(&dilithium_keypair.private_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    dilithium_keypair.private_key = (uint8_t *)p;
    
    rsa_keypair.algorithm = UCI_ALG_RSA2048;
    dilithium_keypair.algorithm = UCI_ALG_DILITHIUM2;
    
    const uint8_t *sig_p = signature->data;
    memcpy(&rsa_sig.data_len, sig_p, sizeof(size_t));
    sig_p += sizeof(size_t);
    rsa_sig.data = (uint8_t *)sig_p;
    sig_p += rsa_sig.data_len;
    memcpy(&dilithium_sig.data_len, sig_p, sizeof(size_t));
    sig_p += sizeof(size_t);
    dilithium_sig.data = (uint8_t *)sig_p;
    
    ret = classic_rsa2048_verify(&rsa_keypair, message, message_len, &rsa_sig);
    if (ret != UCI_SUCCESS) {
        return ret;
    }
    
    ret = pqc_dilithium2_verify(&dilithium_keypair, message, message_len, &dilithium_sig);
    if (ret != UCI_SUCCESS) {
        return ret;
    }
    
    return UCI_SUCCESS;
}

int hybrid_ecdsa_dilithium_keygen(uci_keypair_t *keypair) {
    uci_keypair_t ecdsa_keypair, dilithium_keypair;
    int ret;
    
    ret = classic_ecdsa_p256_keygen(&ecdsa_keypair);
    if (ret != UCI_SUCCESS) {
        return ret;
    }
    
    ret = pqc_dilithium2_keygen(&dilithium_keypair);
    if (ret != UCI_SUCCESS) {
        uci_keypair_free(&ecdsa_keypair);
        return ret;
    }
    
    size_t total_public_len = sizeof(size_t) * 2 + ecdsa_keypair.public_key_len + dilithium_keypair.public_key_len;
    size_t total_private_len = sizeof(size_t) * 2 + ecdsa_keypair.private_key_len + dilithium_keypair.private_key_len;
    
    keypair->public_key = (uint8_t *)malloc(total_public_len);
    keypair->private_key = (uint8_t *)malloc(total_private_len);
    
    if (!keypair->public_key || !keypair->private_key) {
        free(keypair->public_key);
        free(keypair->private_key);
        uci_keypair_free(&ecdsa_keypair);
        uci_keypair_free(&dilithium_keypair);
        return UCI_ERROR_INTERNAL;
    }
    
    uint8_t *p = keypair->public_key;
    memcpy(p, &ecdsa_keypair.public_key_len, sizeof(size_t));
    p += sizeof(size_t);
    memcpy(p, ecdsa_keypair.public_key, ecdsa_keypair.public_key_len);
    p += ecdsa_keypair.public_key_len;
    memcpy(p, &dilithium_keypair.public_key_len, sizeof(size_t));
    p += sizeof(size_t);
    memcpy(p, dilithium_keypair.public_key, dilithium_keypair.public_key_len);
    
    p = keypair->private_key;
    memcpy(p, &ecdsa_keypair.private_key_len, sizeof(size_t));
    p += sizeof(size_t);
    memcpy(p, ecdsa_keypair.private_key, ecdsa_keypair.private_key_len);
    p += ecdsa_keypair.private_key_len;
    memcpy(p, &dilithium_keypair.private_key_len, sizeof(size_t));
    p += sizeof(size_t);
    memcpy(p, dilithium_keypair.private_key, dilithium_keypair.private_key_len);
    
    keypair->public_key_len = total_public_len;
    keypair->private_key_len = total_private_len;
    
    uci_keypair_free(&ecdsa_keypair);
    uci_keypair_free(&dilithium_keypair);
    
    return UCI_SUCCESS;
}

int hybrid_ecdsa_dilithium_sign(const uci_keypair_t *keypair, const uint8_t *message,
                                size_t message_len, uci_signature_t *signature) {
    uci_keypair_t ecdsa_keypair, dilithium_keypair;
    uci_signature_t ecdsa_sig, dilithium_sig;
    int ret;
    
    const uint8_t *p = keypair->public_key;
    memcpy(&ecdsa_keypair.public_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    ecdsa_keypair.public_key = (uint8_t *)p;
    p += ecdsa_keypair.public_key_len;
    memcpy(&dilithium_keypair.public_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    dilithium_keypair.public_key = (uint8_t *)p;
    
    p = keypair->private_key;
    memcpy(&ecdsa_keypair.private_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    ecdsa_keypair.private_key = (uint8_t *)p;
    p += ecdsa_keypair.private_key_len;
    memcpy(&dilithium_keypair.private_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    dilithium_keypair.private_key = (uint8_t *)p;
    
    ecdsa_keypair.algorithm = UCI_ALG_ECDSA_P256;
    dilithium_keypair.algorithm = UCI_ALG_DILITHIUM2;
    
    ret = classic_ecdsa_p256_sign(&ecdsa_keypair, message, message_len, &ecdsa_sig);
    if (ret != UCI_SUCCESS) {
        return ret;
    }
    
    ret = pqc_dilithium2_sign(&dilithium_keypair, message, message_len, &dilithium_sig);
    if (ret != UCI_SUCCESS) {
        uci_signature_free(&ecdsa_sig);
        return ret;
    }
    
    size_t total_sig_len = sizeof(size_t) * 2 + ecdsa_sig.data_len + dilithium_sig.data_len;
    signature->data = (uint8_t *)malloc(total_sig_len);
    
    if (!signature->data) {
        uci_signature_free(&ecdsa_sig);
        uci_signature_free(&dilithium_sig);
        return UCI_ERROR_INTERNAL;
    }
    
    uint8_t *sig_p = signature->data;
    memcpy(sig_p, &ecdsa_sig.data_len, sizeof(size_t));
    sig_p += sizeof(size_t);
    memcpy(sig_p, ecdsa_sig.data, ecdsa_sig.data_len);
    sig_p += ecdsa_sig.data_len;
    memcpy(sig_p, &dilithium_sig.data_len, sizeof(size_t));
    sig_p += sizeof(size_t);
    memcpy(sig_p, dilithium_sig.data, dilithium_sig.data_len);
    
    signature->data_len = total_sig_len;
    
    uci_signature_free(&ecdsa_sig);
    uci_signature_free(&dilithium_sig);
    
    return UCI_SUCCESS;
}

int hybrid_ecdsa_dilithium_verify(const uci_keypair_t *keypair, const uint8_t *message,
                                  size_t message_len, const uci_signature_t *signature) {
    uci_keypair_t ecdsa_keypair, dilithium_keypair;
    uci_signature_t ecdsa_sig, dilithium_sig;
    int ret;
    
    const uint8_t *p = keypair->public_key;
    memcpy(&ecdsa_keypair.public_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    ecdsa_keypair.public_key = (uint8_t *)p;
    p += ecdsa_keypair.public_key_len;
    memcpy(&dilithium_keypair.public_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    dilithium_keypair.public_key = (uint8_t *)p;
    
    p = keypair->private_key;
    memcpy(&ecdsa_keypair.private_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    ecdsa_keypair.private_key = (uint8_t *)p;
    p += ecdsa_keypair.private_key_len;
    memcpy(&dilithium_keypair.private_key_len, p, sizeof(size_t));
    p += sizeof(size_t);
    dilithium_keypair.private_key = (uint8_t *)p;
    
    ecdsa_keypair.algorithm = UCI_ALG_ECDSA_P256;
    dilithium_keypair.algorithm = UCI_ALG_DILITHIUM2;
    
    const uint8_t *sig_p = signature->data;
    memcpy(&ecdsa_sig.data_len, sig_p, sizeof(size_t));
    sig_p += sizeof(size_t);
    ecdsa_sig.data = (uint8_t *)sig_p;
    sig_p += ecdsa_sig.data_len;
    memcpy(&dilithium_sig.data_len, sig_p, sizeof(size_t));
    sig_p += sizeof(size_t);
    dilithium_sig.data = (uint8_t *)sig_p;
    
    ret = classic_ecdsa_p256_verify(&ecdsa_keypair, message, message_len, &ecdsa_sig);
    if (ret != UCI_SUCCESS) {
        return ret;
    }
    
    ret = pqc_dilithium2_verify(&dilithium_keypair, message, message_len, &dilithium_sig);
    if (ret != UCI_SUCCESS) {
        return ret;
    }
    
    return UCI_SUCCESS;
}
