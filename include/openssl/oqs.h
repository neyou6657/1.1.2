#ifndef OPENSSL_OQS_H
#define OPENSSL_OQS_H

#include <stddef.h>

#include <openssl/evp.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 便捷的算法常量，确保在应用中无需记忆底层算法名称。
 */
#define OQS_KEM_KYBER512   "kyber512"
#define OQS_KEM_KYBER768   "kyber768"
#define OQS_KEM_KYBER1024  "kyber1024"
#define OQS_SIG_DILITHIUM2 "dilithium2"
#define OQS_SIG_DILITHIUM3 "dilithium3"
#define OQS_SIG_DILITHIUM5 "dilithium5"
#define OQS_SIG_FALCON512  "falcon512"

/*
 * 主动加载/卸载 UCI Provider，方便在无需修改 openssl.cnf 的情况下完成编程式初始化。
 *
 * 返回值: 1 表示成功，0 表示失败（可通过 OpenSSL ERR API 获取具体错误）。
 */
int oqs_provider_load(void);
void oqs_provider_unload(void);

/*
 * 生成指定算法的 KEM 密钥对。
 * algorithm: 例如 OQS_KEM_KYBER768
 * keypair: 输出 EVP_PKEY*，由调用者负责 EVP_PKEY_free()
 */
int oqs_kem_keygen(const char *algorithm, EVP_PKEY **keypair);

/*
 * 使用公钥执行封装。
 * ciphertext/shared_secret 由函数内部分配，调用者负责使用 OPENSSL_free() 释放。
 */
int oqs_kem_encapsulate(EVP_PKEY *public_key,
                        unsigned char **ciphertext, size_t *ciphertext_len,
                        unsigned char **shared_secret, size_t *shared_secret_len);

/*
 * 使用私钥执行解封装。
 * shared_secret 由函数内部分配，调用者负责使用 OPENSSL_free() 释放。
 */
int oqs_kem_decapsulate(EVP_PKEY *keypair,
                        const unsigned char *ciphertext, size_t ciphertext_len,
                        unsigned char **shared_secret, size_t *shared_secret_len);

#ifdef __cplusplus
}
#endif

#endif  /* OPENSSL_OQS_H */
