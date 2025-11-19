#include "uci_provider.h"

#ifdef HAVE_OPENSSL

#include "unified_crypto_interface.h"
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    UCI_PROVIDER_CTX *provctx;
    uci_algorithm_id_t algorithm;
    uci_keypair_t keypair;
} UCI_KEM_CTX;

static OSSL_FUNC_kem_newctx_fn uci_kem_newctx;
static OSSL_FUNC_kem_freectx_fn uci_kem_freectx;
static OSSL_FUNC_kem_encapsulate_init_fn uci_kem_encapsulate_init;
static OSSL_FUNC_kem_encapsulate_fn uci_kem_encapsulate;
static OSSL_FUNC_kem_decapsulate_init_fn uci_kem_decapsulate_init;
static OSSL_FUNC_kem_decapsulate_fn uci_kem_decapsulate;

static void *uci_kem_newctx(void *provctx) {
    UCI_KEM_CTX *ctx = malloc(sizeof(UCI_KEM_CTX));
    if (ctx != NULL) {
        memset(ctx, 0, sizeof(UCI_KEM_CTX));
        ctx->provctx = (UCI_PROVIDER_CTX *)provctx;
    }
    return ctx;
}

static void uci_kem_freectx(void *ctx) {
    UCI_KEM_CTX *kemctx = (UCI_KEM_CTX *)ctx;
    if (kemctx != NULL) {
        uci_keypair_free(&kemctx->keypair);
        free(kemctx);
    }
}

static int uci_kem_encapsulate_init(void *ctx, void *provkey, const OSSL_PARAM params[]) {
    UCI_KEM_CTX *kemctx = (UCI_KEM_CTX *)ctx;
    if (kemctx == NULL || provkey == NULL)
        return 0;
    
    return 1;
}

static int uci_kem_encapsulate(void *ctx,
                                unsigned char *out, size_t *outlen,
                                unsigned char *secret, size_t *secretlen) {
    UCI_KEM_CTX *kemctx = (UCI_KEM_CTX *)ctx;
    uci_kem_encaps_result_t result;
    int ret;
    
    if (kemctx == NULL)
        return 0;
    
    if (out == NULL || secret == NULL) {
        uci_algorithm_info_t info;
        if (uci_get_algorithm_info(kemctx->algorithm, &info) != UCI_SUCCESS)
            return 0;
        
        if (outlen != NULL)
            *outlen = 1088;
        if (secretlen != NULL)
            *secretlen = 32;
        return 1;
    }
    
    ret = uci_kem_encaps(&kemctx->keypair, &result);
    if (ret != UCI_SUCCESS)
        return 0;
    
    if (*outlen < result.ciphertext_len || *secretlen < result.shared_secret_len) {
        uci_kem_encaps_result_free(&result);
        return 0;
    }
    
    memcpy(out, result.ciphertext, result.ciphertext_len);
    *outlen = result.ciphertext_len;
    
    memcpy(secret, result.shared_secret, result.shared_secret_len);
    *secretlen = result.shared_secret_len;
    
    uci_kem_encaps_result_free(&result);
    return 1;
}

static int uci_kem_decapsulate_init(void *ctx, void *provkey, const OSSL_PARAM params[]) {
    UCI_KEM_CTX *kemctx = (UCI_KEM_CTX *)ctx;
    if (kemctx == NULL || provkey == NULL)
        return 0;
    
    return 1;
}

static int uci_kem_decapsulate(void *ctx,
                                unsigned char *out, size_t *outlen,
                                const unsigned char *in, size_t inlen) {
    UCI_KEM_CTX *kemctx = (UCI_KEM_CTX *)ctx;
    int ret;
    
    if (kemctx == NULL)
        return 0;
    
    if (out == NULL) {
        *outlen = 32;
        return 1;
    }
    
    ret = uci_kem_decaps(&kemctx->keypair, in, inlen, out, outlen);
    
    return (ret == UCI_SUCCESS) ? 1 : 0;
}

#define MAKE_KEM_FUNCTIONS(name, ucialg) \
static void *name##_newctx(void *provctx) { \
    UCI_KEM_CTX *ctx = uci_kem_newctx(provctx); \
    if (ctx != NULL) ctx->algorithm = ucialg; \
    return ctx; \
} \
static const OSSL_DISPATCH name##_functions[] = { \
    { OSSL_FUNC_KEM_NEWCTX, (void (*)(void))name##_newctx }, \
    { OSSL_FUNC_KEM_FREECTX, (void (*)(void))uci_kem_freectx }, \
    { OSSL_FUNC_KEM_ENCAPSULATE_INIT, (void (*)(void))uci_kem_encapsulate_init }, \
    { OSSL_FUNC_KEM_ENCAPSULATE, (void (*)(void))uci_kem_encapsulate }, \
    { OSSL_FUNC_KEM_DECAPSULATE_INIT, (void (*)(void))uci_kem_decapsulate_init }, \
    { OSSL_FUNC_KEM_DECAPSULATE, (void (*)(void))uci_kem_decapsulate }, \
    { 0, NULL } \
}

MAKE_KEM_FUNCTIONS(kyber512, UCI_ALG_KYBER512);
MAKE_KEM_FUNCTIONS(kyber768, UCI_ALG_KYBER768);
MAKE_KEM_FUNCTIONS(kyber1024, UCI_ALG_KYBER1024);

const OSSL_ALGORITHM *uci_provider_query_kem(void *provctx, int *no_cache) {
    static const OSSL_ALGORITHM kem_algs[] = {
        { "kyber512", "provider=uci", kyber512_functions, "Kyber512 KEM" },
        { "kyber768", "provider=uci", kyber768_functions, "Kyber768 KEM" },
        { "kyber1024", "provider=uci", kyber1024_functions, "Kyber1024 KEM" },
        { NULL, NULL, NULL, NULL }
    };
    
    *no_cache = 0;
    return kem_algs;
}

#else

const OSSL_ALGORITHM *uci_provider_query_kem(void *provctx, int *no_cache) {
    return NULL;
}

#endif /* HAVE_OPENSSL */
