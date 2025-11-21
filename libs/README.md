# 第三方密码库

本目录用于存放第三方密码库。这些库需要单独下载。

## 下载步骤

### LibOQS (抗量子密码库)

```bash
cd libs
git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git
```

LibOQS是一个开源的抗量子密码算法库，提供了NIST标准化的各种抗量子签名算法和密钥封装机制。

**支持的算法**:
- 数字签名: Dilithium, Falcon, SPHINCS+
- KEM: Kyber, NTRU, SABER

### GmSSL (国密算法库)

```bash
cd libs
git clone --depth 1 https://github.com/guanzhi/GmSSL.git
```

GmSSL是一个开源的国密算法库，支持中国商用密码标准。

**支持的算法**:
- SM2: 椭圆曲线公钥密码
- SM3: 哈希算法
- SM4: 对称加密算法

### OpenSSL (系统级经典算法库)

UCI 的经典算法（RSA/ECDSA）与 Provider 功能依赖系统自带的 OpenSSL，因此仓库中不会额外提供 `libs/openssl` 目录。如果系统中缺少 OpenSSL，请使用包管理器安装运行库与开发头文件：

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install openssl libssl-dev

# CentOS/RHEL
sudo yum install openssl openssl-devel

# macOS (Homebrew)
brew install openssl@3
```

安装后可通过 `openssl version` 验证，若使用自编译的 OpenSSL，请在 CMake 配置时添加 `-DOPENSSL_ROOT_DIR=/path/to/openssl`，或设置 `OPENSSL_ROOT_DIR` 环境变量指向安装路径。

## 编译步骤

下载后，这些库需要先编译才能被UCI使用。

### 编译 LibOQS

```bash
cd liboqs
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=.. ..
make -j$(nproc)
make install
```

### 编译 GmSSL

```bash
cd GmSSL
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=.. ..
make -j$(nproc)
make install
```

## 自动化脚本

您也可以使用项目根目录下的 `build.sh` 脚本来自动完成编译：

```bash
cd ..  # 返回项目根目录
./build.sh
```

此脚本会自动编译LibOQS、GmSSL和UCI库。

## 目录结构

编译后的目录结构：

```
libs/
├── README.md           # 本文件
├── liboqs/             # LibOQS源码和编译产物
│   ├── build/          # 构建目录
│   ├── include/        # 头文件
│   ├── lib/            # 库文件
│   └── ...
└── GmSSL/              # GmSSL源码和编译产物
    ├── build/          # 构建目录
    ├── include/        # 头文件
    ├── lib/            # 库文件
    └── ...
```

## 注意事项

1. 这些库较大，使用 `--depth 1` 参数进行浅克隆可以节省时间和空间
2. 编译可能需要几分钟时间
3. 确保系统已安装必要的编译工具（gcc, cmake等）
4. 这些库已被添加到 `.gitignore` 中，不会被提交到版本控制

## 许可证

- LibOQS: MIT License
- GmSSL: Apache License 2.0

请遵守各库的许可证要求。
