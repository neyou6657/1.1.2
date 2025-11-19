# 更新日志 (Changelog)

## [1.1.0] - 2024-11-19

### 新增 (Added)
- ✨ **OpenSSL适配器**: 完整的RSA和ECDSA实现
  - RSA-2048, RSA-3072, RSA-4096
  - ECDSA-P256, ECDSA-P384
  - 使用OpenSSL EVP API
  - DER格式的密钥导入导出

- 📚 **设计文档**: 新增关键文档
  - `docs/design_decisions.md`: 设计决策和理由说明
  - `docs/improvements.md`: 项目改进说明
  - `CHANGELOG.md`: 更新日志

- 🔧 **CMake配置**: 添加OpenSSL支持
  - `USE_OPENSSL` 编译选项
  - 自动检测OpenSSL库

### 改进 (Improved)
- 📖 **README.md**: 添加设计亮点章节
  - 说明为什么选择独立接口而非OpenSSL Provider
  - 展示参考项目的借鉴内容
  - 强调项目的创新点和价值

- 🏗️ **架构说明**: 更新架构文档
  - 明确与oqs-provider和GmSSL兼容层的关系
  - 说明借鉴内容和创新点

### 技术决策 (Technical Decisions)

#### 为什么不使用OpenSSL Provider API？

**选择**: 独立统一接口层  
**而不是**: OpenSSL 3.0 Provider扩展

**理由**:
1. **毕设定位**: 重点是"统一密码服务接口设计"，而非OpenSSL扩展开发
2. **多库支持**: 需要同时集成OpenSSL、LibOQS、GmSSL等多个库
3. **灵活性**: 不受OpenSSL版本限制，支持OpenSSL 1.x和3.x
4. **独立价值**: 展示接口抽象和适配能力，体现原创性
5. **实用性**: 许多系统仍在使用OpenSSL 1.x或没有OpenSSL

#### 参考项目的借鉴

**oqs-provider**:
- ✅ 算法注册机制的设计思想
- ✅ 初始化和清理的流程
- ✅ 算法元信息的管理方式
- ❌ 不采用OpenSSL Provider API

**GmSSL兼容层**:
- ✅ 适配器模式的设计
- ✅ 接口转换的方法
- ✅ 参数类型的映射
- ❌ 不完全模拟OpenSSL API

### 项目价值

#### 学术价值
1. **接口抽象能力**: 设计通用的密码服务接口
2. **多库集成能力**: 证明接口的通用性和灵活性
3. **架构设计能力**: 清晰的分层和职责划分

#### 工程价值
1. **实用性**: 可以直接用于实际项目
2. **可扩展性**: 易于添加新算法和新库
3. **可维护性**: 清晰的代码结构和完善的文档

#### 创新点
1. **独立统一接口**: 不依赖特定密码库
2. **算法敏捷**: 运行时动态选择算法
3. **混合密码**: 透明的混合方案支持

---

## [1.0.0] - 2024-11-19

### 初始版本 (Initial Release)

#### 核心功能
- ✅ 统一的密码服务接口API
- ✅ 算法注册和管理系统
- ✅ LibOQS适配器（抗量子算法）
- ✅ GmSSL适配器（国密算法）
- ✅ 混合密码方案支持

#### 支持的算法
**经典算法**:
- RSA-2048/3072/4096 (占位符)
- ECDSA-P256/P384 (占位符)
- SM2/SM3/SM4 (GmSSL)

**抗量子算法**:
- Dilithium2/3/5
- Falcon-512/1024
- Kyber512/768/1024

**混合方案**:
- Hybrid-RSA-Dilithium
- Hybrid-ECDSA-Dilithium

#### 构建系统
- CMake构建配置
- Makefile支持
- 自动化构建脚本 (build.sh)

#### 文档
- README.md: 项目说明
- README_UCI.md: 详细技术文档
- docs/architecture.md: 架构设计
- docs/interface_analysis.md: 接口差异性分析

#### 示例和测试
- examples/demo.c: 基础演示
- examples/list_algorithms.c: 算法列表
- examples/signature_demo.c: 数字签名演示
- examples/kem_demo.c: KEM演示
- tests/test_basic.c: 基础测试

---

## 版本说明

### 版本号规则
遵循语义化版本 (Semantic Versioning):
- **主版本号**: 不兼容的API修改
- **次版本号**: 向后兼容的功能新增
- **修订号**: 向后兼容的问题修正

### 当前版本: 1.1.0
- 主版本 1: 基础API稳定
- 次版本 1: 新增OpenSSL适配器和设计文档
- 修订号 0: 初始发布

---

## 未来规划

### v1.2.0 (计划中)
- [ ] PEM格式的密钥导入导出
- [ ] 更多示例程序
- [ ] 性能测试和基准

### v1.3.0 (计划中)
- [ ] X.509证书支持
- [ ] 更多抗量子算法 (SPHINCS+等)
- [ ] 流式接口支持

### v2.0.0 (研究方向)
- [ ] 硬件加速支持 (HSM/TPM)
- [ ] 异步密码操作
- [ ] 分布式密钥管理

---

## 贡献者

- 主要开发者: [Your Name]
- 指导老师: [Advisor Name]
- 学校: 北京电子科技学院

## 参考项目

感谢以下开源项目提供的灵感和参考：

1. [oqs-provider](https://github.com/open-quantum-safe/oqs-provider) - OpenSSL Provider for post-quantum cryptography
2. [GmSSL兼容层](https://github.com/GmSSL/OpenSSL-Compatibility-Layer) - GmSSL compatibility with OpenSSL
3. [LibOQS](https://github.com/open-quantum-safe/liboqs) - Open Quantum Safe library
4. [GmSSL](https://github.com/guanzhi/GmSSL) - Chinese national crypto standards

## 许可证

MIT License

---

**项目主页**: [GitHub Repository]  
**问题反馈**: [Issues]  
**毕业设计**: 北京电子科技学院 - 面向抗量子迁移的统一密码服务接口设计与实现
