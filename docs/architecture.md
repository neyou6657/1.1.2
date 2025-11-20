# 统一密码服务接口架构设计

## 1. 架构概述

统一密码服务接口（UCI）采用分层架构设计，通过抽象、适配和注册机制，实现了对经典密码算法、抗量子密码算法和混合密码方案的统一封装。

### 1.1 设计目标

1. **接口统一**: 提供一致的API，屏蔽底层算法实现差异
2. **算法敏捷**: 支持运行时动态选择和切换算法
3. **易于扩展**: 新增算法无需修改核心接口代码
4. **向后兼容**: 保证接口稳定性，支持长期维护
5. **性能优化**: 最小化抽象层开销

### 1.2 系统架构图

```
┌─────────────────────────────────────────────────────────────┐
│                     应用层 (Application)                     │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │
│  │ Web服务  │  │  移动应用 │  │  IoT设备  │  │  区块链  │    │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓ ↓ ↓
┌─────────────────────────────────────────────────────────────┐
│              统一密码接口层 (Unified Interface)              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  uci_init() / uci_cleanup()                          │   │
│  │  uci_keygen() / uci_keypair_free()                   │   │
│  │  uci_sign() / uci_verify()                           │   │
│  │  uci_encrypt() / uci_decrypt()                       │   │
│  │  uci_kem_keygen() / uci_kem_encaps() / uci_kem_decaps() │
│  │  uci_get_algorithm_info() / uci_list_algorithms()    │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            ↓ ↓ ↓
┌─────────────────────────────────────────────────────────────┐
│            算法注册管理层 (Algorithm Registry)               │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Algorithm Registry Table                            │   │
│  │  ┌────────┬────────┬────────┬────────┬────────┐      │   │
│  │  │ Alg 1  │ Alg 2  │ Alg 3  │  ...   │ Alg N  │      │   │
│  │  ├────────┼────────┼────────┼────────┼────────┤      │   │
│  │  │ Info   │ Info   │ Info   │  ...   │ Info   │      │   │
│  │  │ Impl   │ Impl   │ Impl   │  ...   │ Impl   │      │   │
│  │  └────────┴────────┴────────┴────────┴────────┘      │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            ↓ ↓ ↓
┌─────────────────────────────────────────────────────────────┐
│                适配器层 (Adapter Layer)                      │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐        │
│  │ Classic      │ │ Post-Quantum │ │   Hybrid     │        │
│  │ Adapter      │ │   Adapter    │ │   Adapter    │        │
│  │              │ │              │ │              │        │
│  │ - RSA        │ │ - Dilithium  │ │ - RSA+Dil.   │        │
│  │ - ECDSA      │ │ - Falcon     │ │ - ECDSA+Dil. │        │
│  │ - SM2        │ │ - Kyber      │ │ - RSA+Kyber  │        │
│  │              │ │ - NTRU       │ │ - ECDH+Kyber │        │
│  └──────────────┘ └──────────────┘ └──────────────┘        │
└─────────────────────────────────────────────────────────────┘
                            ↓ ↓ ↓
┌─────────────────────────────────────────────────────────────┐
│                底层密码库 (Crypto Libraries)                 │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐        │
│  │   OpenSSL    │ │    LibOQS    │ │    GmSSL     │        │
│  └──────────────┘ └──────────────┘ └──────────────┘        │
└─────────────────────────────────────────────────────────────┘
```

## 2. 模块详细设计

### 2.1 统一接口层 (Unified Interface Layer)

#### 2.1.1 职责
- 提供统一的外部API
- 参数验证和错误处理
- 调用算法注册层获取具体实现
- 资源生命周期管理

#### 2.1.2 核心数据结构

```c
// 密钥对结构
typedef struct {
    uci_algorithm_id_t algorithm;
    uci_algorithm_type_t type;
    uint8_t *public_key;
    size_t public_key_len;
    uint8_t *private_key;
    size_t private_key_len;
} uci_keypair_t;

// 签名结构
typedef struct {
    uci_algorithm_id_t algorithm;
    uint8_t *data;
    size_t data_len;
} uci_signature_t;

// 密文结构
typedef struct {
    uci_algorithm_id_t algorithm;
    uint8_t *ciphertext;
    size_t ciphertext_len;
} uci_ciphertext_t;

// KEM封装结果
typedef struct {
    uint8_t *shared_secret;
    size_t shared_secret_len;
    uint8_t *ciphertext;
    size_t ciphertext_len;
} uci_kem_encaps_result_t;

// 算法信息
typedef struct {
    const char *name;
    uci_algorithm_id_t id;
    uci_algorithm_type_t type;
    size_t public_key_len;
    size_t private_key_len;
    size_t signature_len;
    size_t ciphertext_overhead;
    uint32_t security_level;
} uci_algorithm_info_t;
```

#### 2.1.3 接口设计模式

**初始化模式**: 单例初始化，保证资源正确分配
```c
int uci_init(void) {
    // 1. 初始化注册表
    // 2. 初始化各适配器
    // 3. 注册所有算法
}
```

**工厂模式**: 通过算法ID创建密钥对
```c
int uci_keygen(uci_algorithm_id_t algorithm, uci_keypair_t *keypair) {
    // 1. 查找算法实现
    // 2. 调用对应的keygen函数
    // 3. 设置密钥对的算法标识
}
```

**策略模式**: 根据算法选择不同的签名策略
```c
int uci_sign(const uci_keypair_t *keypair, ...) {
    // 1. 根据keypair->algorithm查找实现
    // 2. 调用对应的sign函数
}
```

### 2.2 算法注册管理层 (Algorithm Registry)

#### 2.2.1 职责
- 维护算法注册表
- 提供算法查询和枚举功能
- 管理算法实现的生命周期

#### 2.2.2 注册表结构

```c
typedef struct {
    uci_algorithm_info_t info;          // 算法元信息
    uci_keygen_func_t keygen;           // 密钥生成函数指针
    uci_sign_func_t sign;               // 签名函数指针
    uci_verify_func_t verify;           // 验证函数指针
    uci_encrypt_func_t encrypt;         // 加密函数指针
    uci_decrypt_func_t decrypt;         // 解密函数指针
    uci_kem_keygen_func_t kem_keygen;   // KEM密钥生成
    uci_kem_encaps_func_t kem_encaps;   // KEM封装
    uci_kem_decaps_func_t kem_decaps;   // KEM解封装
} uci_algorithm_impl_t;

// 全局注册表
static uci_algorithm_impl_t *algorithm_table[MAX_ALGORITHMS];
static size_t algorithm_count = 0;
```

#### 2.2.3 注册机制

```c
// 适配器初始化时注册算法
int classic_adapter_init(void) {
    uci_algorithm_impl_t impl;
    
    // 设置算法信息
    impl.info.name = "RSA-2048";
    impl.info.id = UCI_ALG_RSA2048;
    impl.info.type = UCI_ALG_TYPE_CLASSIC;
    // ... 其他信息
    
    // 设置函数指针
    impl.keygen = classic_rsa2048_keygen;
    impl.sign = classic_rsa2048_sign;
    impl.verify = classic_rsa2048_verify;
    // ... 其他函数
    
    // 注册到注册表
    registry_register_algorithm(&impl);
}
```

#### 2.2.4 查询优化

当前实现使用线性查找O(n)，可优化为：

1. **哈希表**: 使用算法ID作为键，O(1)查找
2. **分类索引**: 按类型分组，减少搜索空间
3. **缓存**: 缓存最近使用的算法实现

### 2.3 适配器层 (Adapter Layer)

#### 2.3.1 经典密码适配器

**设计要点**:
- 封装OpenSSL/GmSSL的复杂API
- 处理对象创建和销毁
- 统一错误处理

**示例：RSA适配**
```c
int classic_rsa2048_keygen(uci_keypair_t *keypair) {
    // 1. 创建OpenSSL RSA对象
    RSA *rsa = RSA_new();
    BIGNUM *bn = BN_new();
    BN_set_word(bn, RSA_F4);
    
    // 2. 生成密钥
    RSA_generate_key_ex(rsa, 2048, bn, NULL);
    
    // 3. 导出为DER格式
    unsigned char *pub_der = NULL;
    int pub_len = i2d_RSA_PUBKEY(rsa, &pub_der);
    
    unsigned char *priv_der = NULL;
    int priv_len = i2d_RSAPrivateKey(rsa, &priv_der);
    
    // 4. 转换为UCI格式
    keypair->public_key = malloc(pub_len);
    memcpy(keypair->public_key, pub_der, pub_len);
    keypair->public_key_len = pub_len;
    
    keypair->private_key = malloc(priv_len);
    memcpy(keypair->private_key, priv_der, priv_len);
    keypair->private_key_len = priv_len;
    
    // 5. 清理OpenSSL对象
    OPENSSL_free(pub_der);
    OPENSSL_free(priv_der);
    BN_free(bn);
    RSA_free(rsa);
    
    return UCI_SUCCESS;
}
```

#### 2.3.2 抗量子密码适配器

**设计要点**:
- 利用LibOQS的相对统一接口
- 处理大尺寸密钥和签名
- 确保内存安全

**示例：Dilithium适配**
```c
int pqc_dilithium2_keygen(uci_keypair_t *keypair) {
    OQS_SIG *sig = OQS_SIG_new(OQS_SIG_alg_dilithium_2);
    if (!sig) return UCI_ERROR_INTERNAL;
    
    // 分配内存
    keypair->public_key = malloc(sig->length_public_key);
    keypair->private_key = malloc(sig->length_secret_key);
    
    // 生成密钥
    if (OQS_SIG_keypair(sig, keypair->public_key, 
                        keypair->private_key) != OQS_SUCCESS) {
        free(keypair->public_key);
        free(keypair->private_key);
        OQS_SIG_free(sig);
        return UCI_ERROR_INTERNAL;
    }
    
    keypair->public_key_len = sig->length_public_key;
    keypair->private_key_len = sig->length_secret_key;
    
    OQS_SIG_free(sig);
    return UCI_SUCCESS;
}
```

#### 2.3.3 混合密码适配器

**设计要点**:
- 组合经典和抗量子算法
- 合并密钥和签名格式
- 实现双重验证逻辑

**混合密钥格式**:
```
Hybrid Public Key:
[classic_pk_len(8)] [classic_pk] [pq_pk_len(8)] [pq_pk]

Hybrid Private Key:
[classic_sk_len(8)] [classic_sk] [pq_sk_len(8)] [pq_sk]

Hybrid Signature:
[classic_sig_len(8)] [classic_sig] [pq_sig_len(8)] [pq_sig]
```

**示例：混合签名**
```c
int hybrid_rsa_dilithium_sign(const uci_keypair_t *keypair, ...) {
    // 1. 解析混合密钥
    extract_classic_key(keypair, &classic_keypair);
    extract_pq_key(keypair, &pq_keypair);
    
    // 2. 分别签名
    classic_rsa2048_sign(&classic_keypair, message, len, &classic_sig);
    pqc_dilithium2_sign(&pq_keypair, message, len, &pq_sig);
    
    // 3. 组合签名
    combine_signatures(&classic_sig, &pq_sig, signature);
    
    return UCI_SUCCESS;
}
```

## 3. 数据流分析

### 3.1 密钥生成流程

```
应用调用 uci_keygen(UCI_ALG_DILITHIUM2, &keypair)
    ↓
[统一接口层]
    - 参数验证
    - 设置 keypair.algorithm = UCI_ALG_DILITHIUM2
    ↓
[注册表层]
    - 查找 UCI_ALG_DILITHIUM2 的实现
    - 返回 uci_algorithm_impl_t*
    ↓
[适配器层]
    - 调用 pqc_dilithium2_keygen(&keypair)
    ↓
[LibOQS]
    - OQS_SIG_new(OQS_SIG_alg_dilithium_2)
    - OQS_SIG_keypair(sig, pk, sk)
    ↓
[适配器层]
    - 分配UCI密钥结构内存
    - 拷贝密钥数据
    - 设置密钥长度
    ↓
[统一接口层]
    - 返回 UCI_SUCCESS
    ↓
应用获得 keypair（包含公钥和私钥）
```

### 3.2 签名流程

```
应用调用 uci_sign(&keypair, message, len, &signature)
    ↓
[统一接口层]
    - 参数验证（keypair != NULL, message != NULL等）
    - 根据 keypair.algorithm 查找实现
    ↓
[注册表层]
    - 查找算法实现
    - 获取 sign 函数指针
    ↓
[适配器层]
    - 根据算法类型调用相应的签名函数
    - pqc_dilithium2_sign(&keypair, message, len, &signature)
    ↓
[LibOQS]
    - OQS_SIG_sign(sig, sig_buf, &sig_len, msg, msg_len, sk)
    ↓
[适配器层]
    - 分配签名结构内存
    - signature.data = malloc(sig_len)
    - memcpy(signature.data, sig_buf, sig_len)
    - signature.data_len = sig_len
    ↓
[统一接口层]
    - signature.algorithm = keypair.algorithm
    - 返回 UCI_SUCCESS
    ↓
应用获得 signature
```

### 3.3 验证流程

```
应用调用 uci_verify(&keypair, message, len, &signature)
    ↓
[统一接口层]
    - 参数验证
    - 检查 keypair.algorithm == signature.algorithm
    - 查找算法实现
    ↓
[注册表层]
    - 返回 verify 函数指针
    ↓
[适配器层]
    - pqc_dilithium2_verify(&keypair, message, len, &signature)
    ↓
[LibOQS]
    - OQS_SIG_verify(sig, msg, msg_len, sig_data, sig_len, pk)
    - 返回 OQS_SUCCESS 或 OQS_ERROR
    ↓
[适配器层]
    - 转换为 UCI 错误码
    - OQS_SUCCESS → UCI_SUCCESS
    - OQS_ERROR → UCI_ERROR_SIGNATURE_INVALID
    ↓
[统一接口层]
    - 返回验证结果
    ↓
应用判断签名是否有效
```

## 4. 错误处理机制

### 4.1 错误码设计

```c
#define UCI_SUCCESS 0                    // 成功
#define UCI_ERROR_INVALID_PARAM -1       // 参数错误
#define UCI_ERROR_NOT_SUPPORTED -2       // 操作不支持
#define UCI_ERROR_BUFFER_TOO_SMALL -3    // 缓冲区太小
#define UCI_ERROR_ALGORITHM_NOT_FOUND -4 // 算法未找到
#define UCI_ERROR_INTERNAL -5            // 内部错误
#define UCI_ERROR_SIGNATURE_INVALID -6   // 签名无效
```

### 4.2 错误处理策略

1. **接口层**: 验证参数，返回UCI_ERROR_INVALID_PARAM
2. **注册表层**: 算法未找到，返回UCI_ERROR_ALGORITHM_NOT_FOUND
3. **适配器层**: 捕获底层错误，转换为UCI错误码
4. **底层库**: 不直接暴露给应用

### 4.3 错误传播路径

```
底层库错误 (OQS_ERROR)
    ↓ 适配器捕获
适配器层错误 (UCI_ERROR_INTERNAL)
    ↓ 向上返回
统一接口层错误
    ↓ 应用处理
应用层 (if (ret != UCI_SUCCESS) { ... })
```

## 5. 内存管理策略

### 5.1 内存所有权规则

**密钥对**:
- `uci_keygen()` 分配密钥内存
- 应用负责调用 `uci_keypair_free()` 释放
- 不允许多次释放（free后指针置NULL）

**签名**:
- `uci_sign()` 分配签名内存
- 应用负责调用 `uci_signature_free()` 释放

**KEM结果**:
- `uci_kem_encaps()` 分配共享密钥和密文内存
- 应用负责调用 `uci_kem_encaps_result_free()` 释放

### 5.2 内存泄漏防护

```c
int uci_keypair_free(uci_keypair_t *keypair) {
    if (!keypair) return UCI_ERROR_INVALID_PARAM;
    
    if (keypair->public_key) {
        // 可选：清零敏感数据
        memset(keypair->public_key, 0, keypair->public_key_len);
        free(keypair->public_key);
        keypair->public_key = NULL;
    }
    
    if (keypair->private_key) {
        memset(keypair->private_key, 0, keypair->private_key_len);
        free(keypair->private_key);
        keypair->private_key = NULL;
    }
    
    keypair->public_key_len = 0;
    keypair->private_key_len = 0;
    
    return UCI_SUCCESS;
}
```

### 5.3 内存池优化（未来）

对于频繁的密钥生成和签名操作，可实现内存池：

```c
typedef struct {
    void *pool;
    size_t block_size;
    size_t num_blocks;
    // ...
} uci_memory_pool_t;

uci_memory_pool_t *uci_create_memory_pool(size_t block_size, size_t num_blocks);
void *uci_pool_alloc(uci_memory_pool_t *pool);
void uci_pool_free(uci_memory_pool_t *pool, void *ptr);
```

## 6. 线程安全性

### 6.1 当前实现

当前实现**不是线程安全的**，主要问题：
- 全局注册表无锁保护
- 算法初始化不是原子操作

### 6.2 线程安全改进方案

#### 方案1: 全局互斥锁
```c
static pthread_mutex_t registry_mutex = PTHREAD_MUTEX_INITIALIZER;

const uci_algorithm_impl_t *registry_get_algorithm(uci_algorithm_id_t alg) {
    pthread_mutex_lock(&registry_mutex);
    // ... 查找算法
    pthread_mutex_unlock(&registry_mutex);
    return impl;
}
```

#### 方案2: 读写锁
```c
static pthread_rwlock_t registry_rwlock = PTHREAD_RWLOCK_INITIALIZER;

int registry_register_algorithm(...) {
    pthread_rwlock_wrlock(&registry_rwlock);
    // ... 注册
    pthread_rwlock_unlock(&registry_rwlock);
}

const uci_algorithm_impl_t *registry_get_algorithm(...) {
    pthread_rwlock_rdlock(&registry_rwlock);
    // ... 查找
    pthread_rwlock_unlock(&registry_rwlock);
}
```

#### 方案3: 无锁设计
- 注册表在初始化后不再修改
- 使用原子操作标记初始化状态
- 算法实现本身无状态

## 7. 性能优化

### 7.1 已实现的优化

1. **指针传递**: 避免大结构体拷贝
2. **零拷贝**: 密钥和签名使用指针，不额外拷贝
3. **按需编译**: 通过条件编译排除未使用的算法

### 7.2 可优化点

#### 哈希表注册表
```c
#define REGISTRY_HASH_SIZE 64

typedef struct registry_node {
    uci_algorithm_impl_t impl;
    struct registry_node *next;
} registry_node_t;

static registry_node_t *registry_hash_table[REGISTRY_HASH_SIZE];

static inline int hash_algorithm_id(uci_algorithm_id_t id) {
    return id % REGISTRY_HASH_SIZE;
}
```

#### 内联关键函数
```c
static inline const uci_algorithm_impl_t *
fast_get_algorithm(uci_algorithm_id_t id) {
    int hash = hash_algorithm_id(id);
    for (registry_node_t *node = registry_hash_table[hash]; 
         node != NULL; node = node->next) {
        if (node->impl.info.id == id) {
            return &node->impl;
        }
    }
    return NULL;
}
```

#### 热路径优化
```c
// 缓存最近使用的算法
static __thread const uci_algorithm_impl_t *last_used_impl = NULL;
static __thread uci_algorithm_id_t last_used_id = 0;

const uci_algorithm_impl_t *registry_get_algorithm(uci_algorithm_id_t id) {
    if (id == last_used_id && last_used_impl) {
        return last_used_impl;  // 快速路径
    }
    
    // 慢速路径：查找注册表
    const uci_algorithm_impl_t *impl = slow_lookup(id);
    if (impl) {
        last_used_id = id;
        last_used_impl = impl;
    }
    return impl;
}
```

## 8. 可扩展性设计

### 8.1 添加新算法

步骤：
1. 在 `uci_algorithm_id_t` 枚举中添加新ID
2. 实现适配器函数
3. 在适配器初始化时注册算法

**示例：添加SPHINCS+**
```c
// 1. 添加ID
typedef enum {
    // ... 现有ID
    UCI_ALG_SPHINCS_SHA256_128F = 220,
} uci_algorithm_id_t;

// 2. 实现适配器
int pqc_sphincs_sha256_128f_keygen(uci_keypair_t *keypair) {
    return pqc_sig_keygen(OQS_SIG_alg_sphincs_sha256_128f_simple, keypair);
}

int pqc_sphincs_sha256_128f_sign(...) { /* ... */ }
int pqc_sphincs_sha256_128f_verify(...) { /* ... */ }

// 3. 注册
int pqc_adapter_init(void) {
    // ... 现有注册
    
    uci_algorithm_impl_t impl;
    memset(&impl, 0, sizeof(impl));
    impl.info.name = "SPHINCS+-SHA256-128f";
    impl.info.id = UCI_ALG_SPHINCS_SHA256_128F;
    impl.info.type = UCI_ALG_TYPE_POST_QUANTUM;
    impl.keygen = pqc_sphincs_sha256_128f_keygen;
    impl.sign = pqc_sphincs_sha256_128f_sign;
    impl.verify = pqc_sphincs_sha256_128f_verify;
    registry_register_algorithm(&impl);
}
```

### 8.2 添加新操作

当前支持的操作：
- 密钥生成
- 签名/验证
- 加密/解密
- KEM封装/解封装

**未来可能的扩展**:
- 密钥协商（DH, ECDH）
- 哈希函数（SHA-256, SHA-3, SM3）
- 对称加密（AES, SM4）
- 消息认证码（HMAC, CMAC）

**扩展方法**:
```c
// 在 uci_algorithm_impl_t 中添加新函数指针
typedef struct {
    // ... 现有字段
    uci_hash_func_t hash;
    uci_mac_func_t mac;
} uci_algorithm_impl_t;

// 在统一接口中添加新API
int uci_hash(uci_algorithm_id_t algorithm, 
             const uint8_t *input, size_t input_len,
             uint8_t *output, size_t *output_len);
```

## 9. 部署架构

### 9.1 单体应用部署

```
Application Binary
├── UCI Library (libuci.so)
├── LibOQS (liboqs.so)
├── GmSSL (libgmssl.so)
└── OpenSSL (libssl.so, libcrypto.so)
```

### 9.2 微服务架构

```
┌─────────────────┐
│  Application    │
└────────┬────────┘
         │ HTTP/gRPC
         ↓
┌─────────────────┐
│ Crypto Service  │ ← UCI Library
│   (Container)   │
└────────┬────────┘
         │
         ↓
┌─────────────────┐
│  Key Management │
│     Service     │
└─────────────────┘
```

### 9.3 硬件加速场景

```
Application
    ↓
UCI Library
    ↓ (动态选择)
┌───────────┬───────────┬───────────┐
│  Software │  Hardware │  Cloud    │
│  Adapter  │  Adapter  │  Adapter  │
└───────────┴───────────┴───────────┘
    ↓            ↓            ↓
 LibOQS      HSM/TPM    Cloud KMS
```

## 10. 总结

UCI的架构设计遵循以下原则：

1. **分层清晰**: 职责分离，便于维护和测试
2. **高内聚低耦合**: 每层独立，接口明确
3. **可扩展性**: 插件式架构，易于添加新算法
4. **性能优先**: 最小化抽象开销
5. **安全第一**: 内存安全，错误处理完善

这种架构为抗量子密码迁移提供了坚实的技术基础，支持经典、抗量子和混合密码方案的平滑过渡和长期演进。
