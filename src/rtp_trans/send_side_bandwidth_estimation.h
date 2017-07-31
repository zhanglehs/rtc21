/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 *
 *  FEC and NACK added bitrate is handled outside class
 */

#ifndef WEBRTC_MODULES_BITRATE_CONTROLLER_SEND_SIDE_BANDWIDTH_ESTIMATION_H_
#define WEBRTC_MODULES_BITRATE_CONTROLLER_SEND_SIDE_BANDWIDTH_ESTIMATION_H_

#include <pthread.h>
#include <deque>
#include <stdint.h>
#include <list>
#include <vector>
#include <map>

//#include "webrtc/modules/rtp_rtcp/interface/rtp_rtcp_defines.h"
//#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

namespace avformat {
  struct RTCPReportBlock;
}

namespace live_stream_sdk {
  class SendSideBandwidthEstimation {
  public:
    SendSideBandwidthEstimation();
    virtual ~SendSideBandwidthEstimation();

    void CurrentEstimate(uint32_t* bitrate, uint8_t* loss, int64_t* rtt);

    // Call periodically to update estimate.
    void UpdateEstimate(int64_t now_ms);

    void SetSendBitrate(uint32_t bitrate);
    void SetMinMaxBitrate(uint32_t min_bitrate, uint32_t max_bitrate);
    uint32_t GetMinBitrate() const;

    // zhangle
    // Received RTCP receiver block.
    void OnReceivedRtcpReceiverReport(const std::list<avformat::RTCPReportBlock>& report_blocks,
      int64_t rtt,
      int64_t now_ms);

    // zhangle
    // Received RTCP REMB or TMMBR.
    void OnReceivedEstimatedBitrate(uint32_t bitrate);

  private:
    enum UmaState { kNoUpdate, kFirstDone, kDone };

    // zhangle
    // Call when we receive a RTCP message with a ReceiveBlock.
    void UpdateReceiverBlock(uint8_t fraction_loss,
      int64_t rtt,
      int number_of_packets,
      int64_t now_ms);

    // zhangle
    // Call when we receive a RTCP message with TMMBR or REMB.
    void UpdateReceiverEstimate(uint32_t bandwidth);

    bool IsInStartPhase(int64_t now_ms) const;

    void UpdateUmaStats(int64_t now_ms, int64_t rtt, int lost_packets);

    // Returns the input bitrate capped to the thresholds defined by the max,
    // min and incoming bandwidth.
    uint32_t CapBitrateToThresholds(uint32_t bitrate);

    // Updates history of min bitrates.
    // After this method returns min_bitrate_history_.front().second contains the
    // min bitrate used during last kBweIncreaseIntervalMs.
    void UpdateMinHistory(int64_t now_ms);

    std::deque<std::pair<int64_t, uint32_t> > min_bitrate_history_;

    // incoming filters
    int accumulate_lost_packets_Q8_;
    int accumulate_expected_packets_;

    uint32_t bitrate_;
    uint32_t min_bitrate_configured_;
    uint32_t max_bitrate_configured_;

    int64_t time_last_receiver_block_ms_;
    uint8_t last_fraction_loss_;
    int64_t last_round_trip_time_ms_;

    uint32_t bwe_incoming_;
    int64_t time_last_decrease_ms_;
    int64_t first_report_time_ms_;
    int initially_lost_packets_;
    int bitrate_at_2_seconds_kbps_;
    UmaState uma_update_state_;
    std::vector<bool> rampup_uma_stats_updated_;

    // zhangle
    std::map<uint32_t, uint32_t> ssrc_to_last_received_extended_high_seq_num_;
    pthread_mutex_t mutex_;

    uint32_t rtt_bitrate_;
    std::vector<int> rtt_his_;

    uint32_t quick_recorver_count_;
    int64_t quick_recorver_ms_;
  };
}  // namespace webrtc
#endif  // WEBRTC_MODULES_BITRATE_CONTROLLER_SEND_SIDE_BANDWIDTH_ESTIMATION_H_
