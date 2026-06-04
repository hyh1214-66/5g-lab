# 调度器改动：Round-Robin → Proportional Fair

## 改动背景

srsRAN 默认 MAC 调度器采用 Round-Robin（RR）算法，
轮流给每个 UE 分配资源，不考虑信道质量差异，
在多 UE 场景下资源利用率不够理想。

## 改动内容

在 DL 新传调度部分引入 Proportional Fair（PF）算法。

**改动文件：**
`srsgnb/src/stack/mac/sched_nr_time_rr.cc`

**核心公式：**
PF 优先级分数 = 当前可达速率 / 历史平均速率
**实现细节：**
- 用 `std::map<uint16_t, float>` 记录每个 UE 的历史平均吞吐量
- 平滑因子 α = 0.9，指数加权移动平均更新历史值
- 每个调度时隙对所有候选 UE 计算 PF 分数并排序
- 分数最高的 UE 优先获得 PRB 资源
- 重传（retx）保留 RR 调度，保证时延
- UL 方向保留 RR，PF 扩展作为后续工作

**改动前（RR）：**
```cpp
round_robin_apply(ue_db, slot_alloc.get_pdcch_tti().to_uint(), newtx_ue_function);
```

**改动后（PF）：**
```cpp
pf_apply(ue_db, slot_alloc);
```

## 编译验证

```bash
cd srsRAN_Project/build
make -j$(nproc)
# 结果：[100%] Built target srsenb — 编译成功，无报错
```

## 预期效果

| 指标 | Round-Robin | Proportional Fair |
|------|-------------|-------------------|
| 资源分配策略 | 轮流均分 | 按信道质量加权 |
| 公平性（Jain Index） | 高 | 高（理论保证） |
| 系统总吞吐量 | 基准 | 预期提升 10~20% |
| 实现复杂度 | O(n) | O(n log n) |

## 源码文件

改动后的完整源码见 `sched_nr_time_rr_pf.cc`
原始备份见 srsRAN 仓库中的 `.bak` 文件
