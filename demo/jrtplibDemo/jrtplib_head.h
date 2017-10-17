#pragma once

// RTP Payload types (PT) https://www.ietf.org/assignments/rtp-parameters/rtp-parameters.xml
// RTP/RTCP–≠“È∑÷Œˆ http://blog.csdn.net/bripengandre/article/details/2238818

#include <jrtplib3/rtpsession.h>
#include <jrtplib3/rtpudpv4transmitter.h>
#include <jrtplib3/rtpipv4address.h>
#include <jrtplib3/rtpsessionparams.h>
#include <jrtplib3/rtperrors.h>
#include <jrtplib3/rtplibraryversion.h>
#include <jrtplib3/rtppacket.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace jrtplib;

#ifdef _DEBUG
#   pragma comment(lib, "jrtplib_d.lib")
#else
#   pragma comment(lib, "jrtplib.lib")
#endif // _DEBUG

#define checkerror(rtperr) { if (rtperr < 0) { std::cout << "ERROR: " << jrtplib::RTPGetErrorString(rtperr) << std::endl; }}


class MyRTPSession : public jrtplib::RTPSession
{
public:

protected:
    virtual void OnSentRTPOrRTCPData(void *senddata, size_t sendlen, bool isrtp) override;
    virtual RTPTransmitter * NewUserDefinedTransmitter() override;
    virtual void OnRTPPacket(RTPPacket *pack, const RTPTime &receivetime, const RTPAddress *senderaddress) override;
    virtual void OnRTCPCompoundPacket(RTCPCompoundPacket *pack, const RTPTime &receivetime, const RTPAddress *senderaddress) override;
    virtual void OnSSRCCollision(RTPSourceData *srcdat, const RTPAddress *senderaddress, bool isrtp) override;
    virtual void OnCNAMECollision(RTPSourceData *srcdat, const RTPAddress *senderaddress, const uint8_t *cname, size_t cnamelength) override;
    virtual void OnNewSource(RTPSourceData *srcdat) override;
    virtual void OnRemoveSource(RTPSourceData *srcdat) override;
    virtual void OnTimeout(RTPSourceData *srcdat) override;
    virtual void OnBYETimeout(RTPSourceData *srcdat) override;
    virtual void OnAPPPacket(RTCPAPPPacket *apppacket, const RTPTime &receivetime, const RTPAddress *senderaddress) override;
    virtual void OnUnknownPacketType(RTCPPacket *rtcppack, const RTPTime &receivetime, const RTPAddress *senderaddress) override;
    virtual void OnUnknownPacketFormat(RTCPPacket *rtcppack, const RTPTime &receivetime, const RTPAddress *senderaddress) override;
    virtual void OnNoteTimeout(RTPSourceData *srcdat) override;
    virtual void OnRTCPSenderReport(RTPSourceData *srcdat) override;
    virtual void OnRTCPReceiverReport(RTPSourceData *srcdat) override;
    virtual void OnRTCPSDESItem(RTPSourceData *srcdat, RTCPSDESPacket::ItemType t, const void *itemdata, size_t itemlength) override;
    virtual void OnRTCPSDESPrivateItem(RTPSourceData *srcdat, const void *prefixdata, size_t prefixlen, const void *valuedata, size_t valuelen) override;
    virtual void OnBYEPacket(RTPSourceData *srcdat) override;
    virtual void OnSendRTCPCompoundPacket(RTCPCompoundPacket *pack) override;
    virtual int OnChangeRTPOrRTCPData(const void *origdata, size_t origlen, bool isrtp, void **senddata, size_t *sendlen) override;
    virtual bool OnChangeIncomingData(RTPRawPacket *rawpack) override;
    virtual void OnValidatedRTPPacket(RTPSourceData *srcdat, RTPPacket *rtppack, bool isonprobation, bool *ispackethandled) override;

};
