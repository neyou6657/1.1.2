#!/bin/bash
# 生成抗量子证书的脚本

set -e

echo "==================================="
echo "生成抗量子密码证书"
echo "==================================="
echo ""

# 配置
CERT_DIR="./ssl"
CA_SUBJECT="/C=CN/ST=Beijing/L=Beijing/O=Test CA/CN=Test CA"
SERVER_SUBJECT="/C=CN/ST=Beijing/L=Beijing/O=Test/CN=example.com"
VALIDITY_DAYS=365

# 创建目录
mkdir -p "$CERT_DIR"
cd "$CERT_DIR"

echo "Step 1: 生成CA证书（使用Dilithium2）"
openssl req -x509 -new -newkey dilithium2 \
    -keyout ca.key -out ca.crt -nodes \
    -subj "$CA_SUBJECT" \
    -days 3650 \
    -provider uci
echo "CA证书生成完成: ca.crt"
echo ""

echo "Step 2: 生成服务器私钥（使用Dilithium2）"
openssl genpkey -algorithm dilithium2 \
    -out server.key \
    -provider uci
echo "服务器私钥生成完成: server.key"
echo ""

echo "Step 3: 生成证书签名请求（CSR）"
openssl req -new -key server.key \
    -out server.csr \
    -subj "$SERVER_SUBJECT" \
    -provider uci
echo "CSR生成完成: server.csr"
echo ""

echo "Step 4: 签发服务器证书"
openssl x509 -req -in server.csr \
    -out server.crt \
    -CA ca.crt -CAkey ca.key \
    -CAcreateserial \
    -days $VALIDITY_DAYS \
    -provider uci
echo "服务器证书生成完成: server.crt"
echo ""

echo "==================================="
echo "证书生成完成！"
echo "==================================="
echo ""
echo "生成的文件："
echo "  - ca.crt: CA证书（用于客户端验证）"
echo "  - ca.key: CA私钥"
echo "  - server.crt: 服务器证书"
echo "  - server.key: 服务器私钥"
echo "  - server.csr: 证书签名请求"
echo ""
echo "安装方法："
echo "  sudo cp ca.crt server.crt server.key /etc/nginx/ssl/"
echo "  sudo chmod 600 /etc/nginx/ssl/server.key"
echo ""
echo "客户端CA证书安装："
echo "  sudo cp ca.crt /usr/local/share/ca-certificates/"
echo "  sudo update-ca-certificates"
echo ""

# 显示证书信息
echo "证书信息："
openssl x509 -in server.crt -text -noout | grep -A2 "Subject:\|Issuer:\|Validity"
