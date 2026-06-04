# 信令分析

记录 UE 注册流程的完整 S1AP/NAS 信令抓包与分析。

## 抓包环境
- 工具：tshark
- 接口：lo（本地回环）
- 过滤：SCTP 协议

## 注册流程关键消息（实测包序号）
| 包号 | 方向 | 消息名称 | 说明 |
|------|------|----------|------|
| 22 | UE→MME | Attach Request | UE 发起注册 |
| 23 | MME→UE | Identity Request | 核心网询问身份 |
| 24 | UE→MME | Identity Response | UE 回应 IMSI |
| 25 | MME→UE | Authentication Request | 下发鉴权随机数 RAND |
| 26 | UE→MME | Authentication Response | UE 返回计算结果 RES |
| 27 | MME→UE | Security Mode Command | 启动 NAS 加密 |
| 28 | UE→MME | Security Mode Complete | 加密通道建立完成 |
| 29 | MME→UE | Attach Accept | 注册成功，分配 IP |
| 32 | UE→MME | Attach Complete | UE 确认收到 |
| 33 | MME→UE | EMM Information | 下发网络信息 |

## 实验结果
- UE 获得 IP 地址：172.16.0.2
- 注册总耗时：约 0.36 秒（包22到包32）
- ping 172.16.0.1 延迟：0.018ms ~ 0.485ms，0% 丢包
