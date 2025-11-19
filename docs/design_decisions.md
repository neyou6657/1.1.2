# 设计决策文档

## 概述

本文档记录了在设计和实现统一密码服务接口（UCI）过程中做出的关键设计决策，以及这些决策的理由和参考来源。

## 1. 架构选择：独立接口 vs OpenSSL Provider

### 问题

在设计UCI时，面临两个主要的架构选择：

**选项A**: 基于OpenSSL 3.0 Provider架构（如oqs-provider项目）
- 深度集成OpenSSL生态
- 用户可以通过标准OpenSSL API使用
- 依赖OpenSSL 3.0+

**选项B**: 设计独立的统一接口层（当前选择）
- 提供独立的API接口
- 适配多个底层密码库
- 保持架构灵活性

### 决策

**选择选项B：独立接口层架构**

### 理由

1. **符合毕设定位**：
   - 毕设核心是"统一密码服务接口设计"，而不是"OpenSSL扩展开发"
   - 需要展示接口抽象能力和多库集成能力
   - 独立接口更能体现设计的原创性

2. **灵活性更强**：
   - 不受OpenSSL版本限制
   - 可以同时支持OpenSSL 1.x和3.x
   - 可以集成任意密码库（GmSSL、LibOQS、国密设备等）

3. **研究价值更高**：
   - 需要分析和对比不同库的接口差异
   - 需要设计统一的抽象层
   - 更符合学术研究的要求

4. **实用性考虑**：
   - 许多系统仍在使用OpenSSL 1.x
   - 嵌入式系统可能没有OpenSSL
   - 需要支持国密设备接口

### 参考

- [oqs-provider](https://github.com/open-quantum-safe/oqs-provider): 借鉴其算法注册机制和初始化流程
- [GmSSL兼容层](https://github.com/GmSSL/OpenSSL-Compatibility-Layer): 借鉴其适配器设计模式

## 2. 算法注册机制

### 问题

如何管理和注册多种密码算法？

### 决策

采用**动态注册表机制**，参考oqs-provider的设计。

### 实现

```c
typedef struct {
    uci_algorithm_info_t info;          // 算法元信息
    uci_keygen_func_t keygen;           // 函数指针
    uci_sign_func_t sign;
    uci_verify_func_t verify;
    // ... 其他操作
} uci_algorithm_impl_t;

// 注册表
static uci_algorithm_impl_t *algorithm_table[MAX_ALGORITHMS];

// 注册函数
int registry_register_algorithm(const uci_algorithm_impl_t *impl);
const uci_algorithm_impl_t *registry_get_algorithm(uci_algorithm_id_t algorithm);
```

### 优点

1. **可扩展性**：新增算法无需修改核心代码
2. **运行时发现**：可以在初始化时动态注册可用算法
3. **统一管理**：所有算法通过注册表统一查询
4. **性能优化**：可以升级为哈希表实现O(1)查找

### 借鉴来源

oqs-provider的`oqs_prov_init`和算法注册机制。

## 3. 适配器层设计

### 问题

如何封装不同密码库的接口差异？

### 决策

采用**适配器模式**，为每个密码库创建独立的适配器。

### 架构

```
应用层
    ↓
统一接口层 (uci_*)
    ↓
算法注册层
    ↓
适配器层
    ├── OpenSSL Adapter
    ├── LibOQS Adapter
    ├── GmSSL Adapter
    └── Hybrid Adapter
    ↓
底层密码库
```

### 适配器职责

1. **接口转换**：将底层库的接口转换为UCI格式
2. **参数转换**：处理不同的数据结构
3. **错误映射**：统一错误码
4. **内存管理**：统一内存分配和释放

### 示例：OpenSSL适配器

```c
// OpenSSL的复杂接口
EVP_PKEY *pkey = NULL;
EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
EVP_PKEY_keygen_init(ctx);
EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048);
EVP_PKEY_keygen(ctx, &pkey);
// ... 导出为DER格式

// UCI的简单接口
uci_keypair_t keypair;
uci_keygen(UCI_ALG_RSA2048, &keypair);
```

### 借鉴来源

GmSSL兼容层的适配器设计思路。

## 4. 密钥和签名的数据格式

### 问题

如何统一表示不同算法的密钥和签名？

### 决策

采用**DER编码的字节数组**作为统一格式。

### 数据结构

```c
typedef struct {
    uci_algorithm_id_t algorithm;    // 算法标识
    uci_algorithm_type_t type;       // 算法类型
    uint8_t *public_key;              // 公钥（DER格式）
    size_t public_key_len;            // 公钥长度
    uint8_t *private_key;             // 私钥（DER格式）
    size_t private_key_len;           // 私钥长度
} uci_keypair_t;
```

### 优点

1. **通用性**：所有算法都可以编码为字节数组
2. **互操作性**：DER是标准格式，易于导入导出
3. **扩展性**：可以支持任意长度的密钥
4. **兼容性**：与现有工具（OpenSSL CLI等）兼容

### 替代方案（未采用）

1. **特定结构体**：每种算法定义专门的结构体
   - ❌ 不够通用，扩展性差
   
2. **字符串格式（PEM）**：使用PEM编码
   - ❌ 需要额外的Base64编解码开销
   - ✅ 但可以作为导出格式提供

## 5. 混合密码方案的实现

### 问题

如何实现经典算法与抗量子算法的混合方案？

### 决策

采用**密钥和签名的串联格式**。

### 混合密钥格式

```
Hybrid Public Key:
[classic_pk_len(8)] [classic_pk] [pq_pk_len(8)] [pq_pk]

Hybrid Private Key:
[classic_sk_len(8)] [classic_sk] [pq_sk_len(8)] [pq_sk]

Hybrid Signature:
[classic_sig_len(8)] [classic_sig] [pq_sig_len(8)] [pq_sig]
```

### 验证策略

```c
int hybrid_verify() {
    // 两种签名都必须验证通过
    if (verify_classic() != SUCCESS) return FAIL;
    if (verify_pq() != SUCCESS) return FAIL;
    return SUCCESS;
}
```

### 优点

1. **双重安全**：同时提供经典和抗量子保护
2. **向后兼容**：经典系统可以验证经典部分
3. **前向兼容**：为量子时代做准备
4. **透明性**：对应用层完全透明

### 安全性讨论

- **AND策略**：两种签名都必须有效（当前采用）
  - ✅ 最高安全性
  - ❌ 任一算法失败都导致验证失败
  
- **OR策略**：任一签名有效即可（未采用）
  - ❌ 安全性降低到最弱的算法
  - ✅ 兼容性更好

## 6. 初始化顺序和依赖关系

### 问题

多个适配器的初始化顺序？

### 决策

按照依赖关系初始化：

```c
int uci_init(void) {
    registry_init();           // 1. 注册表
    openssl_adapter_init();    // 2. OpenSSL（经典算法）
    classic_adapter_init();    // 3. 其他经典算法（GmSSL）
    pqc_adapter_init();        // 4. 抗量子算法（LibOQS）
    hybrid_adapter_init();     // 5. 混合算法（依赖前面的）
}
```

### 理由

1. **依赖顺序**：混合算法依赖经典和抗量子算法
2. **错误处理**：失败时按相反顺序清理
3. **可选性**：如果某个库不可用，其他仍可工作

## 7. 错误处理策略

### 问题

如何统一不同库的错误处理？

### 决策

定义**统一的错误码体系**。

### 错误码定义

```c
#define UCI_SUCCESS 0                    // 成功
#define UCI_ERROR_INVALID_PARAM -1       // 参数错误
#define UCI_ERROR_NOT_SUPPORTED -2       // 操作不支持
#define UCI_ERROR_BUFFER_TOO_SMALL -3    // 缓冲区太小
#define UCI_ERROR_ALGORITHM_NOT_FOUND -4 // 算法未找到
#define UCI_ERROR_INTERNAL -5            // 内部错误
#define UCI_ERROR_SIGNATURE_INVALID -6   // 签名无效
```

### 错误映射

```c
// OpenSSL错误 → UCI错误
if (EVP_DigestSign(...) <= 0) {
    return UCI_ERROR_INTERNAL;
}

// LibOQS错误 → UCI错误
if (OQS_SIG_sign(...) != OQS_SUCCESS) {
    return UCI_ERROR_INTERNAL;
}
```

### 优点

1. **一致性**：所有接口返回相同的错误码
2. **可读性**：提供错误描述字符串
3. **调试友好**：易于定位问题

## 8. 条件编译策略

### 问题

如何支持可选的密码库？

### 决策

使用**条件编译宏**。

### 实现

```c
#ifdef HAVE_LIBOQS
    // LibOQS相关代码
    OQS_SIG *sig = OQS_SIG_new(...);
#else
    // 降级实现
    return UCI_ERROR_NOT_SUPPORTED;
#endif
```

### CMake配置

```cmake
option(USE_LIBOQS "Build with LibOQS support" ON)

if(USE_LIBOQS)
    find_library(LIBOQS_LIBRARY oqs)
    if(LIBOQS_LIBRARY)
        add_definitions(-DHAVE_LIBOQS)
    endif()
endif()
```

### 优点

1. **灵活性**：可以按需包含库
2. **可移植性**：在缺少某些库的系统上仍可编译
3. **最小化依赖**：减少不必要的依赖
4. **性能优化**：避免链接未使用的库

## 9. 内存管理原则

### 问题

谁负责分配和释放内存？

### 决策

**调用者负责内存管理**原则。

### 规则

1. **密钥生成**：UCI分配，调用者释放
   ```c
   uci_keygen(&keypair);         // UCI分配
   // 使用密钥
   uci_keypair_free(&keypair);   // 调用者释放
   ```

2. **签名**：UCI分配，调用者释放
   ```c
   uci_sign(&keypair, msg, len, &sig);  // UCI分配
   // 使用签名
   uci_signature_free(&sig);            // 调用者释放
   ```

3. **验证**：调用者提供所有数据
   ```c
   uci_verify(&keypair, msg, len, &sig); // 无分配
   ```

### 安全性考虑

```c
int uci_keypair_free(uci_keypair_t *keypair) {
    if (keypair->private_key) {
        // 清零敏感数据
        memset(keypair->private_key, 0, keypair->private_key_len);
        free(keypair->private_key);
    }
    // ...
}
```

## 10. 未来扩展考虑

### 已预留的扩展点

1. **算法注册机制**：支持运行时动态注册自定义算法

2. **操作扩展**：预留了加密/解密接口
   ```c
   int uci_encrypt(...);
   int uci_decrypt(...);
   ```

3. **密钥格式**：可以添加PEM导入导出
   ```c
   int uci_keypair_to_pem(...);
   int uci_keypair_from_pem(...);
   ```

4. **性能优化**：注册表可升级为哈希表

5. **硬件加速**：适配器可以对接HSM/TPM

### 待实现的功能

1. **密钥序列化**：PEM/DER格式的完整支持
2. **证书集成**：X.509证书的生成和验证
3. **流式接口**：支持大数据的分块处理
4. **异步接口**：支持异步密码操作
5. **硬件适配器**：对接密码设备

## 总结

UCI的设计决策充分考虑了：

1. **毕设定位**：独立的统一接口设计
2. **参考借鉴**：oqs-provider的注册机制，GmSSL兼容层的适配思路
3. **实用性**：支持多库集成，灵活可扩展
4. **学术价值**：展示接口抽象和适配能力
5. **工程质量**：完善的错误处理，清晰的内存管理

通过这些设计决策，UCI成功实现了"面向抗量子迁移的统一密码服务接口"的目标。

## 参考文献

1. [oqs-provider](https://github.com/open-quantum-safe/oqs-provider) - OpenSSL Provider for post-quantum cryptography
2. [GmSSL OpenSSL Compatibility Layer](https://github.com/GmSSL/OpenSSL-Compatibility-Layer) - GmSSL compatibility with OpenSSL
3. NIST Post-Quantum Cryptography Standardization
4. OpenSSL Provider API Documentation
5. LibOQS Documentation
