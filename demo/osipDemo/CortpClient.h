//////////////////////////////////////////////////////////////////////////  
//  COPYRIGHT NOTICE  
//  Copyright (c) 2011, 华中科技大学 卢俊（版权声明）  
//  All rights reserved.  
//   
/// @file    CortpClient.h    
/// @brief   ortp客户端类声明文件  
///  
/// 实现和提供ortp的客户端应用接口  
///  
/// @version 1.0     
/// @author  卢俊   
/// @date    2011/11/03  
//  
//  
//  修订说明：  
//////////////////////////////////////////////////////////////////////////  

#ifndef CORTPCLIENT_H_  
#define CORTPCLIENT_H_  

#include <ortp/ortp.h>  
#include <string>  

/**
*  COrtpClient ortp客户端管理类
*
*  负责封装和提供ortp相关接口
*/
class COrtpClient
{
public:

    /**  构造函数/析构函数
    *
    *  在创建/销毁该类对象时自动调用
    */
    COrtpClient();
    ~COrtpClient();

    /** ORTP模块的初始化
    *
    *  在整个系统最开始调用，负责ORTP库的初始化
    *  @return: bool  是否成功
    *  @note:
    *  @see:
    */
    static bool init();

    /** ORTP模块的逆初始化
    *
    *  在系统退出前调用，负责ORTP库的释放
    *  @return: bool  是否成功
    *  @note:
    *  @see:
    */
    static bool deInit();

    /** 创建RTP接收会话
    *
    *  负责产生RTP接收端会话，监听服务器端的数据
    *  @param:  const char * localip 本地ip地址
    *  @param:  int local_rtp_port   本地rtp监听端口
    *  @param:  int local_rtcp_port  本地rtcp监听端口
    *  @return: bool  是否成功
    *  @note:
    *  @see:
    */
    bool create(const char * localip, int local_rtp_port, int local_rtcp_port);

    /** 获取接收到的rtp包
    *
    *  将接收到的rtp数据包取出
    *  @param:  char * pBuffer
    *  @param:  int & len
    *  @return: bool  是否成功
    *  @note:
    *  @see:
    */
    int get_recv_data(char *pBuffer, int &len);

private:

    RtpSession *m_pSession;     /** rtp会话句柄 */

    long        m_curTimeStamp; /** 当前时间戳 */
    int         m_timeStampInc; /** 时间戳增量 */

};

#endif // CortpClient_H_  
