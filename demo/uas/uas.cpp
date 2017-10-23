/*
===============================================================
GBT28181 SIP组件libGBT28181SipComponent.so注册实现
作者：程序人生
博客地址：http://blog.csdn.net/hiwubihe
QQ：1269122125
注：请尊重原作者劳动成果，仅供学习使用，请勿盗用，违者必究！
================================================================
*/

#include <iostream>
#include <string>
#include <sstream>
#include <assert.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <osipparser2/osip_port.h>

#include <eXosip2/eXosip.h>
#include <eXosip2/eX_setup.h>
#include <eXosip2/eX_register.h>
#include <eXosip2/eX_options.h>
#include <eXosip2/eX_message.h>
#include <sys/types.h>
//#include <arpa/inet.h>
//#include <sys/socket.h>
#include <WinSock2.h>
#include <Windows.h>


const char* LISTEN_ADDR = "192.168.6.80";
const int UASPORT = 5060;

#define snprintf _snprintf

//该系数是由UAS维护的，UAS在接收到UAC的未鉴权报文后，给UAC回复401，在该报文中必须要带相关认证系数和认证方法 （UAS赋值的认证随机数）
const char* NONCE = "9bd055";
//UAS默认加密算法
const char* ALGORITHTHM = "MD5";

struct eXosip_t *excontext = NULL;
//SIP From头部
class CSipFromHeader
{
public:
    void SetHeader(std::string addrCod, std::string addrI, std::string addrPor) {
        addrCode = addrCod;
        addrIp = addrI;
        addrPort = addrPor;
    }

    std::string GetFormatHeader() {
        std::stringstream stream;
        stream << "<sip:" << addrCode << "@" << addrIp << ":" << addrPort << ">";
        return stream.str();
    }
    //主机名称
    std::string GetRealName() {
        std::stringstream stream;
        stream << addrIp;
        return stream.str();
    }
private:
    std::string addrCode;
    std::string addrIp;
    std::string addrPort;
};

//SIP Contract头部
class CContractHeader : public CSipFromHeader
{
public:
    void SetContractHeader(std::string addrCod, std::string addrI, std::string addrPor, int expire) {
        SetHeader(addrCod, addrI, addrPor);
        expires = expire;
    }
    std::string GetContractFormatHeader(bool bExpires) {
        if (!bExpires) {
            return GetFormatHeader();
        }
        else {
            std::string sTmp = GetFormatHeader();
            std::stringstream stream;
            stream << ";" << "expires=" << expires;
            sTmp += stream.str();
            return sTmp;
        }

    }
private:
    int expires;
};

struct SipContextInfo
{
    //Sip层返回的请求的标志 响应时返回即可
    int sipRequestId;
    //维护一次注册
    std::string callId;
    //消息所属的功能方法名字符串
    std::string method;
    //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
    CSipFromHeader from;
    //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
    CSipFromHeader proxy;
    //地址编码@域名或IP地址:连接端口，例如sip:1111@127.0.0.1:5060
    CContractHeader contact;
    //消息内容,一般为DDCP消息体XML文档,或者具体协议帧要求的其他字符串文本
    std::string content;
    //响应状态信息
    std::string status;
    //超时,时间单位为秒
    int expires;
};

struct SipAuthInfo
{
    //平台主机名
    std::string digestRealm;
    //平台提供的随机数
    std::string nonce;
    //用户名
    std::string userName;
    //密码
    std::string response;
    //“sip:平台地址”,不需要uac赋值
    std::string uri;
    //加密算法MD5
    std::string algorithm;
};

struct sipRegisterInfo
{
    SipContextInfo baseInfo;
    SipAuthInfo authInfo;
    bool isAuthNull;
};

void OnRegister(eXosip_event_t *osipEvent);
void Register_SendAnswer(sipRegisterInfo&info);
void printRegisterPkt(sipRegisterInfo&info);
void parserRegisterInfo(osip_message_t*request, int iReqId, sipRegisterInfo &regInfo);

void __stdcall MyCbSipCallback(osip_message_t * msg, int received)
{
    char* dest;
    size_t length;
    osip_message_to_str(msg, &dest, &length);

    printf("\n\t\t\t\t\t <#######[%s] sip_method: %s, status_code: %d  %s \n%s\n#######>\n",
        (received == 1) ? "recv" : "send", msg->sip_method, msg->status_code, msg->reason_phrase,
        dest);
}

int main()
{
    int result = OSIP_SUCCESS;
    int call_id, dialog_id;
    // 申请
    excontext = eXosip_malloc();

    // 初始化
    result = eXosip_init(excontext);
    assert(OSIP_SUCCESS == result);

    // 设置SIP消息回调(发送+接收)
    result = eXosip_set_cbsip_message(excontext, MyCbSipCallback);
    assert(OSIP_SUCCESS == result);

    eXosip_set_user_agent(excontext, NULL);

    // 监听
    result = eXosip_listen_addr(excontext, IPPROTO_UDP, NULL, UASPORT, AF_INET, 0);
    assert(OSIP_SUCCESS == result);
    printf("listen on port %d success\n", UASPORT);

    result = eXosip_set_option(excontext, EXOSIP_OPT_SET_IPV4_FOR_GATEWAY, LISTEN_ADDR);
    assert(OSIP_SUCCESS == result);

    //开启循环消息，实际应用中可以开启多线程同时接收信号
    while (true)
    {
        // 等待sip事件
        eXosip_event_t* je = ::eXosip_event_wait(excontext, 0, 200);// 0的单位是秒，200是毫秒  If get nothing osip event,then continue the loop.
        if (NULL == je) {
            continue;
        }

        {
            eXosip_lock(excontext);
            eXosip_default_action(excontext, je);
            eXosip_unlock(excontext);
        }


        // 事件处理
        switch (je->type)
        {
        case EXOSIP_MESSAGE_NEW: // 新的消息到来
        {
            if (MSG_IS_REGISTER(je->request)) {
                OnRegister(je);
            }
            else if (MSG_IS_MESSAGE(je->request))
            {
                {
                    osip_body_t *body = NULL;
                    osip_message_get_body(je->request, 0, &body);
                    std::cout << "I get the msg is: " << body->body << std::endl;
                }

                // 按照规则，需要回复 OK 信息
                osip_message_t* answer = NULL;
                eXosip_message_build_answer(excontext, je->tid, 200, &answer);
                eXosip_message_send_answer(excontext, je->tid, 200, answer);
            }
        } break;
        case EXOSIP_CALL_INVITE: // INVITE 请求消息
        {
            // 得到接收到消息的具体信息
            std::cout << "\n\tReceived a INVITE msg from " << je->request->req_uri->host
                << " : " << je->request->req_uri->port
                << ", username is " << je->request->req_uri->username << std::endl;

            // 得到消息体,认为该消息就是 SDP 格式.
            sdp_message_t* remote_sdp = eXosip_get_remote_sdp(excontext, je->did);
            call_id = je->cid;
            dialog_id = je->did;

            eXosip_lock(excontext);

            osip_message_t* answer;
            eXosip_call_send_answer(excontext, je->tid, 180, NULL);
            int result = eXosip_call_build_answer(excontext, je->tid, 200, &answer);
            if (result != OSIP_SUCCESS)
            {
                std::cout << "\n\t--> This request msg is invalid! Cann't response!\n" << std::endl;
                eXosip_call_send_answer(excontext, je->tid, 400, NULL);
            }
            else
            {
                char tmp[4096] = { 0 };
                snprintf(tmp, 4096,
                    "v=0\r\n"
                    "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"
                    "t=1 10\r\n"
                    "a=username:rainfish\r\n"
                    "a=password:123\r\n");

                // 设置回复的SDP消息体,下一步计划分析消息体
                // 没有分析消息体，直接回复原来的消息，这一块做的不好。
                osip_message_set_body(answer, tmp, strlen(tmp));
                osip_message_set_content_type(answer, "application/sdp");

                eXosip_call_send_answer(excontext, je->tid, 200, answer);
                std::cout << "\n\t--> send 200 over!" << std::endl;
            }

            eXosip_unlock(excontext);

            // 显示出在 sdp 消息体中的 attribute 的内容,里面计划存放我们的信息
            std::cout << "\n\t--> The INFO is :\n";
            int pos = 0;
            while (!osip_list_eol(&(remote_sdp->a_attributes), pos))
            {
                sdp_attribute_t *at;

                //这里解释了为什么在SDP消息体中属性a里面存放必须是两列
                at = (sdp_attribute_t *)osip_list_get(&remote_sdp->a_attributes, pos);
                std::cout << "\n\t" << at->a_att_field
                    << " : " << at->a_att_value << std::endl;

                pos++;
            }
            break;

        } break;
        case EXOSIP_CALL_ACK:
            std::cout << "\n\t--> ACK recieved!\n" << std::endl;
            // printf ("the cid is %s, did is %s\n", je->did, je->cid); 
            break;
        case EXOSIP_CALL_CLOSED:
        {
            std::cout << "\n\t--> the remote hold the session!\n" << std::endl;
            // eXosip_call_build_ack(dialog_id, &ack);
            // eXosip_call_send_ack(dialog_id, ack); 
            osip_message_t* answer = NULL;
            int result = eXosip_call_build_answer(excontext, je->tid, 200, &answer);
            if (result != 0)
            {
                printf("This request msg is invalid!Cann't response!\n");
                eXosip_call_send_answer(excontext, je->tid, 400, NULL);
            }
            else
            {
                eXosip_call_send_answer(excontext, je->tid, 200, answer);
                std::cout << "\n\t--> bye send 200 over!\n";
            }
        } break;
        case EXOSIP_CALL_MESSAGE_NEW:
        {
            std::cout << "\n\t*** EXOSIP_CALL_MESSAGE_NEW\n" << std::endl;
            if (MSG_IS_INFO(je->request)) // 如果传输的是 INFO 方法
            {
                eXosip_lock(excontext);
                {
                    osip_message_t* answer = NULL;
                    int result = eXosip_call_build_answer(excontext, je->tid, 200, &answer);
                    if (result == OSIP_SUCCESS) {
                        eXosip_call_send_answer(excontext, je->tid, 200, answer);
                    }
                }
                eXosip_unlock(excontext);

                {
                    osip_body_t *body;
                    osip_message_get_body(je->request, 0, &body);
                    std::cout << "the body is " << body->body << std::endl;
                }
            }
        } break;

        default:
        {
            std::cout << "The sip event type that not be precessed.the event type is : " << je->type << std::endl;
        } break;
        }

        eXosip_event_free(je);
    }

    return 0;
}


// 处理注册消息
void OnRegister(eXosip_event_t *osipEvent)
{
    sipRegisterInfo regInfo;

    parserRegisterInfo(osipEvent->request, osipEvent->tid, regInfo);

    //printRegisterPkt(regInfo); //打印报文

    //发送应答报文
    Register_SendAnswer(regInfo);
}



// 解析sip消息
void parserRegisterInfo(osip_message_t*request, int iReqId, sipRegisterInfo &regInfo)
{
    std::stringstream stream;
    regInfo.baseInfo.method = request->sip_method;
    regInfo.baseInfo.from.SetHeader(request->from->url->username, request->from->url->host, request->from->url->port);
    regInfo.baseInfo.proxy.SetHeader(request->to->url->username, request->to->url->host, request->to->url->port);

    //获取expires
    osip_header_t* header = NULL;
    {
        osip_message_header_get_byname(request, "expires", 0, &header);
        if (NULL != header && NULL != header->hvalue) {
            regInfo.baseInfo.expires = atoi(header->hvalue);
        }
    }

    //contact字段
    osip_contact_t* contact = NULL;
    osip_message_get_contact(request, 0, &contact);
    if (NULL != contact) {
        regInfo.baseInfo.contact.SetContractHeader(contact->url->username, contact->url->host, contact->url->port, regInfo.baseInfo.expires);
    }

    //注册返回 由发送方维护的请求ID 接收方接收后原样返回即可
    regInfo.baseInfo.sipRequestId = iReqId;
    //CALL_ID
    {
        stream.str("");
        stream << request->call_id->number;
        regInfo.baseInfo.callId = stream.str();
    }

    //解析content消息
    osip_body_t * body = NULL;
    osip_message_get_body(request, 0, &body);
    if (body != NULL)
    {
        stream.str("");
        stream << body->body;
        regInfo.baseInfo.content = stream.str();
    }

    //鉴权信息
    osip_authorization_t* authentication = NULL;
    {
        osip_message_get_authorization(request, 0, &authentication);
        if (NULL == authentication) {
            regInfo.isAuthNull = true;
        }
        else
        {
            regInfo.isAuthNull = false;
            stream.str("");
            stream << authentication->username;
            regInfo.authInfo.userName = stream.str();
            stream.str("");
            stream << authentication->algorithm;
            regInfo.authInfo.algorithm = stream.str();
            stream.str("");
            stream << authentication->realm;
            regInfo.authInfo.digestRealm = stream.str();
            stream.str("");
            stream << authentication->nonce;
            regInfo.authInfo.nonce = stream.str();
            stream.str("");
            stream << authentication->response;
            regInfo.authInfo.response = stream.str();
            stream.str("");
            stream << authentication->uri;
            regInfo.authInfo.uri = stream.str();
        }
    }
    authentication = NULL;
}

//发送应答报文
void Register_SendAnswer(sipRegisterInfo& info)
{
    osip_message_t* answer = NULL;
    int iStatus;
    if (info.isAuthNull) {
        iStatus = 401;
    }
    else {
        iStatus = 200;
    }

    eXosip_lock(excontext);
    {
        int result = ::eXosip_message_build_answer(excontext, info.baseInfo.sipRequestId, iStatus, &answer);
        if (iStatus == 401)
        {
            //由SIP库生成认证方法和认证参数发送客户端
            std::stringstream stream;
            std::string nonce = NONCE;
            std::string algorithm = ALGORITHTHM;
            stream << "Digest realm=\"" << info.baseInfo.from.GetRealName()
                << "\",nonce=\"" << nonce
                << "\",algorithm=" << algorithm;

            osip_message_set_header(answer, "WWW-Authenticate", stream.str().c_str());
            osip_message_set_header(answer, "MyHead", "this is my head 不要包含逗号 查看WWW-Authenticate");
            osip_message_set_header(answer, "MyHead", stream.str().c_str());
            std::cout << "发送401报文" << std::endl;
        }
        else if (iStatus == 200)
        {
            osip_message_set_header(answer, "Contact", info.baseInfo.contact.GetContractFormatHeader(true).c_str());
            std::cout << "发送200报文" << std::endl;
            // std::string_t b = "<sip: 100110000101000000@192.168.31.18:5061>;expires=600";
            //osip_message_set_header(answer, "Contact", b.c_str());
        }
        else
        {
            //Do nothing
        }

        if (OSIP_SUCCESS != result) {
            ::eXosip_message_send_answer(excontext, info.baseInfo.sipRequestId, 400, NULL);
        }
        else {
            //发送消息体
            ::eXosip_message_send_answer(excontext, info.baseInfo.sipRequestId, iStatus, answer);
        }

        if (0 == info.baseInfo.expires) {
            eXosip_register_remove(excontext, info.baseInfo.sipRequestId);
        }
    }
    eXosip_unlock(excontext);
}


//打印接收到的响应报文
void printRegisterPkt(sipRegisterInfo& info)
{
    std::cout << "接收到报文：" << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "method:" << info.baseInfo.method << std::endl;
    std::cout << "from:    " << info.baseInfo.from.GetFormatHeader() << std::endl;
    std::cout << "to:" << info.baseInfo.proxy.GetFormatHeader() << std::endl;
    std::cout << "contact:" << info.baseInfo.contact.GetContractFormatHeader(false) << std::endl;

    //注册返回 由发送方维护的请求ID 接收方接收后原样返回即可
    std::cout << "sipRequestId:" << info.baseInfo.sipRequestId << std::endl;
    //CALL_ID
    std::cout << "CallId:" << info.baseInfo.callId << std::endl;
    //解析content消息
    std::cout << "body:" << info.baseInfo.content << std::endl;
    //获取expires
    std::cout << "expires:" << info.baseInfo.expires << std::endl;
    //鉴权信息
    if (info.isAuthNull) {
        std::cout << "当前报文未提供鉴权信息!!!" << std::endl;
    }
    else {
        std::cout << "当前报文鉴权信息如下:" << std::endl;
        std::cout << "username:" << info.authInfo.userName << std::endl;
        std::cout << "algorithm:" << info.authInfo.algorithm << std::endl;
        std::cout << "Realm:" << info.authInfo.digestRealm << std::endl;
        std::cout << "nonce:" << info.authInfo.nonce << std::endl;
        std::cout << "response:" << info.authInfo.response << std::endl;
        std::cout << "uri:" << info.authInfo.uri << std::endl;
    }
    std::cout << "================================================================" << std::endl;
    return;
}
