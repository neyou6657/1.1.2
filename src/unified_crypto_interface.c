#include "unified_crypto_interface.h"
#include "algorithm_registry.h"
#include "openssl_adapter.h"
#include "classic_crypto_adapter.h"
#include "pqc_adapter.h"
#include "hybrid_crypto.h"
#include <string.h>
#include <stdlib.h>

int uci_init(void) {
    int ret;
    
    ret = registry_init();
    if (ret != UCI_SUCCESS) {
        return ret;
    }
    
    ret = openssl_adapter_init();
    if (ret != UCI_SUCCESS) {
        registry_cleanup();
        return ret;
    }
    
    ret = classic_adapter_init();
    if (ret != UCI_SUCCESS) {
        openssl_adapter_cleanup();
        registry_cleanup();
        return ret;
    }
    
    ret = pqc_adapter_init();
    if (ret != UCI_SUCCESS) {
        classic_adapter_cleanup();
        openssl_adapter_cleanup();
        registry_cleanup();
        return ret;
    }
    
    ret = hybrid_adapter_init();
    if (ret != UCI_SUCCESS) {
        pqc_adapter_cleanup();
        classic_adapter_cleanup();
        openssl_adapter_cleanup();
        registry_cleanup();
        return ret;
    }
    
    return UCI_SUCCESS;
}

int uci_cleanup(void) {
    hybrid_adapter_cleanup();
    pqc_adapter_cleanup();
    classic_adapter_cleanup();
    openssl_adapter_cleanup();
    registry_cleanup();
    return UCI_SUCCESS;
}

int uci_get_algorithm_info(uci_algorithm_id_t algorithm, uci_algorithm_info_t *info) {
    if (!info) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    memcpy(info, &impl->info, sizeof(uci_algorithm_info_t));
    return UCI_SUCCESS;
}

int uci_list_algorithms(uci_algorithm_type_t type, uci_algorithm_id_t *algorithms, size_t *count) {
    if (!count) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    return registry_list_algorithms(type, algorithms, count);
}

int uci_keygen(uci_algorithm_id_t algorithm, uci_keypair_t *keypair) {
    if (!keypair) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->keygen) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    keypair->algorithm = algorithm;
    return impl->keygen(keypair);
}

int uci_keypair_free(uci_keypair_t *keypair) {
    if (!keypair) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    if (keypair->public_key) {
        free(keypair->public_key);
        keypair->public_key = NULL;
    }
    
    if (keypair->private_key) {
        free(keypair->private_key);
        keypair->private_key = NULL;
    }
    
    keypair->public_key_len = 0;
    keypair->private_key_len = 0;
    
    return UCI_SUCCESS;
}

int uci_sign(const uci_keypair_t *keypair, const uint8_t *message, size_t message_len,
             uci_signature_t *signature) {
    if (!keypair || !message || !signature) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(keypair->algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->sign) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    signature->algorithm = keypair->algorithm;
    return impl->sign(keypair, message, message_len, signature);
}

int uci_verify(const uci_keypair_t *keypair, const uint8_t *message, size_t message_len,
               const uci_signature_t *signature) {
    if (!keypair || !message || !signature) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(keypair->algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->verify) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    return impl->verify(keypair, message, message_len, signature);
}

int uci_signature_free(uci_signature_t *signature) {
    if (!signature) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    if (signature->data) {
        free(signature->data);
        signature->data = NULL;
    }
    
    signature->data_len = 0;
    
    return UCI_SUCCESS;
}

int uci_encrypt(const uci_keypair_t *keypair, const uint8_t *plaintext, size_t plaintext_len,
                uci_ciphertext_t *ciphertext) {
    if (!keypair || !plaintext || !ciphertext) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(keypair->algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->encrypt) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    ciphertext->algorithm = keypair->algorithm;
    return impl->encrypt(keypair, plaintext, plaintext_len, ciphertext);
}

int uci_decrypt(const uci_keypair_t *keypair, const uci_ciphertext_t *ciphertext,
                uint8_t *plaintext, size_t *plaintext_len) {
    if (!keypair || !ciphertext || !plaintext || !plaintext_len) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(keypair->algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->decrypt) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    return impl->decrypt(keypair, ciphertext, plaintext, plaintext_len);
}

int uci_ciphertext_free(uci_ciphertext_t *ciphertext) {
    if (!ciphertext) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    if (ciphertext->ciphertext) {
        free(ciphertext->ciphertext);
        ciphertext->ciphertext = NULL;
    }
    
    ciphertext->ciphertext_len = 0;
    
    return UCI_SUCCESS;
}

int uci_kem_keygen(uci_algorithm_id_t algorithm, uci_keypair_t *keypair) {
    if (!keypair) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->kem_keygen) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    keypair->algorithm = algorithm;
    return impl->kem_keygen(keypair);
}

int uci_kem_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result) {
    if (!keypair || !result) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(keypair->algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->kem_encaps) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    return impl->kem_encaps(keypair, result);
}

int uci_kem_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext, size_t ciphertext_len,
                   uint8_t *shared_secret, size_t *shared_secret_len) {
    if (!keypair || !ciphertext || !shared_secret || !shared_secret_len) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(keypair->algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->kem_decaps) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    return impl->kem_decaps(keypair, ciphertext, ciphertext_len, shared_secret, shared_secret_len);
}

int uci_kem_encaps_result_free(uci_kem_encaps_result_t *result) {
    if (!result) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    if (result->shared_secret) {
        free(result->shared_secret);
        result->shared_secret = NULL;
    }
    
    if (result->ciphertext) {
        free(result->ciphertext);
        result->ciphertext = NULL;
    }
    
    result->shared_secret_len = 0;
    result->ciphertext_len = 0;
    
    return UCI_SUCCESS;
}

int uci_hybrid_sign(uci_algorithm_id_t algorithm, 
                    const uci_keypair_t *classic_keypair,
                    const uci_keypair_t *pq_keypair,
                    const uint8_t *message, size_t message_len,
                    uci_signature_t *signature) {
    if (!classic_keypair || !pq_keypair || !message || !signature) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->sign) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    return UCI_ERROR_NOT_SUPPORTED;
}

int uci_hybrid_verify(uci_algorithm_id_t algorithm,
                      const uci_keypair_t *classic_keypair,
                      const uci_keypair_t *pq_keypair,
                      const uint8_t *message, size_t message_len,
                      const uci_signature_t *signature) {
    if (!classic_keypair || !pq_keypair || !message || !signature) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    const uci_algorithm_impl_t *impl = registry_get_algorithm(algorithm);
    if (!impl) {
        return UCI_ERROR_ALGORITHM_NOT_FOUND;
    }
    
    if (!impl->verify) {
        return UCI_ERROR_NOT_SUPPORTED;
    }
    
    return UCI_ERROR_NOT_SUPPORTED;
}

const char *uci_get_error_string(int error_code) {
    switch (error_code) {
        case UCI_SUCCESS:
            return "Success";
        case UCI_ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case UCI_ERROR_NOT_SUPPORTED:
            return "Operation not supported";
        case UCI_ERROR_BUFFER_TOO_SMALL:
            return "Buffer too small";
        case UCI_ERROR_ALGORITHM_NOT_FOUND:
            return "Algorithm not found";
        case UCI_ERROR_INTERNAL:
            return "Internal error";
        case UCI_ERROR_SIGNATURE_INVALID:
            return "Signature verification failed";
        default:
            return "Unknown error";
    }
}
