#include "algorithm_registry.h"
#include <stdlib.h>
#include <string.h>

#define MAX_ALGORITHMS 100

static uci_algorithm_impl_t *algorithm_table[MAX_ALGORITHMS];
static size_t algorithm_count = 0;

int registry_init(void) {
    memset(algorithm_table, 0, sizeof(algorithm_table));
    algorithm_count = 0;
    return UCI_SUCCESS;
}

int registry_cleanup(void) {
    for (size_t i = 0; i < algorithm_count; i++) {
        if (algorithm_table[i]) {
            free(algorithm_table[i]);
            algorithm_table[i] = NULL;
        }
    }
    algorithm_count = 0;
    return UCI_SUCCESS;
}

int registry_register_algorithm(const uci_algorithm_impl_t *impl) {
    if (!impl) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    if (algorithm_count >= MAX_ALGORITHMS) {
        return UCI_ERROR_INTERNAL;
    }
    
    for (size_t i = 0; i < algorithm_count; i++) {
        if (algorithm_table[i] && algorithm_table[i]->info.id == impl->info.id) {
            return UCI_ERROR_INTERNAL;
        }
    }
    
    uci_algorithm_impl_t *new_impl = (uci_algorithm_impl_t *)malloc(sizeof(uci_algorithm_impl_t));
    if (!new_impl) {
        return UCI_ERROR_INTERNAL;
    }
    
    memcpy(new_impl, impl, sizeof(uci_algorithm_impl_t));
    algorithm_table[algorithm_count++] = new_impl;
    
    return UCI_SUCCESS;
}

const uci_algorithm_impl_t *registry_get_algorithm(uci_algorithm_id_t algorithm) {
    for (size_t i = 0; i < algorithm_count; i++) {
        if (algorithm_table[i] && algorithm_table[i]->info.id == algorithm) {
            return algorithm_table[i];
        }
    }
    return NULL;
}

int registry_list_algorithms(uci_algorithm_type_t type, uci_algorithm_id_t *algorithms, size_t *count) {
    if (!count) {
        return UCI_ERROR_INVALID_PARAM;
    }
    
    size_t matched = 0;
    
    for (size_t i = 0; i < algorithm_count; i++) {
        if (!algorithm_table[i]) {
            continue;
        }
        
        if (type == algorithm_table[i]->info.type || type == -1) {
            if (algorithms && matched < *count) {
                algorithms[matched] = algorithm_table[i]->info.id;
            }
            matched++;
        }
    }
    
    if (algorithms && matched > *count) {
        *count = matched;
        return UCI_ERROR_BUFFER_TOO_SMALL;
    }
    
    *count = matched;
    return UCI_SUCCESS;
}
