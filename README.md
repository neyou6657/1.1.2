# 统一密码服务接口 (Unified Crypto Interface)

## 项目概述

本项目是**北京电子科技学院毕业设计**的研究成果，课题名称为《面向抗量子迁移的统一密码服务接口设计与实现》。

### 研究背景

在抗量子密码迁移过程中，经典算法与抗量子算法将长期共存，同时抗量子算法仍存在不确定性，导致系统面临密码服务异构引发的接口碎片化问题。为实现混合密码生态的可持续演进，构建密码服务归一化描述体系与设计统一的密码服务调用接口成为系统设计的必然要求。

### 研究目标

构建支持抗量子密码算法和经典密码算法的统一密码服务接口，主要研究内容包括：

1. **接口差异性分析**: 研究现有密码设备接口、经典算法接口、抗量子密码算法接口，分析接口差异性
2. **统一接口设计**: 设计抽象化的密码服务接口，将经典密码服务、抗量子密码服务、混合密码服务等异构服务的调用参数、执行流程封装为统一的原子操作指令集
3. **算法集成实现**: 集成现有所有抗量子密码算法和经典密码算法，开展接口实现，支持向后兼容扩展

## 快速开始

### 系统要求

- GCC 或 Clang 编译器
- CMake 3.10+ 或 Make
- Git
- OpenSSL 1.1.1+（推荐 3.0+）以及对应的开发包（如 `libssl-dev`、`openssl-devel`）

### 克隆项目

```bash
git clone <repository-url>
cd <repository-name>
```

### 下载密码库

```bash
cd libs

# 下载 LibOQS (抗量子密码库)
git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git

# 下载 GmSSL (国密算法库)
git clone --depth 1 https://github.com/guanzhi/GmSSL.git

cd ..
```

### 安装 OpenSSL（系统级依赖）

UCI 默认使用操作系统提供的 OpenSSL，因此仓库中不会再额外拷贝 `libs/openssl` 目录。请确保系统已经安装 OpenSSL 及其开发头文件：

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install openssl libssl-dev

# CentOS/RHEL
sudo yum install openssl openssl-devel

# macOS (Homebrew)
brew install openssl@3
```

> 如果你是通过源码自定义安装 OpenSSL，可以在 CMake 配置时使用 `-DOPENSSL_ROOT_DIR=/path/to/openssl` 指定安装目录。

安装完成后，可通过 `openssl version` 验证。

### OpenSSL 在 UCI 中的角色

1. **经典算法引擎（静态链接）**：编译 UCI 时，`src/openssl_adapter.c` 会直接链接系统 OpenSSL 的 EVP 接口，提供 RSA-2048/3072/4096、ECDSA-P256/P384 等经典算法能力。应用层通过 `uci_keygen(UCI_ALG_RSA2048, …)` 这类统一 API 调用时，底层实际由 OpenSSL 完成密钥生成与签名。
2. **Provider 宿主（运行时透传）**：启用 `-DBUILD_PROVIDER=ON` 后，会生成 `uci.so` OpenSSL Provider。把它加入 `openssl.cnf`（或使用 `OPENSSL_MODULES` 环境变量）即可在标准 `openssl` 命令中直接调用 UCI 算法，如 `openssl req -new -newkey dilithium2 ...`。

快速验证：

```bash
# 编译并启用 Provider
cmake -DUSE_OPENSSL=ON -DBUILD_PROVIDER=ON ..
make -j && sudo make install

# 列出 Provider 并检查算法
openssl list -providers
openssl list -signature-algorithms -provider uci
openssl list -kem-algorithms -provider uci
```

完成上述操作后，即可继续按照下文的构建流程或 `docs/deployment_guide.md` 中的 Nginx/curl 示例进行端到端测试。

### 构建项目

#### 方法1: 使用自动构建脚本（推荐）

```bash
chmod +x build.sh
./build.sh
```

#### 方法2: 使用CMake

```bash
# 1. 构建 LibOQS
cd libs/liboqs
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=.. ..
make -j$(nproc) && make install
cd ../../..

# 2. 构建 GmSSL
cd libs/GmSSL
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=.. ..
make -j$(nproc) && make install
cd ../../..

# 3. 构建 UCI
mkdir build && cd build
cmake -DUSE_LIBOQS=ON -DUSE_GMSSL=ON -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON ..
make -j$(nproc)
```

#### 方法3: 使用Makefile

```bash
make USE_LIBOQS=1 USE_GMSSL=1
make examples USE_LIBOQS=1 USE_GMSSL=1
```

### 运行示例

```bash
cd build

# 查看所有可用算法
./examples/uci_list_algorithms

# 运行基础演示
./examples/uci_demo

# 数字签名演示
./examples/uci_signature_demo

# KEM演示
./examples/uci_kem_demo
```

### 运行测试

```bash
cd build
make test
# 或直接运行
./tests/test_basic
```

## 核心特性

### 1. 统一的API接口

无论使用经典算法、抗量子算法还是混合方案，API完全一致：

```c
#include "unified_crypto_interface.h"

// 初始化
uci_init();

// 生成密钥（适用于任何算法）
uci_keypair_t keypair;
uci_keygen(UCI_ALG_DILITHIUM2, &keypair);  // 或 UCI_ALG_RSA2048, UCI_ALG_SM2 等

// 签名
uci_signature_t signature;
uci_sign(&keypair, message, message_len, &signature);

// 验证
int result = uci_verify(&keypair, message, message_len, &signature);

// 清理
uci_signature_free(&signature);
uci_keypair_free(&keypair);
uci_cleanup();
```

### 2. 支持的算法

#### 经典密码算法
- **RSA**: RSA-2048, RSA-3072, RSA-4096
- **ECDSA**: ECDSA-P256, ECDSA-P384
- **国密**: SM2, SM3, SM4

#### 抗量子密码算法

**数字签名**:
- **Dilithium**: Dilithium2, Dilithium3, Dilithium5
- **Falcon**: Falcon-512, Falcon-1024
- **SPHINCS+**: SHA256-128f, SHA256-192f, SHA256-256f

**密钥封装机制（KEM）**:
- **Kyber**: Kyber512, Kyber768, Kyber1024
- **NTRU**: HPS2048509, HPS2048677, HPS4096821
- **SABER**: LightSaber, Saber, FireSaber

#### 混合密码方案
- **Hybrid-RSA-Dilithium**: RSA-2048 + Dilithium2
- **Hybrid-ECDSA-Dilithium**: ECDSA-P256 + Dilithium2
- **Hybrid-RSA-Kyber**: RSA + Kyber768
- **Hybrid-ECDH-Kyber**: ECDH + Kyber768

### 3. 模块化架构

```
应用层 → 统一接口层 → 算法注册层 → 适配器层 → 底层密码库
```

- **统一接口层**: 提供标准API，参数验证，错误处理
- **算法注册层**: 管理算法实现，支持动态查询
- **适配器层**: 封装不同密码库的接口差异
- **底层密码库**: LibOQS, OpenSSL, GmSSL

## 使用示例

### 示例1: 数字签名

```c
#include "unified_crypto_interface.h"
#include <stdio.h>
#include <string.h>

int main() {
    uci_init();
    
    // 使用Dilithium2签名算法
    uci_keypair_t keypair;
    uci_keygen(UCI_ALG_DILITHIUM2, &keypair);
    
    const char *message = "Hello, Post-Quantum World!";
    uci_signature_t sig;
    
    if (uci_sign(&keypair, (uint8_t*)message, strlen(message), &sig) == UCI_SUCCESS) {
        printf("Signature created: %zu bytes\n", sig.data_len);
        
        if (uci_verify(&keypair, (uint8_t*)message, strlen(message), &sig) == UCI_SUCCESS) {
            printf("Signature verified successfully!\n");
        }
    }
    
    uci_signature_free(&sig);
    uci_keypair_free(&keypair);
    uci_cleanup();
    
    return 0;
}
```

### 示例2: 密钥封装（KEM）

```c
#include "unified_crypto_interface.h"
#include <stdio.h>

int main() {
    uci_init();
    
    // 使用Kyber768 KEM
    uci_keypair_t keypair;
    uci_kem_keygen(UCI_ALG_KYBER768, &keypair);
    
    // 发送方：封装
    uci_kem_encaps_result_t result;
    uci_kem_encaps(&keypair, &result);
    
    printf("Shared secret: %zu bytes\n", result.shared_secret_len);
    printf("Ciphertext: %zu bytes\n", result.ciphertext_len);
    
    // 接收方：解封装
    uint8_t shared_secret[256];
    size_t secret_len = sizeof(shared_secret);
    uci_kem_decaps(&keypair, result.ciphertext, result.ciphertext_len,
                   shared_secret, &secret_len);
    
    printf("Decapsulated secret: %zu bytes\n", secret_len);
    
    uci_kem_encaps_result_free(&result);
    uci_keypair_free(&keypair);
    uci_cleanup();
    
    return 0;
}
```

### 示例3: 算法枚举

```c
#include "unified_crypto_interface.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    uci_init();
    
    // 查询所有抗量子算法
    size_t count = 0;
    uci_list_algorithms(UCI_ALG_TYPE_POST_QUANTUM, NULL, &count);
    
    uci_algorithm_id_t *algorithms = malloc(count * sizeof(uci_algorithm_id_t));
    uci_list_algorithms(UCI_ALG_TYPE_POST_QUANTUM, algorithms, &count);
    
    printf("Post-Quantum Algorithms:\n");
    for (size_t i = 0; i < count; i++) {
        uci_algorithm_info_t info;
        uci_get_algorithm_info(algorithms[i], &info);
        printf("  %s (Security: %d bits)\n", info.name, info.security_level);
    }
    
    free(algorithms);
    uci_cleanup();
    
    return 0;
}
```

## 项目结构

```
.
├── CMakeLists.txt              # CMake构建配置
├── Makefile                    # Makefile构建配置
├── build.sh                    # 自动构建脚本
├── README.md                   # 本文档
├── README_UCI.md               # 详细技术文档
├── include/                    # 头文件
│   ├── unified_crypto_interface.h
│   ├── algorithm_registry.h
│   ├── classic_crypto_adapter.h
│   ├── pqc_adapter.h
│   └── hybrid_crypto.h
├── src/                        # 源代码
│   ├── unified_crypto_interface.c
│   ├── algorithm_registry.c
│   ├── classic_crypto_adapter.c
│   ├── pqc_adapter.c
│   └── hybrid_crypto.c
├── examples/                   # 示例程序
│   ├── demo.c
│   ├── list_algorithms.c
│   ├── signature_demo.c
│   └── kem_demo.c
├── tests/                      # 测试程序
│   └── test_basic.c
├── docs/                       # 文档
│   ├── architecture.md         # 架构设计文档
│   └── interface_analysis.md   # 接口差异性分析
└── libs/                       # 第三方库（需手动下载）
    ├── liboqs/                 # LibOQS
    └── GmSSL/                  # GmSSL
```

## 文档

- [详细技术文档](README_UCI.md) - 完整的API参考和使用指南
- [架构设计文档](docs/architecture.md) - 系统架构和设计决策
- [接口差异性分析](docs/interface_analysis.md) - 不同密码库接口对比分析
- [设计决策文档](docs/design_decisions.md) - 关键设计决策和理由说明

## 设计亮点

### 1. 独立接口层架构

本项目**没有选择**基于OpenSSL Provider的方案（如oqs-provider），而是设计了**独立的统一接口层**：

- ✅ **参考借鉴**：借鉴oqs-provider的算法注册机制和GmSSL兼容层的适配思路
- ✅ **保持独立**：不依赖特定OpenSSL版本，提供独立的UCI API
- ✅ **多库集成**：同时支持OpenSSL、LibOQS、GmSSL等多个密码库
- ✅ **算法敏捷**：可以在运行时动态选择和切换算法

**为什么不用OpenSSL Provider？**
1. 毕设重点是"统一接口设计"，而不是OpenSSL扩展
2. 需要展示对多种密码库的抽象能力
3. OpenSSL Provider限制了灵活性和扩展性
4. 许多系统仍在使用OpenSSL 1.x或没有OpenSSL

详见：[设计决策文档](docs/design_decisions.md)

### 2. 三层适配器架构

```
应用层 (统一的UCI API)
    ↓
算法注册层 (动态注册与查询)
    ↓
适配器层
    ├── OpenSSL适配器 (RSA, ECDSA)
    ├── GmSSL适配器 (SM2, SM3, SM4)
    ├── LibOQS适配器 (Dilithium, Kyber等)
    └── 混合适配器 (Hybrid schemes)
    ↓
底层密码库
```

### 3. 统一的数据格式

所有算法使用统一的数据结构：
- 密钥：DER编码的字节数组
- 签名：带算法标识的字节数组
- 混合密钥/签名：串联格式，对应用透明

### 4. 算法敏捷性

运行时动态选择算法，无需重新编译：
```c
// 当前使用RSA
uci_keygen(UCI_ALG_RSA2048, &keypair);

// 轻松切换到抗量子算法
uci_keygen(UCI_ALG_DILITHIUM2, &keypair);

// 或使用混合方案
uci_keygen(UCI_ALG_HYBRID_RSA_DILITHIUM, &keypair);
```

## UCI Provider - OpenSSL集成

### 什么是UCI Provider？

UCI Provider是基于UCI实现的OpenSSL 3.0 Provider，它让你可以通过**标准OpenSSL API**使用UCI的所有算法。

### 双模式使用

#### 模式1: 直接使用UCI API（推荐用于新项目）

```c
#include "unified_crypto_interface.h"

uci_init();
uci_keypair_t keypair;
uci_keygen(UCI_ALG_DILITHIUM2, &keypair);
uci_sign(&keypair, message, len, &signature);
uci_cleanup();
```

**优点**：独立、简洁、不依赖OpenSSL版本

#### 模式2: 通过OpenSSL Provider（推荐用于现有项目迁移）

```c
#include <openssl/evp.h>

// UCI Provider自动加载，无需修改应用代码
EVP_PKEY *pkey = EVP_PKEY_Q_keygen(NULL, NULL, "dilithium2");
EVP_MD_CTX *ctx = EVP_MD_CTX_new();
EVP_DigestSignInit(ctx, NULL, NULL, NULL, pkey);
// 使用标准OpenSSL API...
```

**优点**：兼容现有OpenSSL应用，无需修改代码

### 实际部署：Nginx示例

#### 1. 安装UCI Provider

```bash
cd build
cmake -DBUILD_PROVIDER=ON ..
make
sudo make install
```

Provider将安装到：`/usr/local/lib/ossl-modules/uci.so`

#### 2. 配置OpenSSL

编辑 `/etc/ssl/openssl.cnf`：

```ini
[provider_sect]
uci = uci_sect

[uci_sect]
activate = 1
```

#### 3. 生成抗量子证书

```bash
# 使用Dilithium2生成证书
cd examples/deployment/nginx
./generate_certs.sh
```

#### 4. 配置Nginx

```nginx
server {
    listen 443 ssl http2;
    
    ssl_certificate /path/to/server.crt;
    ssl_certificate_key /path/to/server.key;
    
    # 启用抗量子密钥交换
    ssl_ecdh_curve X25519Kyber768:kyber768:X25519;
    ssl_protocols TLSv1.3;
}
```

#### 5. 测试连接

```bash
curl --curves kyber768 https://your-domain.com
```

详见：[部署指南](docs/deployment_guide.md)

### Provider支持的算法

通过Provider，OpenSSL应用可以使用：

**数字签名**：
- `dilithium2`, `dilithium3`, `dilithium5`
- `falcon512`
- （未来：`sphincs-sha256-128f` 等）

**KEM（密钥封装）**：
- `kyber512`, `kyber768`, `kyber1024`
- （未来：`ntru-hps2048509` 等）

**TLS组（混合算法）**：
- `X25519Kyber768` - 经典+抗量子混合

### 验证Provider安装

```bash
# 检查Provider是否加载
openssl list -providers

# 查看可用算法
openssl list -signature-algorithms -provider uci
openssl list -kem-algorithms -provider uci
```

## API参考

### 初始化与清理

```c
int uci_init(void);
int uci_cleanup(void);
```

### 密钥生成

```c
int uci_keygen(uci_algorithm_id_t algorithm, uci_keypair_t *keypair);
int uci_kem_keygen(uci_algorithm_id_t algorithm, uci_keypair_t *keypair);
int uci_keypair_free(uci_keypair_t *keypair);
```

### 数字签名

```c
int uci_sign(const uci_keypair_t *keypair, const uint8_t *message, 
             size_t message_len, uci_signature_t *signature);
int uci_verify(const uci_keypair_t *keypair, const uint8_t *message, 
               size_t message_len, const uci_signature_t *signature);
int uci_signature_free(uci_signature_t *signature);
```

### 密钥封装

```c
int uci_kem_encaps(const uci_keypair_t *keypair, uci_kem_encaps_result_t *result);
int uci_kem_decaps(const uci_keypair_t *keypair, const uint8_t *ciphertext, 
                   size_t ciphertext_len, uint8_t *shared_secret, size_t *shared_secret_len);
int uci_kem_encaps_result_free(uci_kem_encaps_result_t *result);
```

### 算法查询

```c
int uci_get_algorithm_info(uci_algorithm_id_t algorithm, uci_algorithm_info_t *info);
int uci_list_algorithms(uci_algorithm_type_t type, uci_algorithm_id_t *algorithms, size_t *count);
```

### 错误处理

```c
const char *uci_get_error_string(int error_code);
```

## 研究成果

### 1. 接口归一化

通过抽象层设计，将不同密码库（OpenSSL, LibOQS, GmSSL）的接口差异完全屏蔽，提供统一的调用方式。

### 2. 算法敏捷性

支持运行时动态选择算法，便于根据安全态势和性能需求灵活切换。

### 3. 混合密码支持

创新性地实现了混合密码方案，同时提供经典和抗量子双重保护。

### 4. 可扩展架构

基于插件式的算法注册机制，新增算法无需修改核心接口代码。

## 性能对比

| 算法 | 密钥生成 | 签名 | 验证 | 签名尺寸 |
|------|---------|------|------|---------|
| RSA-2048 | 慢 | 慢 | 快 | 256字节 |
| ECDSA-P256 | 中 | 快 | 快 | 72字节 |
| SM2 | 中 | 快 | 快 | 72字节 |
| Dilithium2 | 快 | 快 | 快 | 2420字节 |
| Dilithium3 | 快 | 快 | 快 | 3293字节 |
| Falcon-512 | 中 | 中 | 快 | 666字节 |

*注: 性能数据为相对参考*

## 应用场景

1. **TLS/SSL**: 在握手协议中支持抗量子算法协商
2. **代码签名**: 为软件和固件提供长期安全保护
3. **区块链**: 保护交易和智能合约免受量子威胁
4. **IoT安全**: 为资源受限设备提供可选的安全方案
5. **PKI基础设施**: 构建支持抗量子算法的证书体系

## 依赖项

### 运行时依赖

- **LibOQS** (可选): 提供抗量子密码算法
- **GmSSL** (可选): 提供国密算法
- **OpenSSL** (默认启用，可通过 `-DUSE_OPENSSL=OFF` 禁用): 提供经典密码算法与Provider功能

### 编译依赖

- GCC 或 Clang
- CMake 3.10+ 或 Make
- Git

## 贡献

欢迎提交Issue和Pull Request！

## 致谢

- [LibOQS](https://github.com/open-quantum-safe/liboqs) - 开源抗量子密码库
- [GmSSL](https://github.com/guanzhi/GmSSL) - 国密算法库
- NIST Post-Quantum Cryptography Standardization Project

## 许可证

MIT License

## 联系方式

**北京电子科技学院毕业设计项目**

课题名称：面向抗量子迁移的统一密码服务接口设计与实现

## 引用

如果本项目对您的研究有帮助，请引用：

```
@thesis{uci2024,
  title={面向抗量子迁移的统一密码服务接口设计与实现},
  author={[Your Name]},
  school={北京电子科技学院},
  year={2024}
}
```
