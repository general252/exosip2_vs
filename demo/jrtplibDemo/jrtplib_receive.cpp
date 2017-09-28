/*=============================================================================
*     FileName: jrtplib_receive.cpp
*         Desc: receive packet and print out the payloaddata
*       Author: licaibiao
*   LastChange: 2017-04-10
* =============================================================================*/
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

#define checkerror(rtperr) { if (rtperr < 0) { std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl; exit(-1); }}

int main_recv(void)
{
    RTPSession sess;
    uint16_t portbase = 6664;
    int status;
    bool done = false;

    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;
    sessparams.SetOwnTimestampUnit(1.0 / 10.0);
    sessparams.SetAcceptOwnPackets(true);

    transparams.SetPortbase(portbase);
    status = sess.Create(sessparams, &transparams);
    checkerror(status);

    sess.BeginDataAccess();
    RTPTime delay(0.020);
    RTPTime starttime = RTPTime::CurrentTime();

    while (!done)
    {
        status = sess.Poll();
        checkerror(status);

        if (sess.GotoFirstSourceWithData())
        {
            do
            {
                RTPPacket *pack;

                while ((pack = sess.GetNextPacket()) != NULL)
                {
                    std::cout << pack->GetPayloadData() << std::endl;
                    sess.DeletePacket(pack);
                }
            } while (sess.GotoNextSourceWithData());
        }

        RTPTime::Wait(delay);
        RTPTime t = RTPTime::CurrentTime();
        t -= starttime;
        if (t > RTPTime(60.0))
            done = true;
    }

    sess.EndDataAccess();
    delay = RTPTime(10.0);
    sess.BYEDestroy(delay, 0, 0);

    return 0;
}
