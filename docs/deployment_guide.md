# UCI Provider 部署指南

## 概述

UCI Provider是基于统一密码接口(UCI)实现的OpenSSL Provider，它允许OpenSSL应用程序使用UCI支持的所有密码算法，包括经典算法、抗量子算法和混合方案。

## 双模式使用

UCI支持两种使用模式：

### 模式1: 直接调用UCI API（独立模式）

```c
#include "unified_crypto_interface.h"

uci_init();
uci_keypair_t keypair;
uci_keygen(UCI_ALG_DILITHIUM2, &keypair);
// 使用密钥...
uci_cleanup();
```

**优点**：
- 完全独立，不依赖OpenSSL版本
- API简洁直观
- 适合新项目

### 模式2: 通过OpenSSL Provider（兼容模式）

```c
#include <openssl/evp.h>

// OpenSSL会自动加载UCI Provider
EVP_PKEY *pkey = EVP_PKEY_Q_keygen(NULL, NULL, "dilithium2");
// 使用标准OpenSSL API...
```

**优点**：
- 兼容现有OpenSSL应用
- 无需修改应用代码
- 适合迁移场景

## 安装步骤

### 前提条件

```bash
# Ubuntu/Debian
sudo apt install cmake gcc libssl-dev

# CentOS/RHEL
sudo yum install cmake gcc openssl-devel

# 确保OpenSSL版本 >= 3.0
openssl version
```

### 编译安装

```bash
# 1. 克隆项目
git clone <repository-url>
cd unified-crypto-interface

# 2. 下载依赖库
cd libs
git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git
git clone --depth 1 https://github.com/guanzhi/GmSSL.git
cd ..

# 3. 编译所有组件（包括Provider）
./build.sh

# 或使用CMake
mkdir build && cd build
cmake -DUSE_OPENSSL=ON \
      -DUSE_LIBOQS=ON \
      -DUSE_GMSSL=ON \
      -DBUILD_PROVIDER=ON \
      -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# 4. 安装
sudo make install
```

安装后的文件：
- `/usr/local/lib/libuci.so` - UCI核心库
- `/usr/local/lib/ossl-modules/uci.so` - UCI Provider
- `/usr/local/include/unified_crypto_interface.h` - UCI头文件

### 配置OpenSSL

#### 方法1: 全局配置（推荐）

编辑 `/etc/ssl/openssl.cnf`：

```ini
openssl_conf = openssl_init

[openssl_init]
providers = provider_sect

[provider_sect]
default = default_sect
uci = uci_sect

[default_sect]
activate = 1

[uci_sect]
activate = 1
```

#### 方法2: 环境变量

```bash
export OPENSSL_CONF=/path/to/custom/openssl.cnf
```

#### 方法3: 程序内配置

```c
#include <openssl/provider.h>

OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "uci");
if (prov == NULL) {
    fprintf(stderr, "Failed to load UCI provider\n");
}
```

### 验证安装

```bash
# 检查Provider是否加载
openssl list -providers

# 应该看到：
# Providers:
#   default
#   uci

# 查看可用的签名算法
openssl list -signature-algorithms -provider uci

# 应该看到：
#   dilithium2
#   dilithium3
#   dilithium5
#   falcon512

# 查看可用的KEM算法
openssl list -kem-algorithms -provider uci

# 应该看到：
#   kyber512
#   kyber768
#   kyber1024
```

## Nginx配置示例

### 生成抗量子证书

```bash
# 1. 生成CA证书（使用Dilithium2）
openssl req -x509 -new -newkey dilithium2 \
    -keyout ca.key -out ca.crt -nodes \
    -subj "/C=CN/ST=Beijing/L=Beijing/O=Test CA/CN=Test CA" \
    -days 3650

# 2. 生成服务器私钥
openssl genpkey -algorithm dilithium2 -out server.key

# 3. 生成证书签名请求
openssl req -new -key server.key -out server.csr \
    -subj "/C=CN/ST=Beijing/L=Beijing/O=Test/CN=example.com"

# 4. 签发服务器证书
openssl x509 -req -in server.csr -out server.crt \
    -CA ca.crt -CAkey ca.key -CAcreateserial \
    -days 365
```

### Nginx配置

```nginx
# /etc/nginx/nginx.conf

server {
    listen 443 ssl http2;
    server_name example.com;
    
    # 抗量子证书
    ssl_certificate /path/to/server.crt;
    ssl_certificate_key /path/to/server.key;
    
    # TLS配置
    ssl_protocols TLSv1.3;
    
    # 密钥交换算法（混合方案优先）
    ssl_ecdh_curve X25519Kyber768:kyber768:X25519:prime256v1;
    
    # 密码套件
    ssl_ciphers TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256;
    ssl_prefer_server_ciphers on;
    
    # 其他安全配置
    ssl_session_timeout 10m;
    ssl_session_cache shared:SSL:10m;
    add_header Strict-Transport-Security "max-age=31536000" always;
    
    location / {
        root /var/www/html;
        index index.html;
    }
}
```

**配置说明**：

1. `ssl_ecdh_curve`: 按优先级列出支持的密钥交换算法
   - `X25519Kyber768`: 混合算法（经典+抗量子）
   - `kyber768`: 纯抗量子算法
   - `X25519`: 经典ECDH（向后兼容）

2. `ssl_protocols TLSv1.3`: 仅启用TLS 1.3（抗量子算法需要）

3. `ssl_ciphers`: 选择强密码套件

### 重启Nginx

```bash
# 测试配置
sudo nginx -t

# 重启服务
sudo systemctl restart nginx

# 查看日志
sudo tail -f /var/log/nginx/error.log
```

## 客户端使用

### 使用curl测试

```bash
# 1. 普通连接（自动协商）
curl https://example.com

# 2. 强制使用Kyber768
curl --curves kyber768 https://example.com

# 3. 查看连接详情
curl -v --curves kyber768 https://example.com 2>&1 | grep "SSL connection"

# 成功的话会看到：
# SSL connection using TLSv1.3 / TLS_AES_256_GCM_SHA384 / kyber768
```

### Python客户端

```python
import ssl
import urllib.request

# 创建SSL上下文
context = ssl.create_default_context()
context.load_verify_locations('/path/to/ca.crt')

# 发起请求
response = urllib.request.urlopen('https://example.com', context=context)
print(response.read())
```

### OpenSSL s_client测试

```bash
# 测试TLS连接
openssl s_client -connect example.com:443 \
    -provider uci \
    -curves kyber768

# 查看服务器证书
openssl s_client -connect example.com:443 \
    -showcerts | openssl x509 -text
```

## Apache配置示例

```apache
# /etc/apache2/sites-available/example.conf

<VirtualHost *:443>
    ServerName example.com
    
    SSLEngine on
    SSLCertificateFile /path/to/server.crt
    SSLCertificateKeyFile /path/to/server.key
    
    SSLProtocol -all +TLSv1.3
    SSLOpenSSLConfCmd ECDHCurve X25519Kyber768:kyber768:X25519
    SSLCipherSuite TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256
    
    DocumentRoot /var/www/html
</VirtualHost>
```

## OpenVPN配置示例

```
# /etc/openvpn/server.conf

port 1194
proto udp
dev tun

# 使用抗量子算法
tls-version-min 1.3
tls-cipher TLS-ECDHE-ECDSA-WITH-AES-256-GCM-SHA384

# 证书
ca ca.crt
cert server.crt
key server.key
dh none  # TLS 1.3不需要DH参数

# 其他配置...
```

## 性能考虑

### 抗量子算法性能对比

| 算法 | 密钥生成 | 签名 | 验证 | 握手增加时间 |
|------|----------|------|------|-------------|
| RSA-2048 | 100ms | 50ms | 2ms | 基准 |
| ECDSA-P256 | 5ms | 3ms | 6ms | 基准 |
| Dilithium2 | 2ms | 4ms | 3ms | +10ms |
| Dilithium3 | 3ms | 5ms | 4ms | +15ms |
| Falcon-512 | 10ms | 8ms | 2ms | +20ms |
| Kyber768 | 3ms | - | - | +5ms (KEM) |

**建议**：
- **高性能需求**：Dilithium2 + Kyber512
- **平衡方案**：Dilithium3 + Kyber768
- **最高安全**：Dilithium5 + Kyber1024

### 优化配置

```nginx
# Nginx优化
worker_processes auto;
worker_rlimit_nofile 65535;

events {
    worker_connections 4096;
    use epoll;
}

http {
    # SSL会话重用
    ssl_session_cache shared:SSL:50m;
    ssl_session_timeout 1d;
    ssl_session_tickets on;
    
    # HTTP/2
    http2_max_field_size 16k;
    http2_max_header_size 32k;
}
```

## 监控和日志

### 启用详细日志

```nginx
error_log /var/log/nginx/error.log debug;
```

### 监控指标

```bash
# 查看TLS统计
openssl s_client -connect localhost:443 -status

# 查看连接状态
ss -tln | grep :443

# 性能测试
ab -n 1000 -c 10 -f TLS1.3 https://example.com/
```

## 故障排除

### Provider未加载

**症状**：
```
error:16000065:STORE routines::missing provider
```

**解决**：
1. 检查provider是否安装：`ls /usr/local/lib/ossl-modules/uci.so`
2. 检查openssl.cnf配置
3. 设置环境变量：`export OPENSSL_MODULES=/usr/local/lib/ossl-modules`

### 算法不可用

**症状**：
```
error:0308010C:digital envelope routines::unsupported
```

**解决**：
1. 确认算法名称：`openssl list -signature-algorithms -provider uci`
2. 检查LibOQS是否编译
3. 重新编译UCI：`cmake -DUSE_LIBOQS=ON ..`

### 证书验证失败

**症状**：
```
SSL certificate problem: unable to get local issuer certificate
```

**解决**：
```bash
# 安装CA证书
sudo cp ca.crt /usr/local/share/ca-certificates/
sudo update-ca-certificates

# 或指定CA路径
curl --cacert /path/to/ca.crt https://example.com
```

## 安全最佳实践

1. **混合方案优先**
   - 使用X25519Kyber768而非纯Kyber768
   - 提供双重保护

2. **仅启用TLS 1.3**
   - 旧版本协议不支持抗量子算法
   - 避免降级攻击

3. **定期更新**
   - 关注NIST标准化进展
   - 及时更新到最新版本

4. **备份方案**
   - 保留经典算法支持
   - 确保向后兼容性

5. **监控和审计**
   - 记录使用的算法
   - 监控异常连接

## 迁移策略

### 阶段1：测试部署（1-2个月）
- 在测试环境部署
- 验证兼容性
- 性能基准测试

### 阶段2：混合部署（3-6个月）
- 使用混合算法（经典+抗量子）
- 监控性能和兼容性
- 逐步扩大范围

### 阶段3：全面迁移（6-12个月）
- 所有新连接使用抗量子算法
- 保留经典算法向后兼容
- 持续监控和优化

## 参考资料

1. [OpenSSL Provider API文档](https://www.openssl.org/docs/man3.0/man7/provider.html)
2. [NIST后量子密码标准化](https://csrc.nist.gov/projects/post-quantum-cryptography)
3. [LibOQS文档](https://github.com/open-quantum-safe/liboqs/wiki)
4. [Nginx TLS配置指南](https://nginx.org/en/docs/http/ngx_http_ssl_module.html)

## 联系支持

- GitHub Issues: [项目仓库]
- 邮件: [support email]
- 文档: [项目文档]

---

**北京电子科技学院毕业设计项目**  
**面向抗量子迁移的统一密码服务接口设计与实现**
