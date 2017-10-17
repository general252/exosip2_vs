/*=============================================================================
*     FileName: jrtplib_send.cpp
*         Desc: sending packets to  destination port
*       Author: licaibiao
*   LastChange: 2017-04-10
* =============================================================================*/
#include "jrtplib_head.h"


int main_send(uint8_t rtp_ip[], uint16_t rtp_port)
{
    int status;

    MyRTPSession rtp_session;
    uint16_t local_rtp_port = 6666;

    jrtplib::RTPSessionParams sessparams;
    sessparams.SetOwnTimestampUnit(1.0 / 10.0);
    sessparams.SetAcceptOwnPackets(true);

    jrtplib::RTPUDPv4TransmissionParams transparams;
    transparams.SetPortbase(local_rtp_port);
    status = rtp_session.Create(sessparams, &transparams);
    checkerror(status);

    jrtplib::RTPIPv4Address addr(rtp_ip, rtp_port);
    status = rtp_session.AddDestination(addr);
    checkerror(status);

    int packetCount = 0;
    char sendBuffer[1500] = { 0 };
    while (true)
    {
        sprintf(sendBuffer, "this is packet %04d", ++packetCount);
        printf("Sending packet %d\n", packetCount);

        char payloadtype = 0;
        status = rtp_session.SendPacket(sendBuffer, sizeof(sendBuffer), payloadtype, false, 10);
        checkerror(status);

        jrtplib::RTPTime::Wait(jrtplib::RTPTime(1, 0));
    }

    rtp_session.BYEDestroy(jrtplib::RTPTime(10, 0), 0, 0);

    return 0;
}

