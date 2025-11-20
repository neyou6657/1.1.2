#include "openssl/oqs.h"

#include <openssl/crypto.h>
#include <openssl/provider.h>

static OSSL_PROVIDER *g_uci_provider = NULL;

int oqs_provider_load(void) {
    if (g_uci_provider != NULL) {
        return 1;
    }

    g_uci_provider = OSSL_PROVIDER_load(NULL, "uci");
    if (g_uci_provider == NULL) {
        return 0;
    }

    return 1;
}

void oqs_provider_unload(void) {
    if (g_uci_provider != NULL) {
        OSSL_PROVIDER_unload(g_uci_provider);
        g_uci_provider = NULL;
    }
}

int oqs_kem_keygen(const char *algorithm, EVP_PKEY **keypair) {
    EVP_PKEY_CTX *ctx = NULL;
    int ret = 0;

    if (algorithm == NULL || keypair == NULL) {
        return 0;
    }

    *keypair = NULL;

    ctx = EVP_PKEY_CTX_new_from_name(NULL, algorithm, NULL);
    if (ctx == NULL) {
        goto cleanup;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        goto cleanup;
    }

    if (EVP_PKEY_keygen(ctx, keypair) <= 0) {
        goto cleanup;
    }

    ret = 1;

cleanup:
    EVP_PKEY_CTX_free(ctx);
    return ret;
}

static int oqs_kem_prepare_output(EVP_PKEY_CTX *ctx,
                                  unsigned char **ciphertext, size_t *ciphertext_len,
                                  unsigned char **shared_secret, size_t *shared_secret_len) {
    if (ciphertext == NULL || ciphertext_len == NULL ||
        shared_secret == NULL || shared_secret_len == NULL) {
        return 0;
    }

    if (EVP_PKEY_encapsulate(ctx, NULL, ciphertext_len, NULL, shared_secret_len) <= 0) {
        return 0;
    }

    *ciphertext = OPENSSL_malloc(*ciphertext_len);
    *shared_secret = OPENSSL_malloc(*shared_secret_len);

    if (*ciphertext == NULL || *shared_secret == NULL) {
        OPENSSL_free(*ciphertext);
        OPENSSL_free(*shared_secret);
        *ciphertext = NULL;
        *shared_secret = NULL;
        return 0;
    }

    return 1;
}

int oqs_kem_encapsulate(EVP_PKEY *public_key,
                        unsigned char **ciphertext, size_t *ciphertext_len,
                        unsigned char **shared_secret, size_t *shared_secret_len) {
    EVP_PKEY_CTX *ctx = NULL;
    int ret = 0;

    if (public_key == NULL || ciphertext == NULL || ciphertext_len == NULL ||
        shared_secret == NULL || shared_secret_len == NULL) {
        return 0;
    }

    if (ciphertext != NULL) {
        *ciphertext = NULL;
    }
    if (shared_secret != NULL) {
        *shared_secret = NULL;
    }
    if (ciphertext_len != NULL) {
        *ciphertext_len = 0;
    }
    if (shared_secret_len != NULL) {
        *shared_secret_len = 0;
    }

    ctx = EVP_PKEY_CTX_new_from_pkey(NULL, public_key, NULL);
    if (ctx == NULL) {
        goto cleanup;
    }

    if (EVP_PKEY_encapsulate_init(ctx, NULL) <= 0) {
        goto cleanup;
    }

    if (!oqs_kem_prepare_output(ctx, ciphertext, ciphertext_len, shared_secret, shared_secret_len)) {
        goto cleanup;
    }

    if (EVP_PKEY_encapsulate(ctx, ciphertext, ciphertext_len, shared_secret, shared_secret_len) <= 0) {
        goto cleanup;
    }

    ret = 1;

cleanup:
    if (!ret) {
        if (ciphertext != NULL && *ciphertext != NULL) {
            OPENSSL_free(*ciphertext);
            *ciphertext = NULL;
        }
        if (shared_secret != NULL && *shared_secret != NULL) {
            OPENSSL_free(*shared_secret);
            *shared_secret = NULL;
        }
        if (ciphertext_len != NULL) {
            *ciphertext_len = 0;
        }
        if (shared_secret_len != NULL) {
            *shared_secret_len = 0;
        }
    }

    EVP_PKEY_CTX_free(ctx);
    return ret;
}

int oqs_kem_decapsulate(EVP_PKEY *keypair,
                        const unsigned char *ciphertext, size_t ciphertext_len,
                        unsigned char **shared_secret, size_t *shared_secret_len) {
    EVP_PKEY_CTX *ctx = NULL;
    int ret = 0;

    if (keypair == NULL || ciphertext == NULL || shared_secret == NULL || shared_secret_len == NULL) {
        return 0;
    }

    *shared_secret = NULL;
    *shared_secret_len = 0;

    ctx = EVP_PKEY_CTX_new_from_pkey(NULL, keypair, NULL);
    if (ctx == NULL) {
        goto cleanup;
    }

    if (EVP_PKEY_decapsulate_init(ctx, NULL) <= 0) {
        goto cleanup;
    }

    if (EVP_PKEY_decapsulate(ctx, NULL, shared_secret_len, ciphertext, ciphertext_len) <= 0) {
        goto cleanup;
    }

    *shared_secret = OPENSSL_malloc(*shared_secret_len);
    if (*shared_secret == NULL) {
        goto cleanup;
    }

    if (EVP_PKEY_decapsulate(ctx, shared_secret, shared_secret_len, ciphertext, ciphertext_len) <= 0) {
        goto cleanup;
    }

    ret = 1;

cleanup:
    if (!ret) {
        if (shared_secret != NULL && *shared_secret != NULL) {
            OPENSSL_free(*shared_secret);
            *shared_secret = NULL;
        }
        if (shared_secret_len != NULL) {
            *shared_secret_len = 0;
        }
    }

    EVP_PKEY_CTX_free(ctx);
    return ret;
}
