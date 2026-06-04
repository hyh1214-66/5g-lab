# 代码改动记录

基于 srsRAN 源码实现调度算法改进实验。

## 改动目标
将 MAC 层默认的 Round-Robin（RR）调度器替换为
比例公平（Proportional Fair，PF）调度算法，
在多 UE 并发场景下提升资源分配公平性。

## 改动文件
- 位置：srsRAN_4G/lib/scheduler/
- 核心文件：ue_scheduler_impl.cpp

## 评估指标
- Jain 公平指数（Jain's Fairness Index）
- 多 UE 并发吞吐量（iperf3 测量）

## 实验状态
- [ ] 阅读调度器源码
- [ ] 实现 PF 算法
- [ ] 多 UE 场景测试
- [ ] 记录实验数据
