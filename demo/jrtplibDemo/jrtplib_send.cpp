/*=============================================================================
*     FileName: jrtplib_send.cpp
*         Desc: sending packets to  destination port
*       Author: licaibiao
*   LastChange: 2017-04-10
* =============================================================================*/
#include <jrtplib3/rtpsession.h>
#include <jrtplib3/rtpudpv4transmitter.h>
#include <jrtplib3/rtpipv4address.h>
#include <jrtplib3/rtpsessionparams.h>
#include <jrtplib3/rtperrors.h>
#include <jrtplib3/rtplibraryversion.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace jrtplib;

#define checkerror(rtperr) { if (rtperr < 0) { std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl; exit(-1); }}

int main_send(void)
{
    int i;
    int num;
    int status;

    RTPSession sess;
    uint16_t portbase = 6666;
    uint16_t destport = 6664;

#if 0
    uint32_t destip;
    destip = inet_addr("192.168.0.6");
    if (destip == INADDR_NONE)
    {
        std::cerr << "Bad IP address specified" << std::endl;
        return -1;
    }
    destip = ntohl(destip);
#else
    uint8_t destip[] = { 192, 168, 6, 80 };
#endif 

    //std::cout << "Number of packets you wish to be sent:" << std::endl;
    //std::cin >> num;
    num = 1000;

    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;

    sessparams.SetOwnTimestampUnit(1.0 / 10.0);

    sessparams.SetAcceptOwnPackets(true);
    transparams.SetPortbase(portbase);
    status = sess.Create(sessparams, &transparams);
    checkerror(status);

    RTPIPv4Address addr(destip, destport);

    status = sess.AddDestination(addr);
    checkerror(status);

    for (i = 1; i <= num; i++)
    {
        printf("\nSending packet %d/%d\n", i, num);

        status = sess.SendPacket((void *)"1234567890", 10, 0, false, 10);
        checkerror(status);
        RTPTime::Wait(RTPTime(1, 0));
    }

    sess.BYEDestroy(RTPTime(10, 0), 0, 0);

    return 0;
}

