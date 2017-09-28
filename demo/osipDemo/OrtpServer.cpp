
//////////////////////////////////////////////////////////////////////////  
//  COPYRIGHT NOTICE  
//  Copyright (c) 2011, 华中科技大学 卢俊（版权声明）  
//  All rights reserved.  
//   
/// @file    CortpServer.cpp  
/// @brief   ortp服务器类实现文件  
///  
/// 实现和提供ortp的服务器端应用接口  
///  
/// @version 1.0     
/// @author  lujun   
/// @date    2011/11/03  
//  
//  
//  修订说明：  
//////////////////////////////////////////////////////////////////////////  

#include "OrtpServer.h"
#include <assert.h>

/* the payload type define */
#define PAYLOAD_TYPE_VIDEO 34  

/* RTP video Send time stamp increase */
#define VIDEO_TIME_STAMP_INC  3600  

COrtpServer::COrtpServer()
{
    m_ssrc = NULL;
    m_pSession = NULL;
    m_timeStampInc = 0;
    m_curTimeStamp = 0;
}

COrtpServer::~COrtpServer()
{
    if (!m_pSession)
    {
        rtp_session_destroy(m_pSession);
    }
}

bool COrtpServer::init()
{
    WSADATA wsaData;

    /** 初始化winsocket */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }

    ortp_init();
    ortp_scheduler_init();

    return true;
}

bool COrtpServer::deInit()
{
    ortp_exit();

    if (WSACleanup() == SOCKET_ERROR) {
        return false;
    }

    return true;
}

bool COrtpServer::create(const char * destIP, int dest_rtp_port, int dest_rtcp_port)
{
    int rc = 0;

    m_pSession = rtp_session_new(RTP_SESSION_SENDONLY);
    assert(m_pSession != NULL);

    rtp_session_set_scheduling_mode(m_pSession, 1);
    rtp_session_set_blocking_mode(m_pSession, 1);
    rc = rtp_session_set_remote_addr_and_port(m_pSession, destIP, dest_rtp_port, dest_rtcp_port);
    if (rc != 0) {
        printf("rtp_session_set_local_addr: %d\n", GetLastError());
    }

    m_ssrc = getenv("SSRC");
    if (m_ssrc != NULL) {
        rtp_session_set_ssrc(m_pSession, atoi(m_ssrc));
    }

    rtp_session_set_payload_type(m_pSession, PAYLOAD_TYPE_VIDEO);
    m_timeStampInc = VIDEO_TIME_STAMP_INC;

    return true;
}

int COrtpServer::send_data(unsigned char *buffer, int len)
{
    int sendBytes = 0;

    /** 强转 */
    const char *sendBuffer = (const char*)buffer;

    sendBytes = rtp_session_send_with_ts(m_pSession, (uint8_t*)sendBuffer, len, m_curTimeStamp);

    //~if (sendBytes > 0) {
        //m_curTimeStamp += m_timeStampInc; /** 增加时间戳 */
    //}

    return sendBytes;
}