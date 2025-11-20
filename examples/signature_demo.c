#include "unified_crypto_interface.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_signature_algorithm(uci_algorithm_id_t alg_id, const char *alg_name) {
    printf("\nTesting %s...\n", alg_name);
    
    uci_keypair_t keypair;
    memset(&keypair, 0, sizeof(keypair));
    
    printf("  Generating keypair...\n");
    int ret = uci_keygen(alg_id, &keypair);
    if (ret != UCI_SUCCESS) {
        printf("  ERROR: Key generation failed: %s\n", uci_get_error_string(ret));
        return;
    }
    
    printf("  Public key size: %zu bytes\n", keypair.public_key_len);
    printf("  Private key size: %zu bytes\n", keypair.private_key_len);
    
    const char *message = "Hello, Post-Quantum World!";
    size_t message_len = strlen(message);
    
    uci_signature_t signature;
    memset(&signature, 0, sizeof(signature));
    
    printf("  Signing message...\n");
    ret = uci_sign(&keypair, (const uint8_t *)message, message_len, &signature);
    if (ret != UCI_SUCCESS) {
        printf("  ERROR: Signing failed: %s\n", uci_get_error_string(ret));
        uci_keypair_free(&keypair);
        return;
    }
    
    printf("  Signature size: %zu bytes\n", signature.data_len);
    
    printf("  Verifying signature...\n");
    ret = uci_verify(&keypair, (const uint8_t *)message, message_len, &signature);
    if (ret == UCI_SUCCESS) {
        printf("  SUCCESS: Signature verification passed!\n");
    } else {
        printf("  ERROR: Signature verification failed: %s\n", uci_get_error_string(ret));
    }
    
    printf("  Testing with tampered message...\n");
    const char *tampered_message = "Hello, Quantum World!";
    ret = uci_verify(&keypair, (const uint8_t *)tampered_message, strlen(tampered_message), &signature);
    if (ret != UCI_SUCCESS) {
        printf("  SUCCESS: Tampered message correctly rejected!\n");
    } else {
        printf("  ERROR: Tampered message was incorrectly accepted!\n");
    }
    
    uci_signature_free(&signature);
    uci_keypair_free(&keypair);
}

int main() {
    printf("=== Digital Signature Demo ===\n");
    
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
            
            for (size_t i = 0; i < count && i < 3; i++) {
                uci_algorithm_info_t info;
                if (uci_get_algorithm_info(algorithms[i], &info) == UCI_SUCCESS) {
                    if (info.signature_len > 0) {
                        test_signature_algorithm(algorithms[i], info.name);
                    }
                }
            }
            
            free(algorithms);
        }
    } else {
        printf("\nNo post-quantum signature algorithms available.\n");
        printf("Please build with LibOQS support.\n");
    }
    
    count = 0;
    uci_list_algorithms(UCI_ALG_TYPE_CLASSIC, NULL, &count);
    
    if (count > 0) {
        uci_algorithm_id_t *algorithms = malloc(count * sizeof(uci_algorithm_id_t));
        if (algorithms) {
            uci_list_algorithms(UCI_ALG_TYPE_CLASSIC, algorithms, &count);
            
            for (size_t i = 0; i < count; i++) {
                uci_algorithm_info_t info;
                if (uci_get_algorithm_info(algorithms[i], &info) == UCI_SUCCESS) {
                    if (info.signature_len > 0) {
                        test_signature_algorithm(algorithms[i], info.name);
                    }
                }
            }
            
            free(algorithms);
        }
    }
    
    uci_cleanup();
    
    printf("\n=== Demo completed ===\n");
    
    return 0;
}
