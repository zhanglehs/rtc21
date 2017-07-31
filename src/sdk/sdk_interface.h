#pragma once

namespace live_stream_sdk
{
  class RtpNetworkObserver {
  public:
    RtpNetworkObserver() {}
    virtual ~RtpNetworkObserver() {}

    virtual void OnNetworkState(int state_no, int wParam = 0, int lParam = 0) {}
    virtual void OnReceivedRTCP(const unsigned char* data, unsigned int len, int type) {}
    virtual void OnReceivedRTP(unsigned char* data, unsigned int len, int type) {}
    virtual void OnReceivedSDP(const char* sdp) {}
  };
}
