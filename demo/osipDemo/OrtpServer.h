//////////////////////////////////////////////////////////////////////////  
//  COPYRIGHT NOTICE  
//  Copyright (c) 2011, 华中科技大学 卢俊（版权声明）  
//  All rights reserved.  
//   
/// @file    CortpServer.h  
/// @brief   ortp服务器类声明文件  
///  
/// 实现和提供ortp的服务器端应用接口  
///  
/// @version 1.0     
/// @author  卢俊  
/// @date    2011/11/03  
//  
//  
//  修订说明：  
//////////////////////////////////////////////////////////////////////////  

#ifndef CORTPSERVER_H_  
#define CORTPSERVER_H_  

#include <ortp/ortp.h>  

/**
*  COrtpServer RTP发送类
*
*  负责使用RTP协议进行数据的发送
*/
class COrtpServer
{
public:

    /**  构造函数
    *
    *  该函数为该类的构造函数，在创建该类对象时自动调用
    */
    COrtpServer();

    /** 析构函数
    *
    * 该函数执行析构操作，由系统自动调用
    */
    ~COrtpServer();

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
    *  @param:  const char * destIP 目的地址的IP
    *  @param:  int dest_rtp_port 目的地址的rtp监听端口号
    *  @param:  int dest_rtcp_port 目的地址的rtcp监听端口号
    *  @return: bool  是否成功
    *  @note:
    *  @see:
    */
    bool create(const char * destIP, int dest_rtp_port, int dest_rtcp_port);

    /** 发送RTP数据
    *
    *  将指定的buffer中的数据发送到客户端
    *  @param:  unsigned char * buffer 需要发送的数据
    *  @param:  int len 有效字节数
    *  @return: int 实际发送的字节数
    *  @note:
    *  @see:
    */
    int send_data(unsigned char *buffer, int len);

private:

    RtpSession *m_pSession;     /** rtp会话句柄 */

    long        m_curTimeStamp; /** 当前时间戳 */
    int         m_timeStampInc; /** 时间戳增量 */

    char       *m_ssrc;         /** 数据源标识 */
};

#endif // COrtpServer_H_  
