#include "unified_crypto_interface.h"
#include <stdio.h>
#include <assert.h>

int main() {
    printf("Running basic UCI tests...\n");
    
    printf("Test 1: Initialize UCI\n");
    assert(uci_init() == UCI_SUCCESS);
    printf("  PASSED\n");
    
    printf("Test 2: List algorithms\n");
    size_t count = 0;
    assert(uci_list_algorithms(-1, NULL, &count) == UCI_SUCCESS);
    printf("  Found %zu algorithms\n", count);
    printf("  PASSED\n");
    
    printf("Test 3: Get algorithm info\n");
    if (count > 0) {
        uci_algorithm_id_t *algs = malloc(count * sizeof(uci_algorithm_id_t));
        assert(algs != NULL);
        assert(uci_list_algorithms(-1, algs, &count) == UCI_SUCCESS);
        
        uci_algorithm_info_t info;
        assert(uci_get_algorithm_info(algs[0], &info) == UCI_SUCCESS);
        printf("  First algorithm: %s\n", info.name);
        
        free(algs);
    }
    printf("  PASSED\n");
    
    printf("Test 4: Cleanup UCI\n");
    assert(uci_cleanup() == UCI_SUCCESS);
    printf("  PASSED\n");
    
    printf("\nAll tests passed!\n");
    return 0;
}
