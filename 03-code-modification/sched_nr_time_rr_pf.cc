/**
 * Copyright 2013-2023 Software Radio Systems Limited
 *
 * Modified: Added Proportional Fair (PF) scheduling algorithm
 * alongside Round-Robin for DL new transmissions.
 *
 * PF priority = current_bytes / avg_bytes
 * UE with highest PF score is scheduled first.
 */
#include "srsgnb/hdr/stack/mac/sched_nr_time_rr.h"
#include <algorithm>
#include <vector>

namespace srsenb {
namespace sched_nr_impl {

// PF scheduler state: track average throughput per UE (keyed by RNTI)
static std::map<uint16_t, float> pf_avg_bytes;
static const float PF_ALPHA = 0.9f; // smoothing factor

template <typename Predicate>
bool round_robin_apply(slot_ue_map_t& ue_db, uint32_t rr_count, Predicate p)
{
  if (ue_db.empty()) {
    return false;
  }
  auto it = ue_db.begin();
  std::advance(it, (rr_count % ue_db.size()));
  for (uint32_t count = 0; count < ue_db.size(); ++count, ++it) {
    if (it == ue_db.end()) {
      it = ue_db.begin();
    }
    if (p(it->second)) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Proportional Fair scheduling for DL new transmissions
 * Selects UE with highest (current_bytes / avg_bytes) score
 */
bool pf_apply(slot_ue_map_t& ue_db, bwp_slot_allocator& slot_alloc)
{
  // Build candidate list with PF scores
  std::vector<std::pair<float, uint16_t>> pf_scores;

  for (auto& ue_pair : ue_db) {
    slot_ue& ue = ue_pair.second;
    if (ue.dl_bytes > 0 && ue.h_dl != nullptr && ue.h_dl->empty()) {
      uint16_t rnti    = ue_pair.first;
      float    current = static_cast<float>(ue.dl_bytes);
      float    avg     = pf_avg_bytes.count(rnti) ? pf_avg_bytes[rnti] : 1.0f;
      float    score   = current / avg;
      pf_scores.push_back({score, rnti});
    }
  }

  if (pf_scores.empty()) {
    return false;
  }

  // Sort by PF score descending
  std::sort(pf_scores.begin(), pf_scores.end(),
            [](const auto& a, const auto& b) { return a.first > b.first; });

  // Try to allocate highest-scored UE
  for (auto& score_rnti : pf_scores) {
    uint16_t rnti = score_rnti.second;
    auto it2 = ue_db.begin();
    while (it2 != ue_db.end() && it2->first != rnti) ++it2;
    if (it2 == ue_db.end()) continue;
    slot_ue& ue = it2->second;
    int      ss_id = ue->find_ss_id(srsran_dci_format_nr_1_0);
    if (ss_id < 0) continue;
    prb_grant    prbs = find_optimal_dl_grant(slot_alloc, ue, ss_id);
    alloc_result res  = slot_alloc.alloc_pdsch(ue, ss_id, prbs);
    if (res == alloc_result::success) {
      // Update moving average
      pf_avg_bytes[rnti] = PF_ALPHA * pf_avg_bytes[rnti] +
                           (1.0f - PF_ALPHA) * static_cast<float>(ue.dl_bytes);
      return true;
    }
  }
  return false;
}

void sched_nr_time_rr::sched_dl_users(slot_ue_map_t& ue_db, bwp_slot_allocator& slot_alloc)
{
  // Start with retxs (keep RR for retransmissions)
  auto retx_ue_function = [&slot_alloc](slot_ue& ue) {
    if (ue.h_dl != nullptr and ue.h_dl->has_pending_retx(slot_alloc.get_tti_rx())) {
      alloc_result res = slot_alloc.alloc_pdsch(ue, ue->find_ss_id(srsran_dci_format_nr_1_0), ue.h_dl->prbs());
      if (res == alloc_result::success) {
        return true;
      }
    }
    return false;
  };
  if (round_robin_apply(ue_db, slot_alloc.get_pdcch_tti().to_uint(), retx_ue_function)) {
    return;
  }

  // New txs: use PF scheduler instead of RR
  pf_apply(ue_db, slot_alloc);
}

void sched_nr_time_rr::sched_ul_users(slot_ue_map_t& ue_db, bwp_slot_allocator& slot_alloc)
{
  // UL keeps RR (PF extension left as future work)
  if (round_robin_apply(ue_db, slot_alloc.get_pdcch_tti().to_uint(), [&slot_alloc](slot_ue& ue) {
        if (ue.h_ul != nullptr and ue.h_ul->has_pending_retx(slot_alloc.get_tti_rx())) {
          alloc_result res = slot_alloc.alloc_pusch(ue, ue.h_ul->prbs());
          if (res == alloc_result::success) {
            return true;
          }
        }
        return false;
      })) {
    return;
  }
  round_robin_apply(ue_db, slot_alloc.get_pdcch_tti().to_uint(), [&slot_alloc](slot_ue& ue) {
    if (ue.ul_bytes > 0 and ue.h_ul != nullptr and ue.h_ul->empty()) {
      alloc_result res = slot_alloc.alloc_pusch(ue, prb_interval{0, slot_alloc.cfg.cfg.rb_width});
      if (res == alloc_result::success) {
        return true;
      }
    }
    return false;
  });
}

} // namespace sched_nr_impl
} // namespace srsenb
