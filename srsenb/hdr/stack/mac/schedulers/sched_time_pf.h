/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2020 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#ifndef SRSLTE_SCHED_TIME_PF_H
#define SRSLTE_SCHED_TIME_PF_H

#include "sched_base.h"
#include <queue>

namespace srsenb {

class sched_time_pf final : public sched_base
{
  using ue_cit_t = std::map<uint16_t, sched_ue>::const_iterator;

public:
  sched_time_pf(const sched_cell_params_t& cell_params_);
  void sched_dl_users(std::map<uint16_t, sched_ue>& ue_db, sf_sched* tti_sched) override;
  void sched_ul_users(std::map<uint16_t, sched_ue>& ue_db, sf_sched* tti_sched) override;

private:
  void new_tti(std::map<uint16_t, sched_ue>& ue_db, sf_sched* tti_sched);

  const sched_cell_params_t* cc_cfg = nullptr;

  srslte::tti_point current_tti_rx;

  struct ue_ctxt {
    ue_ctxt(uint16_t rnti_) : rnti(rnti_) {}
    float    dl_avg_rate() const { return dl_nof_samples == 0 ? 0 : dl_avg_rate_; }
    float    ul_avg_rate() const { return ul_nof_samples == 0 ? 0 : ul_avg_rate_; }
    uint32_t dl_count() const { return dl_nof_samples; }
    uint32_t ul_count() const { return ul_nof_samples; }
    void     new_tti(const sched_cell_params_t& cell, sched_ue& ue, sf_sched* tti_sched);
    void     save_dl_alloc(uint32_t alloc_bytes, float alpha);
    void     save_ul_alloc(uint32_t alloc_bytes, float alpha);

    const uint16_t rnti;

    int                 ue_cc_idx  = 0;
    float               dl_prio    = 0;
    float               ul_prio    = 0;
    const dl_harq_proc* dl_retx_h  = nullptr;
    const dl_harq_proc* dl_newtx_h = nullptr;
    const ul_harq_proc* ul_h       = nullptr;

  private:
    float    dl_avg_rate_   = 0;
    float    ul_avg_rate_   = 0;
    uint32_t dl_nof_samples = 0;
    uint32_t ul_nof_samples = 0;
  };
  std::map<uint16_t, ue_ctxt> ue_history_db;
  struct ue_dl_prio_compare {
    bool operator()(const ue_ctxt* lhs, const ue_ctxt* rhs) const;
  };
  struct ue_ul_prio_compare {
    bool operator()(const ue_ctxt* lhs, const ue_ctxt* rhs) const;
  };
  std::priority_queue<ue_ctxt*, std::vector<ue_ctxt*>, ue_dl_prio_compare> dl_queue;
  std::priority_queue<ue_ctxt*, std::vector<ue_ctxt*>, ue_ul_prio_compare> ul_queue;

  uint32_t try_dl_alloc(ue_ctxt& ue_ctxt, sched_ue& ue, sf_sched* tti_sched);
  uint32_t try_ul_alloc(ue_ctxt& ue_ctxt, sched_ue& ue, sf_sched* tti_sched);
};

} // namespace srsenb

#endif // SRSLTE_SCHED_TIME_PF_H