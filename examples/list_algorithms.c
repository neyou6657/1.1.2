#include "unified_crypto_interface.h"
#include <stdio.h>
#include <stdlib.h>

void print_algorithm_details(uci_algorithm_id_t alg_id) {
    uci_algorithm_info_t info;
    if (uci_get_algorithm_info(alg_id, &info) != UCI_SUCCESS) {
        return;
    }
    
    printf("  Algorithm: %s\n", info.name);
    printf("    ID: %d\n", info.id);
    printf("    Security Level: %d bits\n", info.security_level);
    printf("    Public Key Size: %zu bytes\n", info.public_key_len);
    printf("    Private Key Size: %zu bytes\n", info.private_key_len);
    if (info.signature_len > 0) {
        printf("    Signature Size: %zu bytes\n", info.signature_len);
    }
    printf("\n");
}

int main() {
    printf("=== Unified Crypto Interface - Algorithm Details ===\n\n");
    
    if (uci_init() != UCI_SUCCESS) {
        fprintf(stderr, "Failed to initialize UCI\n");
        return 1;
    }
    
    size_t count;
    
    printf("CLASSIC ALGORITHMS:\n");
    printf("===================\n");
    count = 0;
    uci_list_algorithms(UCI_ALG_TYPE_CLASSIC, NULL, &count);
    if (count > 0) {
        uci_algorithm_id_t *algs = malloc(count * sizeof(uci_algorithm_id_t));
        if (algs) {
            uci_list_algorithms(UCI_ALG_TYPE_CLASSIC, algs, &count);
            for (size_t i = 0; i < count; i++) {
                print_algorithm_details(algs[i]);
            }
            free(algs);
        }
    } else {
        printf("  No classic algorithms available\n\n");
    }
    
    printf("POST-QUANTUM ALGORITHMS:\n");
    printf("========================\n");
    count = 0;
    uci_list_algorithms(UCI_ALG_TYPE_POST_QUANTUM, NULL, &count);
    if (count > 0) {
        uci_algorithm_id_t *algs = malloc(count * sizeof(uci_algorithm_id_t));
        if (algs) {
            uci_list_algorithms(UCI_ALG_TYPE_POST_QUANTUM, algs, &count);
            for (size_t i = 0; i < count; i++) {
                print_algorithm_details(algs[i]);
            }
            free(algs);
        }
    } else {
        printf("  No post-quantum algorithms available\n\n");
    }
    
    printf("HYBRID ALGORITHMS:\n");
    printf("==================\n");
    count = 0;
    uci_list_algorithms(UCI_ALG_TYPE_HYBRID, NULL, &count);
    if (count > 0) {
        uci_algorithm_id_t *algs = malloc(count * sizeof(uci_algorithm_id_t));
        if (algs) {
            uci_list_algorithms(UCI_ALG_TYPE_HYBRID, algs, &count);
            for (size_t i = 0; i < count; i++) {
                print_algorithm_details(algs[i]);
            }
            free(algs);
        }
    } else {
        printf("  No hybrid algorithms available\n\n");
    }
    
    uci_cleanup();
    
    return 0;
}
