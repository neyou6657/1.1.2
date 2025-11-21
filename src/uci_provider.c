#include "uci_provider.h"

#ifdef HAVE_OPENSSL

#include "unified_crypto_interface.h"
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <string.h>
#include <stdlib.h>

static OSSL_FUNC_provider_teardown_fn uci_provider_teardown;
static OSSL_FUNC_provider_gettable_params_fn uci_provider_gettable_params;
static OSSL_FUNC_provider_get_params_fn uci_provider_get_params;
static OSSL_FUNC_provider_query_operation_fn uci_provider_query;
static OSSL_FUNC_provider_get_capabilities_fn uci_provider_get_capabilities;

static void uci_provider_teardown(void *provctx) {
    UCI_PROVIDER_CTX *ctx = (UCI_PROVIDER_CTX *)provctx;
    if (ctx) {
        uci_cleanup();
        OSSL_LIB_CTX_free(ctx->libctx);
        free(ctx);
    }
}

static const OSSL_PARAM *uci_provider_gettable_params(void *provctx) {
    static const OSSL_PARAM param_types[] = {
        OSSL_PARAM_DEFN(OSSL_PROV_PARAM_NAME, OSSL_PARAM_UTF8_PTR, NULL, 0),
        OSSL_PARAM_DEFN(OSSL_PROV_PARAM_VERSION, OSSL_PARAM_UTF8_PTR, NULL, 0),
        OSSL_PARAM_DEFN(OSSL_PROV_PARAM_BUILDINFO, OSSL_PARAM_UTF8_PTR, NULL, 0),
        OSSL_PARAM_DEFN(OSSL_PROV_PARAM_STATUS, OSSL_PARAM_INTEGER, NULL, 0),
        OSSL_PARAM_END
    };
    return param_types;
}

static int uci_provider_get_params(void *provctx, OSSL_PARAM params[]) {
    OSSL_PARAM *p;
    
    p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_NAME);
    if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, UCI_PROVIDER_NAME))
        return 0;
    
    p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_VERSION);
    if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, UCI_PROVIDER_VERSION))
        return 0;
    
    p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_BUILDINFO);
    if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, "UCI Provider - Unified Crypto Interface"))
        return 0;
    
    p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_STATUS);
    if (p != NULL && !OSSL_PARAM_set_int(p, 1))
        return 0;
    
    return 1;
}

static const OSSL_ALGORITHM *uci_provider_query(void *provctx,
                                                int operation_id,
                                                int *no_cache) {
    *no_cache = 0;
    
    switch (operation_id) {
        case OSSL_OP_SIGNATURE:
            return uci_provider_query_signature(provctx, no_cache);
        case OSSL_OP_KEM:
            return uci_provider_query_kem(provctx, no_cache);
        case OSSL_OP_KEYMGMT:
            return uci_provider_query_keymgmt(provctx, no_cache);
        default:
            return NULL;
    }
}

static int uci_provider_get_capabilities(void *provctx,
                                         const char *capability,
                                         OSSL_CALLBACK *cb,
                                         void *arg) {
    if (strcmp(capability, "TLS-GROUP") == 0) {
        static const char *tls_group_list[] = {
            "kyber512",
            "kyber768",
            "kyber1024",
            "X25519Kyber768",
            NULL
        };
        
        for (const char **g = tls_group_list; *g != NULL; g++) {
            OSSL_PARAM params[5];
            int idx = 0;
            
            unsigned int group_id = 0xFE00;
            unsigned int security_bits = 128;
            
            params[idx++] = OSSL_PARAM_construct_utf8_string(
                OSSL_CAPABILITY_TLS_GROUP_NAME, (char *)*g, 0);
            params[idx++] = OSSL_PARAM_construct_utf8_string(
                OSSL_CAPABILITY_TLS_GROUP_NAME_INTERNAL, (char *)*g, 0);
            params[idx++] = OSSL_PARAM_construct_uint(
                OSSL_CAPABILITY_TLS_GROUP_ID, &group_id);
            params[idx++] = OSSL_PARAM_construct_uint(
                OSSL_CAPABILITY_TLS_GROUP_SECURITY_BITS, &security_bits);
            params[idx] = OSSL_PARAM_construct_end();
            
            if (!cb(params, arg))
                return 0;
        }
    }
    
    return 1;
}

static const OSSL_DISPATCH uci_provider_functions[] = {
    { OSSL_FUNC_PROVIDER_TEARDOWN, (void (*)(void))uci_provider_teardown },
    { OSSL_FUNC_PROVIDER_GETTABLE_PARAMS, (void (*)(void))uci_provider_gettable_params },
    { OSSL_FUNC_PROVIDER_GET_PARAMS, (void (*)(void))uci_provider_get_params },
    { OSSL_FUNC_PROVIDER_QUERY_OPERATION, (void (*)(void))uci_provider_query },
    { OSSL_FUNC_PROVIDER_GET_CAPABILITIES, (void (*)(void))uci_provider_get_capabilities },
    { 0, NULL }
};

int OSSL_provider_init(const OSSL_CORE_HANDLE *handle,
                       const OSSL_DISPATCH *in,
                       const OSSL_DISPATCH **out,
                       void **provctx) {
    UCI_PROVIDER_CTX *ctx;
    
    ctx = malloc(sizeof(UCI_PROVIDER_CTX));
    if (ctx == NULL)
        return 0;
    
    ctx->handle = handle;
    ctx->libctx = OSSL_LIB_CTX_new();
    if (ctx->libctx == NULL) {
        free(ctx);
        return 0;
    }
    
    if (uci_init() != UCI_SUCCESS) {
        OSSL_LIB_CTX_free(ctx->libctx);
        free(ctx);
        return 0;
    }
    
    *provctx = ctx;
    *out = uci_provider_functions;
    
    return 1;
}

#else

int OSSL_provider_init(const OSSL_CORE_HANDLE *handle,
                       const OSSL_DISPATCH *in,
                       const OSSL_DISPATCH **out,
                       void **provctx) {
    return 0;
}

#endif /* HAVE_OPENSSL */
