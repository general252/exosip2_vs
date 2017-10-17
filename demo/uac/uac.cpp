/*
===============================================================
GBT28181 基于eXosip2,osip库实现注册UAC功能
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
#include <process.h>

//本地监听IP
const char* LISTEN_ADDR = "192.168.6.80";
//本地监听端口
const char* UACPORT = "5061";
//本UAC地址编码
const char* UACCODE = "100110000201000000";
//本地UAC密码
const char* UACPWD = "12345";

//远程UAS IP
const char* UAS_ADDR = "192.168.6.80";
//远程UAS 端口
const char* UAS_PORT = "5060";
//超时
const int EXPIS = 300;

char *source_call = "sip:300@192.168.6.80";
char *dest_call = "sip:200@192.168.6.80:5060";

struct eXosip_t *excontext = NULL;
//当前服务状态 1 已经注册 0 未注册
static int iCurrentStatus;
//注册成功HANDLE, 这个挺重要
static int iHandle = -1;

int call_id, dialog_id;


#define snprintf _snprintf

//SIP From/To 头部
class CSipFromToHeader
{
public:
    void SetHeader(std::string addrCod, std::string addrI, std::string addrPor) {
        addrCode = addrCod;
        addrIp = addrI;
        addrPort = addrPor;
    }
    std::string GetFormatHeader() {
        std::stringstream stream;
        stream << "sip:" << addrCode << "@" << addrIp << ":" << addrPort;
        return stream.str();
    }
    //主机名称
    std::string GetCode() {
        std::stringstream stream;
        stream << addrCode;
        return stream.str();
    }
    //主机地址
    std::string GetAddr() {
        std::stringstream stream;
        stream << addrIp;
        return stream.str();
    }
    //端口
    std::string GetPort() {
        std::stringstream stream;
        stream << addrPort;
        return stream.str();
    }

private:
    std::string addrCode;
    std::string addrIp;
    std::string addrPort;
};

//SIP Contract头部
class CContractHeader : public CSipFromToHeader
{
public:
    void SetContractHeader(std::string addrCod, std::string addrI, std::string addrPor) {
        SetHeader(addrCod, addrI, addrPor);
    }
    std::string GetContractFormatHeader() {
        std::stringstream stream;
        stream << "<sip:" << GetCode() << "@" << GetAddr() << ":" << GetPort() << ">";
        return stream.str();
    }
};


unsigned __stdcall serverHandle(void *pUser);
void help();
void UnRegister();
void RefreshRegister();
void Register();
int SendRegister(int& registerId, CSipFromToHeader &from, CSipFromToHeader &to,
    CContractHeader &contact, const std::string& userName, const std::string& pwd,
    const int expires, int iType);
void Invate_Call();


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
    iCurrentStatus = 0;
    //库处理结果
    int result = OSIP_SUCCESS;

    //初始化库
    excontext = eXosip_malloc();

    result = eXosip_init(excontext);
    assert(OSIP_SUCCESS == result);
    std::cout << "eXosip_init success." << std::endl;

    eXosip_set_cbsip_message(excontext, MyCbSipCallback);
    eXosip_set_user_agent(excontext, NULL);

    //监听
    result = eXosip_listen_addr(excontext, IPPROTO_UDP, NULL, atoi(UACPORT), AF_INET, 0);
    assert(OSIP_SUCCESS == result);

    //设置监听网卡
    result = eXosip_set_option(excontext, EXOSIP_OPT_SET_IPV4_FOR_GATEWAY, LISTEN_ADDR);
    assert(OSIP_SUCCESS == result);

    //开启服务线程
    _beginthreadex(NULL, 0, serverHandle, NULL, 0, 0);

    //开启事件循环
    while (true)
    {
        //等待事件 0的单位是秒，500是毫秒
        eXosip_event_t* je = ::eXosip_event_wait(excontext, 0, 200);
        //事件空继续等待
        if (NULL == je) {
            continue;
        }

        //eXosip库默认处理
        {
            //一般处理401/407采用库默认处理
            eXosip_lock(excontext);
            eXosip_default_action(excontext, je);
            eXosip_unlock(excontext);
        }


        // 处理事件
        {
            switch (je->type)
            {
                //需要继续验证REGISTER是什么类型
            case EXOSIP_REGISTRATION_SUCCESS:
            case EXOSIP_REGISTRATION_FAILURE:
            {
                std::cout << "收到状态码:" << je->response->status_code << "报文" << std::endl;
                if (je->response->status_code == 401) {
                    std::cout << "发送鉴权报文" << std::endl;
                }
                else if (je->response->status_code == 200) {
                    std::cout << "接收成功" << std::endl;
                }
                else {
                }
            } break;
                // INVITE
            case EXOSIP_CALL_INVITE:
                printf("a new invite reveived!\n");
                break;
            case EXOSIP_CALL_PROCEEDING:
                printf("proceeding!\n");
                break;
            case EXOSIP_CALL_RINGING:
            {
                printf("ringing!\n");
                printf("\ncall_id is %d, dialog_id is %d \n", je->cid, je->did);
            } break;
            case EXOSIP_CALL_ANSWERED:
            {
                printf("ok! connected!\n");
                call_id = je->cid;
                dialog_id = je->did;
                printf("call_id is %d, dialog_id is %d \n", je->cid, je->did);

                osip_message_t *ack = NULL;
                eXosip_call_build_ack(excontext, je->did, &ack);
                eXosip_call_send_ack(excontext, je->did, ack);
            } break;
            case EXOSIP_CALL_CLOSED:
                printf("the other sid closed!\n");
                break;
            case EXOSIP_CALL_ACK:
                printf("ACK received!\n");
                break;
            default:
                std::cout << "The sip event type that not be precessed.the event " "type is : " << je->type << std::endl;
                break;
            }
        }

        eXosip_event_free(je);
    }
}

//服务处理线程
unsigned __stdcall serverHandle(void *pUser)
{
    while (true)
    {
        help();
        char ch[256] = { 0 };
        scanf("%s", ch);

        switch (ch[0])
        {
        case '0':
        {
            Register(); //注册
        } break;
        case '1':
        {
            RefreshRegister(); //刷新注册
        } break;
        case '2':
        {
            UnRegister(); //注销
        } break;
        case  '3':
        {
            Invate_Call();
        } break;
        case 's':
        {
            // 传输 INFO 方法
            osip_message_t* info = NULL;;
            eXosip_call_build_info(excontext, dialog_id, &info);

            char tmp[4096] = { 0 };
            snprintf(tmp, 4096, "hello, bluesea");
            osip_message_set_body(info, tmp, strlen(tmp));

            // 格式可以任意设定, text/plain 代表文本信息
            osip_message_set_content_type(info, "text/plain");
            eXosip_call_send_request(excontext, dialog_id, info);
            break;
        } break;
        case 'm':
        {
            // 传输 MESSAGE方法,也就是即时消息，
            // 和 INFO 方法相比，主要区别，是 MESSAGE 不用建立连接，直接传输信息，
            // 而 INFO 必须在建立 INVITE 的基础上传输。
            std::cout << "\n\t--> the mothed :MESSAGE \n" << std::endl;
            osip_message_t* message = NULL;
            eXosip_message_build_request(excontext, &message,
                "MESSAGE",
                dest_call,
                source_call,
                NULL);
            char* strMsg = "message: hello bluesea!";
            osip_message_set_body(message, strMsg, strlen(strMsg));

            // 假设格式是xml
            osip_message_set_content_type(message, "text/xml");
            eXosip_message_send_request(excontext, message);
            break;
        }break;
        case '4':
        {
            if (system("clear") < 0) {
                std::cout << "clear scream error" << std::endl;
                exit(1);
            }
        } break;
        case '5':
        {
            std::cout << "exit sipserver......" << std::endl;
            system("pause");
            exit(0);
        } break;
        default:
        {
            std::cout << "select error" << std::endl;
        } break;
        }

        Sleep(5000);
    }
    return NULL;
}






//发送注册信息
int SendRegister(int& registerId, CSipFromToHeader &from, CSipFromToHeader &to,
    CContractHeader &contact, const std::string& userName, const std::string& pwd,
    const int expires, int iType)
{
    std::cout << "=============================================" << std::endl;
    if (iType == 0) {
        std::cout << "注册请求信息：" << std::endl;
    }
    else if (iType == 1) {
        std::cout << "刷新注册信息：" << std::endl;
    }
    else {
        std::cout << "注销信息:" << std::endl;
    }

    std::cout << "registerId " << registerId << std::endl;
    std::cout << "from " << from.GetFormatHeader() << std::endl;
    std::cout << "to " << to.GetFormatHeader() << std::endl;
    std::cout << "contact " << contact.GetContractFormatHeader() << std::endl;
    std::cout << "username " << userName << std::endl;
    std::cout << "pwd " << pwd << std::endl;
    std::cout << "expires " << expires << std::endl;
    std::cout << "=============================================" << std::endl;

    //服务器注册
    static osip_message_t *regMsg = 0;
    int ret;

    ::eXosip_add_authentication_info(excontext, userName.c_str(), userName.c_str(), pwd.c_str(), "MD5", NULL);
    eXosip_lock(excontext);
    {
        //发送注册信息 401响应由eXosip2库自动发送
        if (0 == registerId)
        {
            // 注册消息的初始化
            registerId = ::eXosip_register_build_initial_register(excontext,
                from.GetFormatHeader().c_str(),
                to.GetFormatHeader().c_str(),
                contact.GetContractFormatHeader().c_str(),
                expires,
                &regMsg);
            if (registerId <= 0) {
                return -1;
            }
        }
        else
        {
            // 构建注册消息
            ret = ::eXosip_register_build_register(excontext, registerId, expires, &regMsg);
            if (ret != OSIP_SUCCESS) {
                return ret;
            }

            //添加注销原因
            if (expires == 0)
            {
                osip_contact_t *contact = NULL;
                char tmp[128];

                osip_message_get_contact(regMsg, 0, &contact);
                {
                    sprintf(tmp, "<sip:%s@%s:%s>;expires=0", contact->url->username, contact->url->host, contact->url->port);
                }
                //osip_contact_free(contact);
                //reset contact header
                osip_list_remove(&regMsg->contacts, 0);
                osip_message_set_contact(regMsg, tmp);
                osip_message_set_header(regMsg, "Logout-Reason", "logout");
            }
        }

        // 发送注册消息
        ret = ::eXosip_register_send_register(excontext, registerId, regMsg);
        if (ret != OSIP_SUCCESS) {
            registerId = 0;
        }
    }
    eXosip_unlock(excontext);

    return ret;
}

//注册
void Register()
{
    if (iCurrentStatus == 1) {
        std::cout << "当前已经注册" << std::endl;
        return;
    }

    CSipFromToHeader stFrom;
    stFrom.SetHeader(UACCODE, UAS_ADDR, UAS_PORT);
    CSipFromToHeader stTo;
    stTo.SetHeader(UACCODE, UAS_ADDR, UAS_PORT);
    CContractHeader stContract;
    stContract.SetContractHeader(UACCODE, LISTEN_ADDR, UACPORT);

    //发送注册信息
    int registerId = 0;
    if (0 > SendRegister(registerId, stFrom, stTo, stContract, UACCODE, UACPWD, 3000, 0)) {
        std::cout << "发送注册失败" << std::endl;
        return;
    }
    iCurrentStatus = 1;
    iHandle = registerId;
}
//刷新注册
void RefreshRegister()
{
    if (iCurrentStatus == 0) {
        std::cout << "当前未注册，不允许刷新" << std::endl;
        return;
    }

    CSipFromToHeader stFrom;
    stFrom.SetHeader(UACCODE, UAS_ADDR, UAS_PORT);
    CSipFromToHeader stTo;
    stTo.SetHeader(UACCODE, UAS_ADDR, UAS_PORT);
    CContractHeader stContract;
    stContract.SetContractHeader(UACCODE, LISTEN_ADDR, UACPORT);
    //发送注册信息
    if (0 > SendRegister(iHandle, stFrom, stTo, stContract, UACCODE, UACPWD, 3000, 1))
    {
        std::cout << "发送刷新注册失败" << std::endl;
        return;
    }
}

//注销
void UnRegister()
{
    if (iCurrentStatus == 0) {
        std::cout << "当前未注册，不允许注销" << std::endl;
        return;
    }

    CSipFromToHeader stFrom;
    stFrom.SetHeader(UACCODE, UAS_ADDR, UAS_PORT);
    CSipFromToHeader stTo;
    stTo.SetHeader(UACCODE, UAS_ADDR, UAS_PORT);
    CContractHeader stContract;
    stContract.SetContractHeader(UACCODE, LISTEN_ADDR, UACPORT);
    //发送注册信息
    if (0 > SendRegister(iHandle, stFrom, stTo, stContract, UACCODE, UACPWD, 0, 2)) {
        std::cout << "发送注销失败" << std::endl;
        return;
    }

    iCurrentStatus = 0;
    iHandle = -1;
}

// 发起请求
void Invate_Call()
{
    osip_message_t *invite = NULL;
    int result = eXosip_call_build_initial_invite(excontext, &invite, dest_call, source_call, NULL, "This si a call for a conversation");
    if (result != OSIP_SUCCESS) {
        printf("Intial INVITE failed!\n");
        return;
    }

    char tmp[4096];
    snprintf(tmp, 4096,
        "v=0\r\n"
        "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"
        "t=1 10\r\n"
        "a=username:300\r\n"
        "a=password:300\r\n");
    osip_message_set_body(invite, tmp, strlen(tmp));
    osip_message_set_content_type(invite, "application/sdp");

    eXosip_lock(excontext);
    result = eXosip_call_send_initial_invite(excontext, invite);
    eXosip_unlock(excontext);
}


void help()
{
    const char *b =
        "\n\n\n-------------------------------------------------------------------------------\n"
        "SIP UAC端 注册,刷新注册,注销实现\n\n"
        "-------------------------------------------------------------------------------\n"
        "\n"
        "              0:Register\n"
        "              1:RefreshRegister\n"
        "              2:UnRegister\n"
        "              3:invate call\n"
        "              s:INFO  method\n"
        "              m:MESSAGE method\n"
        "              4:clear scream\n"
        "              5:exit\n"
        "-------------------------------------------------------------------------------\n"
        "\n";
    fprintf(stderr, b, strlen(b));
    std::cout << "please select method :";
}