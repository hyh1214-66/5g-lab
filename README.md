# 5G-Lab：5G SA 端到端仿真实验

基于 srsRAN + Open5GS + ZeroMQ 搭建的 5G SA 全软件仿真平台，
无需射频硬件，在普通 PC 上实现完整的 LTE/5G 通信链路。

## 项目结构

| 文件夹 | 内容 |
|--------|------|
| 01-environment/ | 环境搭建步骤与配置文件 |
| 02-signaling/ | 信令抓包分析与时序图 |
| 03-code-modification/ | 调度器源码改动记录 |
| 04-results/ | 实验数据与性能对比 |

## 核心成果

- ✅ UE 成功注册进 LTE 网络，获取 IP 172.16.0.2
- ✅ ping 延迟 0.018ms，丢包率 0%
- ✅ Wireshark 抓包完整记录注册信令（10条 S1AP/NAS 消息）
- 🔄 MAC 调度器改进实验进行中

## 技术栈

- srsRAN 4G（eNB + UE）
- Open5GS（MME / HSS / SPGW）
- ZeroMQ（虚拟射频）
- Ubuntu 24.04 LTS / WSL2
- Wireshark / tshark（信令分析）

## 复现步骤

详见各子文件夹的 README.md。

## 相关仓库

- [srsRAN_4G](https://github.com/srsran/srsRAN_4G)
- [Open5GS](https://github.com/open5gs/open5gs)
