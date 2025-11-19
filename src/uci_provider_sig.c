#include "uci_provider.h"

#ifdef HAVE_OPENSSL

#include "unified_crypto_interface.h"
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/err.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    UCI_PROVIDER_CTX *provctx;
    uci_algorithm_id_t algorithm;
    uci_keypair_t keypair;
} UCI_SIG_CTX;

static OSSL_FUNC_signature_newctx_fn uci_sig_newctx;
static OSSL_FUNC_signature_freectx_fn uci_sig_freectx;
static OSSL_FUNC_signature_sign_init_fn uci_sig_sign_init;
static OSSL_FUNC_signature_sign_fn uci_sig_sign;
static OSSL_FUNC_signature_verify_init_fn uci_sig_verify_init;
static OSSL_FUNC_signature_verify_fn uci_sig_verify;

static void *uci_sig_newctx(void *provctx, const char *propq) {
    UCI_SIG_CTX *ctx = malloc(sizeof(UCI_SIG_CTX));
    if (ctx != NULL) {
        memset(ctx, 0, sizeof(UCI_SIG_CTX));
        ctx->provctx = (UCI_PROVIDER_CTX *)provctx;
    }
    return ctx;
}

static void uci_sig_freectx(void *ctx) {
    UCI_SIG_CTX *sigctx = (UCI_SIG_CTX *)ctx;
    if (sigctx != NULL) {
        uci_keypair_free(&sigctx->keypair);
        free(sigctx);
    }
}

static int uci_sig_sign_init(void *ctx, void *provkey, const OSSL_PARAM params[]) {
    UCI_SIG_CTX *sigctx = (UCI_SIG_CTX *)ctx;
    if (sigctx == NULL || provkey == NULL)
        return 0;
    
    return 1;
}

static int uci_sig_sign(void *ctx,
                        unsigned char *sig, size_t *siglen, size_t sigsize,
                        const unsigned char *tbs, size_t tbslen) {
    UCI_SIG_CTX *sigctx = (UCI_SIG_CTX *)ctx;
    uci_signature_t signature;
    int ret;
    
    if (sigctx == NULL)
        return 0;
    
    if (sig == NULL) {
        uci_algorithm_info_t info;
        if (uci_get_algorithm_info(sigctx->algorithm, &info) != UCI_SUCCESS)
            return 0;
        *siglen = info.signature_len;
        return 1;
    }
    
    ret = uci_sign(&sigctx->keypair, tbs, tbslen, &signature);
    if (ret != UCI_SUCCESS)
        return 0;
    
    if (signature.data_len > sigsize) {
        uci_signature_free(&signature);
        return 0;
    }
    
    memcpy(sig, signature.data, signature.data_len);
    *siglen = signature.data_len;
    
    uci_signature_free(&signature);
    return 1;
}

static int uci_sig_verify_init(void *ctx, void *provkey, const OSSL_PARAM params[]) {
    UCI_SIG_CTX *sigctx = (UCI_SIG_CTX *)ctx;
    if (sigctx == NULL || provkey == NULL)
        return 0;
    
    return 1;
}

static int uci_sig_verify(void *ctx,
                          const unsigned char *sig, size_t siglen,
                          const unsigned char *tbs, size_t tbslen) {
    UCI_SIG_CTX *sigctx = (UCI_SIG_CTX *)ctx;
    uci_signature_t signature;
    int ret;
    
    if (sigctx == NULL)
        return 0;
    
    signature.algorithm = sigctx->algorithm;
    signature.data = (uint8_t *)sig;
    signature.data_len = siglen;
    
    ret = uci_verify(&sigctx->keypair, tbs, tbslen, &signature);
    
    return (ret == UCI_SUCCESS) ? 1 : 0;
}

#define MAKE_SIG_FUNCTIONS(name, ucialg) \
static void *name##_newctx(void *provctx, const char *propq) { \
    UCI_SIG_CTX *ctx = uci_sig_newctx(provctx, propq); \
    if (ctx != NULL) ctx->algorithm = ucialg; \
    return ctx; \
} \
static const OSSL_DISPATCH name##_functions[] = { \
    { OSSL_FUNC_SIGNATURE_NEWCTX, (void (*)(void))name##_newctx }, \
    { OSSL_FUNC_SIGNATURE_FREECTX, (void (*)(void))uci_sig_freectx }, \
    { OSSL_FUNC_SIGNATURE_SIGN_INIT, (void (*)(void))uci_sig_sign_init }, \
    { OSSL_FUNC_SIGNATURE_SIGN, (void (*)(void))uci_sig_sign }, \
    { OSSL_FUNC_SIGNATURE_VERIFY_INIT, (void (*)(void))uci_sig_verify_init }, \
    { OSSL_FUNC_SIGNATURE_VERIFY, (void (*)(void))uci_sig_verify }, \
    { 0, NULL } \
}

MAKE_SIG_FUNCTIONS(dilithium2, UCI_ALG_DILITHIUM2);
MAKE_SIG_FUNCTIONS(dilithium3, UCI_ALG_DILITHIUM3);
MAKE_SIG_FUNCTIONS(dilithium5, UCI_ALG_DILITHIUM5);
MAKE_SIG_FUNCTIONS(falcon512, UCI_ALG_FALCON512);

const OSSL_ALGORITHM *uci_provider_query_signature(void *provctx, int *no_cache) {
    static const OSSL_ALGORITHM signature_algs[] = {
        { "dilithium2", "provider=uci", dilithium2_functions, "Dilithium2 signature algorithm" },
        { "dilithium3", "provider=uci", dilithium3_functions, "Dilithium3 signature algorithm" },
        { "dilithium5", "provider=uci", dilithium5_functions, "Dilithium5 signature algorithm" },
        { "falcon512", "provider=uci", falcon512_functions, "Falcon-512 signature algorithm" },
        { NULL, NULL, NULL, NULL }
    };
    
    *no_cache = 0;
    return signature_algs;
}

#else

const OSSL_ALGORITHM *uci_provider_query_signature(void *provctx, int *no_cache) {
    return NULL;
}

#endif /* HAVE_OPENSSL */
