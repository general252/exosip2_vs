/*=============================================================================
*     FileName: jrtplib_receive.cpp
*         Desc: receive packet and print out the payloaddata
*       Author: licaibiao
*   LastChange: 2017-04-10
* =============================================================================*/
#include "jrtplib_head.h"


int main_recv(uint16_t rtp_port, void (*OnRecvPacket)(char* buffer, int len))
{
    MyRTPSession rtpsession;
    int status;

    jrtplib::RTPSessionParams sessparams;
    sessparams.SetOwnTimestampUnit(1.0 / 10.0);
    sessparams.SetAcceptOwnPackets(true);

    jrtplib::RTPUDPv4TransmissionParams transparams;
    transparams.SetPortbase(rtp_port);
    status = rtpsession.Create(sessparams, &transparams); // °ó¶¨¶Ë¿Ú(rtp+rtcp)
    checkerror(status);


    jrtplib::RTPTime delay(0.020);
    while (true)
    {
        jrtplib::RTPTime::Wait(delay);

        status = rtpsession.Poll();
        checkerror(status);

        if (false == rtpsession.GotoFirstSourceWithData()) {
            continue;
        }

        do
        {
            jrtplib::RTPPacket *pack = NULL;
            while (true)
            {
                pack = rtpsession.GetNextPacket();
                if (pack != NULL) {
                    OnRecvPacket((char*)pack->GetPayloadData(), pack->GetPayloadLength());

                    rtpsession.DeletePacket(pack);
                }
                else {
                    break;
                }
            }

        } while (rtpsession.GotoNextSourceWithData());
    }


    rtpsession.BYEDestroy(jrtplib::RTPTime(10.0), 0, 0);

    return 0;
}
