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
#define LISTEN_ADDR ("192.168.6.80")
//本地监听端口
#define UACPORT ("5061")
#define UACPORTINT (5061)
//本UAC地址编码
#define UACCODE ("100110000201000000")
//本地UAC密码
#define UACPWD ("12345")

//远程UAS IP
#define UAS_ADDR ("192.168.6.80")
//远程UAS 端口
#define UAS_PORT ("5060")
//超时
#define EXPIS 300


struct eXosip_t *excontext = NULL;
//当前服务状态 1 已经注册 0 未注册
static int iCurrentStatus;
//注册成功HANDLE
static int iHandle = -1;

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
        stream << "sip: " << addrCode << "@" << addrIp << ":" << addrPort;
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


unsigned __stdcall eventHandle(void *pUser);
unsigned __stdcall serverHandle(void *pUser);
void help();
void UnRegister();
void RefreshRegister();
void Register();
int SendRegister(int& registerId, CSipFromToHeader &from, CSipFromToHeader &to,
    CContractHeader &contact, const std::string& userName, const std::string& pwd,
    const int expires, int iType);


int main()
{
    iCurrentStatus = 0;
    //库处理结果
    int result = OSIP_SUCCESS;
    //初始化库
    excontext = eXosip_malloc();
    if (OSIP_SUCCESS != (result = eXosip_init(excontext))) {
        printf("eXosip_init failure.\n");
        return 1;
    }
    std::cout << "eXosip_init success." << std::endl;
    eXosip_set_user_agent(excontext, NULL);
    //监听
    if (OSIP_SUCCESS != eXosip_listen_addr(excontext, IPPROTO_UDP, NULL, UACPORTINT, AF_INET, 0)) {
        printf("eXosip_listen_addr failure.\n");
        return 1;
    }
    //设置监听网卡
    if (OSIP_SUCCESS != eXosip_set_option(excontext, EXOSIP_OPT_SET_IPV4_FOR_GATEWAY, LISTEN_ADDR)) {
        return -1;
    }

    //开启服务线程
    _beginthreadex(NULL, 0, serverHandle, NULL, 0, 0);

    //事件用于等待
    eXosip_event_t* osipEventPtr = NULL;
    //开启事件循环
    while (true)
    {
        //等待事件 0的单位是秒，500是毫秒
        osipEventPtr = ::eXosip_event_wait(excontext, 0, 200);
        //处理eXosip库默认处理
        {
            Sleep(500);
            eXosip_lock(excontext);
            //一般处理401/407采用库默认处理
            eXosip_default_action(excontext, osipEventPtr);
            eXosip_unlock(excontext);
        }
        //事件空继续等待
        if (NULL == osipEventPtr)
        {
            continue;
        }
        //开启线程处理事件并在事件处理完毕将事件指针释放
        _beginthreadex(NULL, 0, eventHandle, (void*)osipEventPtr, 0, 0);

        osipEventPtr = NULL;
    }
}







//发送注册信息
int SendRegister(int& registerId, CSipFromToHeader &from, CSipFromToHeader &to,
    CContractHeader &contact, const std::string& userName, const std::string& pwd,
    const int expires, int iType)
{
    std::cout << "=============================================" << std::endl;
    if (iType == 0)
    {
        std::cout << "注册请求信息：" << std::endl;
    }
    else if (iType == 1)
    {
        std::cout << "刷新注册信息：" << std::endl;
    }
    else
    {
        std::cout << "注销信息:" << std::endl;
    }
    std::cout << "registerId " << registerId << std::endl;
    std::cout << "from " << from.GetFormatHeader() << std::endl;
    std::cout << "to " << to.GetFormatHeader() << std::endl;
    std::cout << "contact" << contact.GetContractFormatHeader() << std::endl;
    std::cout << "userName" << userName << std::endl;
    std::cout << "pwd" << pwd << std::endl;
    std::cout << "expires" << expires << std::endl;
    std::cout << "=============================================" << std::endl;
    //服务器注册
    static osip_message_t *regMsg = 0;
    int ret;

    ::eXosip_add_authentication_info(excontext, userName.c_str(), userName.c_str(), pwd.c_str(), "MD5", NULL);
    eXosip_lock(excontext);
    //发送注册信息 401响应由eXosip2库自动发送
    if (0 == registerId)
    {
        // 注册消息的初始化
        registerId = ::eXosip_register_build_initial_register(excontext,
            from.GetFormatHeader().c_str(), to.GetFormatHeader().c_str(),
            contact.GetContractFormatHeader().c_str(), expires, &regMsg);
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
    eXosip_unlock(excontext);

    return ret;
}

//注册
void Register()
{
    if (iCurrentStatus == 1)
    {
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
    if (0 > SendRegister(registerId, stFrom, stTo, stContract, UACCODE, UACPWD,
        3000, 0))
    {
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


//服务处理线程
unsigned __stdcall serverHandle(void *pUser)
{
    Sleep(3000);
    while (true)
    {
        help();
        char ch[256] = { 0 };
        scanf("%s", ch);

        switch (ch[0])
        {
        case '0':
            //注册
            Register();
            break;
        case '1':
            //刷新注册
            RefreshRegister();
            break;
        case '2':
            //注销
            UnRegister();
            break;
        case '3':
            if (system("clear") < 0)
            {
                std::cout << "clear scream error" << std::endl;
                exit(1);
            }
            break;
        case '4':
            std::cout << "exit sipserver......" << std::endl;
            system("pause");
            exit(0);
        default:
            std::cout << "select error" << std::endl;
            break;
        }
    }
    return NULL;
}

//事件处理线程
unsigned __stdcall eventHandle(void *pUser)
{
    eXosip_event_t* osipEventPtr = (eXosip_event_t*)pUser;
    switch (osipEventPtr->type)
    {
        //需要继续验证REGISTER是什么类型
    case EXOSIP_REGISTRATION_SUCCESS:
    case EXOSIP_REGISTRATION_FAILURE:
    {
        std::cout << "收到状态码:" << osipEventPtr->response->status_code << "报文" << std::endl;
        if (osipEventPtr->response->status_code == 401)
        {
            std::cout << "发送鉴权报文" << std::endl;
        }
        else if (osipEventPtr->response->status_code == 200)
        {
            std::cout << "接收成功" << std::endl;
        }
        else
        {
        }
    }
    break;
    default:
        std::cout << "The sip event type that not be precessed.the event " "type is : " << osipEventPtr->type << std::endl;
        break;
    }
    eXosip_event_free(osipEventPtr);
    return NULL;
}



void help()
{
    const char *b =
        "-------------------------------------------------------------------------------\n"
        "SIP UAC端 注册,刷新注册,注销实现\n\n"
        "-------------------------------------------------------------------------------\n"
        "\n"
        "              0:Register\n"
        "              1:RefreshRegister\n"
        "              2:UnRegister\n"
        "              3:clear scream\n"
        "              4:exit\n"
        "-------------------------------------------------------------------------------\n"
        "\n";
    fprintf(stderr, b, strlen(b));
    std::cout << "please select method :";
}