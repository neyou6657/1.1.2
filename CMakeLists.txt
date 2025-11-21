cmake_minimum_required(VERSION 3.10)
project(UnifiedCryptoInterface VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(USE_OPENSSL "Build with OpenSSL support" ON)
option(USE_LIBOQS "Build with LibOQS support" ON)
option(USE_GMSSL "Build with GmSSL support" ON)
option(BUILD_PROVIDER "Build OpenSSL Provider" ON)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build test programs" ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(UCI_SOURCES
    src/unified_crypto_interface.c
    src/algorithm_registry.c
    src/openssl_adapter.c
    src/classic_crypto_adapter.c
    src/pqc_adapter.c
    src/hybrid_crypto.c
)

if(USE_OPENSSL)
    find_package(OpenSSL)
    
    if(OPENSSL_FOUND)
        message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
        include_directories(${OPENSSL_INCLUDE_DIR})
        add_definitions(-DHAVE_OPENSSL)
        set(CRYPTO_LIBS ${CRYPTO_LIBS} ${OPENSSL_CRYPTO_LIBRARY})
        list(APPEND UCI_SOURCES src/openssl_oqs_helper.c)
    else()
        message(WARNING "OpenSSL not found, building without OpenSSL support")
    endif()
endif()

if(USE_LIBOQS)
    find_path(LIBOQS_INCLUDE_DIR oqs/oqs.h
        HINTS ${CMAKE_SOURCE_DIR}/libs/liboqs/build/include
    )
    find_library(LIBOQS_LIBRARY oqs
        HINTS ${CMAKE_SOURCE_DIR}/libs/liboqs/build/lib
    )
    
    if(LIBOQS_INCLUDE_DIR AND LIBOQS_LIBRARY)
        message(STATUS "Found LibOQS: ${LIBOQS_LIBRARY}")
        include_directories(${LIBOQS_INCLUDE_DIR})
        add_definitions(-DHAVE_LIBOQS)
        set(CRYPTO_LIBS ${CRYPTO_LIBS} ${LIBOQS_LIBRARY})
    else()
        message(WARNING "LibOQS not found, building without post-quantum support")
    endif()
endif()

if(USE_GMSSL)
    find_path(GMSSL_INCLUDE_DIR gmssl/sm2.h
        HINTS ${CMAKE_SOURCE_DIR}/libs/GmSSL/include /workspace/libs/GmSSL/include
    )
    find_library(GMSSL_LIBRARY gmssl
        HINTS ${CMAKE_SOURCE_DIR}/libs/GmSSL/build/lib ${CMAKE_SOURCE_DIR}/libs/GmSSL/build/bin /workspace/libs/GmSSL/lib /workspace/libs/GmSSL/bin
    )
    
    if(GMSSL_INCLUDE_DIR AND GMSSL_LIBRARY)
        message(STATUS "Found GmSSL: ${GMSSL_LIBRARY}")
        include_directories(${GMSSL_INCLUDE_DIR})
        add_definitions(-DHAVE_GMSSL)
        set(CRYPTO_LIBS ${CRYPTO_LIBS} ${GMSSL_LIBRARY})
    else()
        message(WARNING "GmSSL not found, building without GmSSL support")
    endif()
endif()

add_library(uci SHARED ${UCI_SOURCES})
add_library(uci_static STATIC ${UCI_SOURCES})
set_target_properties(uci_static PROPERTIES OUTPUT_NAME uci)

target_link_libraries(uci ${CRYPTO_LIBS})
target_link_libraries(uci_static ${CRYPTO_LIBS})

if(BUILD_PROVIDER AND USE_OPENSSL AND OPENSSL_FOUND)
    set(UCI_PROVIDER_SOURCES
        src/uci_provider.c
        src/uci_provider_sig.c
        src/uci_provider_kem.c
        src/uci_provider_keymgmt.c
    )
    
    add_library(uciprovider MODULE ${UCI_PROVIDER_SOURCES})
    target_link_libraries(uciprovider uci ${CRYPTO_LIBS})
    set_target_properties(uciprovider PROPERTIES 
        PREFIX ""
        OUTPUT_NAME "uci"
    )
    
    execute_process(
        COMMAND ${OPENSSL_EXECUTABLE} version -d
        OUTPUT_VARIABLE OPENSSL_DIR_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    string(REGEX REPLACE "^OPENSSLDIR: \"(.*)\"$" "\\1" OPENSSL_DIR "${OPENSSL_DIR_OUTPUT}")
    set(PROVIDER_INSTALL_DIR "${OPENSSL_DIR}/ossl-modules" CACHE PATH "OpenSSL provider install directory")
    
    message(STATUS "OpenSSL Provider will be built")
    message(STATUS "Provider install directory: ${PROVIDER_INSTALL_DIR}")
    
    install(TARGETS uciprovider
        LIBRARY DESTINATION ${PROVIDER_INSTALL_DIR}
    )
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS uci uci_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
