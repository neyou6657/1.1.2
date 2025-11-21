# 统一密码服务接口 (Unified Crypto Interface - UCI)

面向抗量子迁移的统一密码服务接口设计与实现

## 项目简介

本项目实现了一个统一的密码服务接口，旨在解决在抗量子密码迁移过程中，经典算法与抗量子算法长期共存导致的接口碎片化问题。通过提供统一的API接口，实现对经典密码算法、抗量子密码算法以及混合密码方案的透明化支持。

## 核心特性

### 1. 统一接口设计

- **算法无关性**: 提供统一的API接口，支持经典、抗量子和混合密码算法
- **操作标准化**: 将密钥生成、签名、验证、加密、解密等操作封装为标准化的原子操作
- **参数归一化**: 统一的参数结构和返回值设计，简化上层应用开发

### 2. 多算法支持

#### 经典密码算法
- RSA (RSA-2048, RSA-3072, RSA-4096)
- ECDSA (P-256, P-384)
- SM2/SM3/SM4 (国密算法)

#### 抗量子密码算法

**数字签名算法**:
- Dilithium (Dilithium2, Dilithium3, Dilithium5)
- Falcon (Falcon-512, Falcon-1024)
- SPHINCS+ (SHA256-128f, SHA256-192f, SHA256-256f)

**密钥封装机制 (KEM)**:
- Kyber (Kyber512, Kyber768, Kyber1024)
- NTRU (HPS2048509, HPS2048677, HPS4096821)
- SABER (LightSaber, Saber, FireSaber)

#### 混合密码方案
- Hybrid-RSA-Dilithium: 结合RSA-2048和Dilithium2
- Hybrid-ECDSA-Dilithium: 结合ECDSA-P256和Dilithium2
- Hybrid-RSA-Kyber: RSA密钥交换与Kyber结合
- Hybrid-ECDH-Kyber: ECDH与Kyber的混合KEM

### 3. 模块化架构

```
┌─────────────────────────────────────────┐
│        应用层 (Application Layer)       │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│   统一接口层 (Unified Interface Layer)  │
│  - uci_keygen()                         │
│  - uci_sign() / uci_verify()            │
│  - uci_encrypt() / uci_decrypt()        │
│  - uci_kem_encaps() / uci_kem_decaps()  │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│   算法注册层 (Algorithm Registry)       │
│  - 算法注册与管理                       │
│  - 算法查询与枚举                       │
└─────────────────────────────────────────┘
                    ↓
┌──────────────┬──────────────┬───────────┐
│   经典算法   │  抗量子算法  │ 混合算法  │
│   适配器     │    适配器    │  适配器   │
├──────────────┼──────────────┼───────────┤
│  OpenSSL     │   LibOQS     │  Hybrid   │
│  GmSSL       │              │  Crypto   │
└──────────────┴──────────────┴───────────┘
```

## 项目结构

```
.
├── CMakeLists.txt              # CMake构建配置
├── README_UCI.md               # 项目文档
├── include/                    # 头文件目录
│   ├── unified_crypto_interface.h    # 统一接口定义
│   ├── algorithm_registry.h          # 算法注册管理
│   ├── classic_crypto_adapter.h      # 经典算法适配器
│   ├── pqc_adapter.h                 # 抗量子算法适配器
│   └── hybrid_crypto.h               # 混合密码方案
├── src/                        # 源代码目录
│   ├── unified_crypto_interface.c
│   ├── algorithm_registry.c
│   ├── classic_crypto_adapter.c
│   ├── pqc_adapter.c
│   └── hybrid_crypto.c
├── examples/                   # 示例程序
│   ├── demo.c                  # 基础演示
│   ├── list_algorithms.c       # 算法列表
│   ├── signature_demo.c        # 数字签名演示
│   └── kem_demo.c              # KEM演示
├── tests/                      # 测试程序
│   └── test_basic.c
├── libs/                       # 第三方库
│   ├── liboqs/                 # LibOQS (抗量子密码库)
│   └── GmSSL/                  # GmSSL (国密库)
└── docs/                       # 文档目录
```

## 编译与安装

### 依赖项

- CMake 3.10+
- GCC 或 Clang 编译器
- OpenSSL 1.1.1+（推荐 3.0+，用于经典算法与Provider功能）
- LibOQS (可选，用于抗量子算法支持)
- GmSSL (可选，用于国密算法支持)

#### 安装 OpenSSL

UCI 依赖系统级的 OpenSSL，请使用包管理器安装对应的运行库和开发头文件：

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install openssl libssl-dev

# CentOS/RHEL
sudo yum install openssl openssl-devel

# macOS (Homebrew)
brew install openssl@3
```

安装完成后，可通过 `openssl version` 确认环境，若使用自定义安装路径，可在 CMake 配置时添加 `-DOPENSSL_ROOT_DIR=/path/to/openssl`。

#### OpenSSL 在 UCI 中的作用

- **经典算法适配器**：`src/openssl_adapter.c` 直接链接 OpenSSL 的 EVP API，提供 RSA、ECDSA 等经典算法。只要在 CMake 中保持 `-DUSE_OPENSSL=ON`，UCI 的 `uci_keygen/uci_sign` 等 API 会自动调用 OpenSSL 完成实际运算。
- **OpenSSL Provider**：启用 `-DBUILD_PROVIDER=ON` 后会生成 `uci.so` Provider 模块，安装到 `${OPENSSLDIR}/ossl-modules`。把它写入 `openssl.cnf` 或通过 `OPENSSL_MODULES`、`OSSL_PROVIDER` 环境变量加载后，就能用标准 `openssl` 命令访问 UCI 的 Dilithium/Kyber/Hybrid 算法。

快速检查命令：

```bash
openssl list -providers
openssl list -signature-algorithms -provider uci
openssl list -kem-algorithms -provider uci
```

更多基于 Provider 的证书、Nginx、curl 示例详见 `docs/deployment_guide.md`。

#### Provider 快速上手

1. `sudo apt install openssl libssl-dev` 或使用对应发行版的包管理器。
2. `mkdir build && cd build && cmake -DUSE_OPENSSL=ON -DUSE_LIBOQS=ON -DBUILD_PROVIDER=ON ..`
3. `make -j && sudo make install`，自动把 `uci.so` 安装到 `${OPENSSLDIR}/ossl-modules`。
4. 在 `/etc/ssl/openssl.cnf` 中加入：
   ```ini
   openssl_conf = openssl_init
   [openssl_init]
   providers = provider_sect
   [provider_sect]
   default = default_sect
   uci = uci_sect
   [uci_sect]
   activate = 1
   ```
5. 执行 `openssl list -providers`、`openssl list -kem-algorithms -provider uci` 验证加载结果。

#### C 语言示例：Kyber768 KEM

安装完成后会得到 `<openssl/oqs.h>` 辅助头文件，封装了 OpenSSL 3.0 的 KEM 流程：

```c
#include <openssl/oqs.h>

int main(void) {
    EVP_PKEY *keypair = NULL;
    unsigned char *ct = NULL, *ss_enc = NULL, *ss_dec = NULL;
    size_t ct_len = 0, ss_enc_len = 0, ss_dec_len = 0;

    oqs_provider_load();
    oqs_kem_keygen(OQS_KEM_KYBER768, &keypair);
    oqs_kem_encapsulate(keypair, &ct, &ct_len, &ss_enc, &ss_enc_len);
    oqs_kem_decapsulate(keypair, ct, ct_len, &ss_dec, &ss_dec_len);
}
```

自定义程序可直接链接 `libuci` 与系统 `libcrypto`：

```bash
cc kyber_app.c -o kyber_app -luci -lcrypto
```

编译并运行官方示例：

```bash
mkdir build && cd build
cmake -DUSE_OPENSSL=ON -DUSE_LIBOQS=ON -DBUILD_PROVIDER=ON -DBUILD_EXAMPLES=ON ..
make uci_provider_kem_demo
sudo make install
./examples/uci_provider_kem_demo
```

### 编译步骤

#### 1. 编译LibOQS (如需抗量子算法支持)

```bash
cd libs/liboqs
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=.. ..
make -j$(nproc)
make install
```

#### 2. 编译GmSSL (如需国密算法支持)

```bash
cd libs/GmSSL
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=.. ..
make -j$(nproc)
make install
```

#### 3. 编译UCI

```bash
# 在项目根目录
mkdir build && cd build

# 配置（启用所有选项）
cmake -DUSE_LIBOQS=ON -DUSE_GMSSL=ON -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON ..

# 编译
make -j$(nproc)

# 运行测试
make test

# 安装（可选）
sudo make install
```

### 编译选项

- `USE_LIBOQS`: 启用LibOQS支持 (默认: ON)
- `USE_GMSSL`: 启用GmSSL支持 (默认: ON)
- `BUILD_EXAMPLES`: 编译示例程序 (默认: ON)
- `BUILD_TESTS`: 编译测试程序 (默认: ON)

## 使用示例

### 基本使用流程

```c
#include "unified_crypto_interface.h"

int main() {
    // 1. 初始化UCI
    if (uci_init() != UCI_SUCCESS) {
        return -1;
    }
    
    // 2. 生成密钥对
    uci_keypair_t keypair;
    uci_keygen(UCI_ALG_DILITHIUM2, &keypair);
    
    // 3. 签名
    const char *message = "Hello, Post-Quantum World!";
    uci_signature_t signature;
    uci_sign(&keypair, (uint8_t*)message, strlen(message), &signature);
    
    // 4. 验证
    int result = uci_verify(&keypair, (uint8_t*)message, strlen(message), &signature);
    if (result == UCI_SUCCESS) {
        printf("Signature verified!\n");
    }
    
    // 5. 清理资源
    uci_signature_free(&signature);
    uci_keypair_free(&keypair);
    uci_cleanup();
    
    return 0;
}
```

### 数字签名示例

```c
// 使用Dilithium2进行数字签名
uci_keypair_t keypair;
uci_keygen(UCI_ALG_DILITHIUM2, &keypair);

const uint8_t *data = "Important message";
size_t data_len = strlen(data);

uci_signature_t sig;
uci_sign(&keypair, data, data_len, &sig);

// 验证签名
if (uci_verify(&keypair, data, data_len, &sig) == UCI_SUCCESS) {
    printf("Signature valid\n");
}

uci_signature_free(&sig);
uci_keypair_free(&keypair);
```

### KEM (密钥封装) 示例

```c
// 使用Kyber768进行密钥封装
uci_keypair_t keypair;
uci_kem_keygen(UCI_ALG_KYBER768, &keypair);

// 发送方：封装
uci_kem_encaps_result_t encaps_result;
uci_kem_encaps(&keypair, &encaps_result);

// 发送 encaps_result.ciphertext 到接收方

// 接收方：解封装
uint8_t shared_secret[256];
size_t secret_len = sizeof(shared_secret);
uci_kem_decaps(&keypair, encaps_result.ciphertext, 
               encaps_result.ciphertext_len, 
               shared_secret, &secret_len);

// 双方现在拥有相同的 shared_secret
uci_kem_encaps_result_free(&encaps_result);
uci_keypair_free(&keypair);
```

### 混合密码方案示例

```c
// 使用RSA+Dilithium混合签名
uci_keypair_t hybrid_keypair;
uci_keygen(UCI_ALG_HYBRID_RSA_DILITHIUM, &hybrid_keypair);

const uint8_t *message = "Hybrid signature message";
uci_signature_t hybrid_sig;

// 内部会同时使用RSA和Dilithium进行签名
uci_sign(&hybrid_keypair, message, strlen(message), &hybrid_sig);

// 验证时会同时验证两个签名
if (uci_verify(&hybrid_keypair, message, strlen(message), &hybrid_sig) == UCI_SUCCESS) {
    printf("Hybrid signature verified\n");
}

uci_signature_free(&hybrid_sig);
uci_keypair_free(&hybrid_keypair);
```

### 算法枚举

```c
// 列出所有可用算法
size_t count = 0;
uci_list_algorithms(-1, NULL, &count);

uci_algorithm_id_t *algorithms = malloc(count * sizeof(uci_algorithm_id_t));
uci_list_algorithms(-1, algorithms, &count);

for (size_t i = 0; i < count; i++) {
    uci_algorithm_info_t info;
    uci_get_algorithm_info(algorithms[i], &info);
    printf("Algorithm: %s, Security Level: %d bits\n", 
           info.name, info.security_level);
}

free(algorithms);
```

## API 参考

### 初始化与清理

- `int uci_init(void)`: 初始化UCI库
- `int uci_cleanup(void)`: 清理UCI库资源

### 算法查询

- `int uci_get_algorithm_info(uci_algorithm_id_t algorithm, uci_algorithm_info_t *info)`: 获取算法信息
- `int uci_list_algorithms(uci_algorithm_type_t type, uci_algorithm_id_t *algorithms, size_t *count)`: 列举算法

### 密钥生成

- `int uci_keygen(uci_algorithm_id_t algorithm, uci_keypair_t *keypair)`: 生成密钥对
- `int uci_kem_keygen(uci_algorithm_id_t algorithm, uci_keypair_t *keypair)`: 生成KEM密钥对
- `int uci_keypair_free(uci_keypair_t *keypair)`: 释放密钥对

### 数字签名

- `int uci_sign(const uci_keypair_t *keypair, const uint8_t *message, size_t message_len, uci_signature_t *signature)`: 签名
- `int uci_verify(const uci_keypair_t *keypair, const uint8_t *message, size_t message_len, const uci_signature_t *signature)`: 验证签名
- `int uci_signature_free(uci_signature_t *signature)`: 释放签名

### 公钥加密

- `int uci_encrypt(const uci_keypair_t *keypair, const uint8_t *plaintext, size_t plaintext_len, uci_ciphertext_t *ciphertext)`: 加密
- `int uci_decrypt(const uci_keypair_t *keypair, const uci_ciphertext_t *ciphertext, uint8_t *plaintext, size_t *plaintext_len)`: 解密
- `int uci_ciphertext_free(uci_ciphertext_t *ciphertext)`: 释放密文

### KEM操作

- `int uci_kem_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result)`: 密钥封装
- `int uci_kem_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext, size_t ciphertext_len, uint8_t *shared_secret, size_t *shared_secret_len)`: 密钥解封装
- `int uci_kem_encaps_result_free(uci_kem_encaps_result_t *result)`: 释放封装结果

### 混合密码

- `int uci_hybrid_sign(...)`: 混合签名
- `int uci_hybrid_verify(...)`: 混合验证

### 错误处理

- `const char *uci_get_error_string(int error_code)`: 获取错误描述

## 设计原则

### 1. 抽象与封装

将不同密码库的接口差异封装在适配层，向上层提供统一的抽象接口，使应用层代码与具体实现解耦。

### 2. 可扩展性

通过算法注册机制，支持动态添加新的密码算法，无需修改核心接口代码。

### 3. 向后兼容

设计时充分考虑向后兼容性，新增算法不影响现有代码。

### 4. 安全性优先

- 内存安全：所有动态分配的内存都有明确的释放路径
- 参数验证：对所有输入参数进行严格验证
- 错误处理：完善的错误码机制和错误信息

### 5. 性能考虑

- 零拷贝设计：尽可能减少内存拷贝
- 缓存友好：数据结构设计考虑缓存局部性
- 可选编译：通过条件编译支持按需包含算法

## 接口差异性分析

### 经典密码算法接口特点

1. **密钥尺寸**: 相对较小 (RSA-2048: 256字节, ECDSA-P256: 32-65字节)
2. **签名长度**: 较小 (RSA-2048: 256字节, ECDSA: 64-72字节)
3. **性能**: 成熟优化，硬件加速支持广泛
4. **安全性**: 面临量子计算威胁

### 抗量子密码算法接口特点

1. **密钥尺寸**: 较大 (Dilithium3: 公钥1952字节, 私钥4000字节)
2. **签名长度**: 显著增大 (Dilithium3: 3293字节)
3. **性能**: 新兴算法，优化空间大
4. **安全性**: 抗量子计算攻击

### 混合方案特点

1. **密钥尺寸**: 两种算法密钥的组合
2. **签名长度**: 两种签名的组合
3. **安全性**: 同时提供经典和量子安全
4. **过渡方案**: 适合迁移期使用

### 统一接口的优势

通过UCI，这些差异对上层应用完全透明，应用只需：

```c
uci_keygen(algorithm_id, &keypair);
uci_sign(&keypair, message, len, &sig);
uci_verify(&keypair, message, len, &sig);
```

无需关心具体算法的实现细节。

## 性能对比

| 算法 | 公钥(字节) | 私钥(字节) | 签名(字节) | 密钥生成 | 签名 | 验证 |
|------|----------|----------|----------|---------|------|------|
| RSA-2048 | 270 | 1190 | 256 | 慢 | 慢 | 快 |
| ECDSA-P256 | 65 | 32 | 72 | 中 | 快 | 快 |
| SM2 | 65 | 32 | 72 | 中 | 快 | 快 |
| Dilithium2 | 1312 | 2528 | 2420 | 快 | 快 | 快 |
| Dilithium3 | 1952 | 4000 | 3293 | 快 | 快 | 快 |
| Falcon-512 | 897 | 1281 | 666 | 中 | 中 | 快 |
| Kyber512 | 800 | 1632 | - | 快 | - | - |

*注: 性能数据为相对参考，实际性能取决于硬件和实现*

## 应用场景

### 1. 过渡期系统

在量子计算威胁尚未完全显现但需要提前布局的系统中，可以使用混合方案：

- 当前使用经典算法维持兼容性
- 同时部署抗量子算法做好准备
- 通过配置切换算法

### 2. 高安全系统

对于国防、金融等高安全要求的系统：

- 采用混合签名方案
- 双重验证确保安全性
- 抵御当前和未来威胁

### 3. IoT设备

资源受限的IoT设备：

- 可选择Falcon或Dilithium2等较小的算法
- 根据设备能力选择合适算法
- 统一接口便于批量管理

### 4. 区块链系统

区块链和分布式账本：

- 需要长期安全保证
- 混合签名保护交易
- 准备量子计算时代

## 未来工作

### 短期目标

- [ ] 完善RSA和ECDSA的OpenSSL适配
- [ ] 增加更多抗量子算法支持
- [ ] 性能优化和基准测试
- [ ] 完善文档和示例

### 中期目标

- [ ] 支持硬件加速
- [ ] 实现密钥管理功能
- [ ] 证书和PKI集成
- [ ] 多语言绑定 (Python, Java, Go)

### 长期目标

- [ ] 标准化接口推广
- [ ] 与主流密码库深度集成
- [ ] 支持更多混合方案
- [ ] 生产环境部署和验证

## 参考文献

1. NIST Post-Quantum Cryptography Standardization
2. LibOQS Documentation
3. GmSSL Documentation
4. RFC 8446 (TLS 1.3)
5. Hybrid Post-Quantum Key Encapsulation Methods (PQC KEMs) for Transport Layer Security 1.2 (TLS)

## 贡献指南

欢迎提交Issue和Pull Request！

### 开发规范

- 代码风格遵循项目现有风格
- 提交前运行测试确保通过
- 添加新算法需同时更新文档
- 重要修改需要添加测试用例

## 许可证

本项目采用 MIT 许可证。

## 联系方式

- 项目主页: [GitHub Repository]
- 问题反馈: [Issues]
- 邮件: [Email]

---

**北京电子科技学院毕业设计项目**  
**课题: 面向抗量子迁移的统一密码服务接口设计与实现**
