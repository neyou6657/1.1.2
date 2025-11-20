#include "uci_provider.h"

#ifdef HAVE_OPENSSL

#include "unified_crypto_interface.h"
#include <openssl/core_names.h>
#include <openssl/params.h>

const OSSL_ALGORITHM *uci_provider_query_keymgmt(void *provctx, int *no_cache) {
    *no_cache = 0;
    return NULL;
}

#else

const OSSL_ALGORITHM *uci_provider_query_keymgmt(void *provctx, int *no_cache) {
    return NULL;
}

#endif /* HAVE_OPENSSL */
