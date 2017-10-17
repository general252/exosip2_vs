#include "jrtplib_head.h"


void MyRTPSession::OnRTPPacket(RTPPacket *pack, const RTPTime &receivetime, const RTPAddress *senderaddress)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnRTCPCompoundPacket(RTCPCompoundPacket *pack, const RTPTime &receivetime, const RTPAddress *senderaddress)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnSSRCCollision(RTPSourceData *srcdat, const RTPAddress *senderaddress, bool isrtp)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnCNAMECollision(RTPSourceData *srcdat, const RTPAddress *senderaddress, const uint8_t *cname, size_t cnamelength)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnNewSource(RTPSourceData *srcdat)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnRemoveSource(RTPSourceData *srcdat)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnTimeout(RTPSourceData *srcdat)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnBYETimeout(RTPSourceData *srcdat)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnAPPPacket(RTCPAPPPacket *apppacket, const RTPTime &receivetime, const RTPAddress *senderaddress)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnUnknownPacketType(RTCPPacket *rtcppack, const RTPTime &receivetime, const RTPAddress *senderaddress)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnUnknownPacketFormat(RTCPPacket *rtcppack, const RTPTime &receivetime, const RTPAddress *senderaddress)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnNoteTimeout(RTPSourceData *srcdat)
{
    printf("%s\n", __FUNCTION__);
}


void MyRTPSession::OnRTCPSenderReport(RTPSourceData *srcdat)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnRTCPReceiverReport(RTPSourceData *srcdat)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnRTCPSDESItem(RTPSourceData *srcdat, RTCPSDESPacket::ItemType t, const void *itemdata, size_t itemlength)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnRTCPSDESPrivateItem(RTPSourceData *srcdat, const void *prefixdata, size_t prefixlen, const void *valuedata, size_t valuelen)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnBYEPacket(RTPSourceData *srcdat)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnSendRTCPCompoundPacket(RTCPCompoundPacket *pack)
{
    printf("%s\n", __FUNCTION__);
}


int MyRTPSession::OnChangeRTPOrRTCPData(const void *origdata, size_t origlen, bool isrtp, void **senddata, size_t *sendlen)
{
    printf("%s\n", __FUNCTION__);
    return RTPSession::OnChangeRTPOrRTCPData(origdata, origlen, isrtp, senddata, sendlen);
}

bool MyRTPSession::OnChangeIncomingData(RTPRawPacket *rawpack)
{
    printf("%s\n", __FUNCTION__);
    return RTPSession::OnChangeIncomingData(rawpack);
}

void MyRTPSession::OnValidatedRTPPacket(RTPSourceData *srcdat, RTPPacket *rtppack, bool isonprobation, bool *ispackethandled)
{
    printf("%s\n", __FUNCTION__);
}

void MyRTPSession::OnSentRTPOrRTCPData(void *senddata, size_t sendlen, bool isrtp)
{
    printf("%s\n", __FUNCTION__);
}

RTPTransmitter* MyRTPSession::NewUserDefinedTransmitter()
{
    printf("%s\n", __FUNCTION__);
    return RTPSession::NewUserDefinedTransmitter();
}
