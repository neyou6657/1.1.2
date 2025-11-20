add_executable(uci_demo demo.c)
target_link_libraries(uci_demo uci)

add_executable(uci_list_algorithms list_algorithms.c)
target_link_libraries(uci_list_algorithms uci)

add_executable(uci_signature_demo signature_demo.c)
target_link_libraries(uci_signature_demo uci)

add_executable(uci_kem_demo kem_demo.c)
target_link_libraries(uci_kem_demo uci)
