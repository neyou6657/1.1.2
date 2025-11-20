#include "unified_crypto_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Unified Crypto Interface Demo ===\n\n");
    
    if (uci_init() != UCI_SUCCESS) {
        fprintf(stderr, "Failed to initialize UCI\n");
        return 1;
    }
    
    printf("UCI initialized successfully\n\n");
    
    size_t count = 0;
    uci_list_algorithms(-1, NULL, &count);
    
    printf("Total algorithms available: %zu\n\n", count);
    
    uci_algorithm_id_t *algorithms = malloc(count * sizeof(uci_algorithm_id_t));
    if (algorithms) {
        uci_list_algorithms(-1, algorithms, &count);
        
        printf("Algorithm List:\n");
        printf("%-30s %-20s %-15s %-10s\n", "Name", "Type", "Security", "ID");
        printf("------------------------------------------------------------"
               "------------\n");
        
        for (size_t i = 0; i < count; i++) {
            uci_algorithm_info_t info;
            if (uci_get_algorithm_info(algorithms[i], &info) == UCI_SUCCESS) {
                const char *type_str;
                switch (info.type) {
                    case UCI_ALG_TYPE_CLASSIC:
                        type_str = "Classic";
                        break;
                    case UCI_ALG_TYPE_POST_QUANTUM:
                        type_str = "Post-Quantum";
                        break;
                    case UCI_ALG_TYPE_HYBRID:
                        type_str = "Hybrid";
                        break;
                    default:
                        type_str = "Unknown";
                }
                
                printf("%-30s %-20s %-15d %-10d\n",
                       info.name, type_str, info.security_level, info.id);
            }
        }
        
        free(algorithms);
    }
    
    printf("\n");
    
    uci_cleanup();
    
    printf("Demo completed successfully\n");
    
    return 0;
}
