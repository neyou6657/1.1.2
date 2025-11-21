#include <stdio.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/oqs.h>

int main(void) {
    EVP_PKEY *keypair = NULL;
    unsigned char *ciphertext = NULL;
    unsigned char *shared_secret_enc = NULL;
    unsigned char *shared_secret_dec = NULL;
    size_t ciphertext_len = 0;
    size_t shared_secret_enc_len = 0;
    size_t shared_secret_dec_len = 0;
    int ret = 1;

    if (!oqs_provider_load()) {
        fprintf(stderr, "Failed to load UCI provider. Ensure uci.so is installed and OPENSSL_MODULES is set correctly.\n");
        return 1;
    }

    if (!oqs_kem_keygen(OQS_KEM_KYBER768, &keypair)) {
        fprintf(stderr, "Key generation failed.\n");
        ret = 1;
        goto cleanup;
    }

    if (!oqs_kem_encapsulate(keypair, &ciphertext, &ciphertext_len,
                             &shared_secret_enc, &shared_secret_enc_len)) {
        fprintf(stderr, "KEM encapsulation failed.\n");
        ret = 1;
        goto cleanup;
    }

    if (!oqs_kem_decapsulate(keypair, ciphertext, ciphertext_len,
                             &shared_secret_dec, &shared_secret_dec_len)) {
        fprintf(stderr, "KEM decapsulation failed.\n");
        ret = 1;
        goto cleanup;
    }

    if (shared_secret_enc_len != shared_secret_dec_len ||
        CRYPTO_memcmp(shared_secret_enc, shared_secret_dec, shared_secret_enc_len) != 0) {
        fprintf(stderr, "Shared secrets do not match.\n");
        ret = 1;
        goto cleanup;
    }

    printf("Kyber768 KEM round trip succeeded. Ciphertext: %zu bytes, Shared secret: %zu bytes.\n",
           ciphertext_len, shared_secret_enc_len);
    ret = 0;

cleanup:
    OPENSSL_free(ciphertext);
    OPENSSL_free(shared_secret_enc);
    OPENSSL_free(shared_secret_dec);
    EVP_PKEY_free(keypair);
    oqs_provider_unload();
    return ret;
}
