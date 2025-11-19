#ifndef UCI_PROVIDER_H
#define UCI_PROVIDER_H

#ifdef HAVE_OPENSSL

#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/params.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UCI_PROVIDER_NAME "uci"
#define UCI_PROVIDER_VERSION "1.0.0"

typedef struct uci_provider_ctx_st {
    const OSSL_CORE_HANDLE *handle;
    OSSL_LIB_CTX *libctx;
} UCI_PROVIDER_CTX;

int OSSL_provider_init(const OSSL_CORE_HANDLE *handle,
                       const OSSL_DISPATCH *in,
                       const OSSL_DISPATCH **out,
                       void **provctx);

const OSSL_ALGORITHM *uci_provider_query_signature(void *provctx, int *no_cache);
const OSSL_ALGORITHM *uci_provider_query_kem(void *provctx, int *no_cache);
const OSSL_ALGORITHM *uci_provider_query_keymgmt(void *provctx, int *no_cache);

#ifdef __cplusplus
}
#endif

#endif /* HAVE_OPENSSL */

#endif /* UCI_PROVIDER_H */
