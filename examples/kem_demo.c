#include "unified_crypto_interface.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_kem_algorithm(uci_algorithm_id_t alg_id, const char *alg_name) {
    printf("\nTesting %s KEM...\n", alg_name);
    
    uci_keypair_t keypair;
    memset(&keypair, 0, sizeof(keypair));
    
    printf("  Generating KEM keypair...\n");
    int ret = uci_kem_keygen(alg_id, &keypair);
    if (ret != UCI_SUCCESS) {
        printf("  ERROR: KEM key generation failed: %s\n", uci_get_error_string(ret));
        return;
    }
    
    printf("  Public key size: %zu bytes\n", keypair.public_key_len);
    printf("  Private key size: %zu bytes\n", keypair.private_key_len);
    
    uci_kem_encaps_result_t encaps_result;
    memset(&encaps_result, 0, sizeof(encaps_result));
    
    printf("  Encapsulating...\n");
    ret = uci_kem_encaps(&keypair, &encaps_result);
    if (ret != UCI_SUCCESS) {
        printf("  ERROR: Encapsulation failed: %s\n", uci_get_error_string(ret));
        uci_keypair_free(&keypair);
        return;
    }
    
    printf("  Shared secret size: %zu bytes\n", encaps_result.shared_secret_len);
    printf("  Ciphertext size: %zu bytes\n", encaps_result.ciphertext_len);
    
    uint8_t decaps_secret[1024];
    size_t decaps_secret_len = sizeof(decaps_secret);
    
    printf("  Decapsulating...\n");
    ret = uci_kem_decaps(&keypair, encaps_result.ciphertext, encaps_result.ciphertext_len,
                         decaps_secret, &decaps_secret_len);
    if (ret != UCI_SUCCESS) {
        printf("  ERROR: Decapsulation failed: %s\n", uci_get_error_string(ret));
        uci_kem_encaps_result_free(&encaps_result);
        uci_keypair_free(&keypair);
        return;
    }
    
    printf("  Decapsulated secret size: %zu bytes\n", decaps_secret_len);
    
    if (decaps_secret_len == encaps_result.shared_secret_len &&
        memcmp(decaps_secret, encaps_result.shared_secret, decaps_secret_len) == 0) {
        printf("  SUCCESS: Shared secrets match!\n");
    } else {
        printf("  ERROR: Shared secrets do not match!\n");
    }
    
    uci_kem_encaps_result_free(&encaps_result);
    uci_keypair_free(&keypair);
}

int main() {
    printf("=== Key Encapsulation Mechanism (KEM) Demo ===\n");
    
    if (uci_init() != UCI_SUCCESS) {
        fprintf(stderr, "Failed to initialize UCI\n");
        return 1;
    }
    
    size_t count = 0;
    uci_list_algorithms(UCI_ALG_TYPE_POST_QUANTUM, NULL, &count);
    
    if (count > 0) {
        uci_algorithm_id_t *algorithms = malloc(count * sizeof(uci_algorithm_id_t));
        if (algorithms) {
            uci_list_algorithms(UCI_ALG_TYPE_POST_QUANTUM, algorithms, &count);
            
            for (size_t i = 0; i < count; i++) {
                uci_algorithm_info_t info;
                if (uci_get_algorithm_info(algorithms[i], &info) == UCI_SUCCESS) {
                    if (info.signature_len == 0) {
                        test_kem_algorithm(algorithms[i], info.name);
                    }
                }
            }
            
            free(algorithms);
        }
    } else {
        printf("\nNo post-quantum KEM algorithms available.\n");
        printf("Please build with LibOQS support.\n");
    }
    
    uci_cleanup();
    
    printf("\n=== Demo completed ===\n");
    
    return 0;
}
