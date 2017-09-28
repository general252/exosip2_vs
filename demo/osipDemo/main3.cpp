/** 包含注册, 通话. 测试ms_init失败 */

//用osip2 + eXosip2 + ortp + mediastreamer实现的linux简易软电话源代码（可实现通话）
//http://blog.csdn.net/wanghaobo920/article/details/7993768
//编译命令：
//gcc - losip2 - leXosip2 - lmediastreamer - lpthread - o my_exosip_phone my_exosip_phone.c
//my_exosip_phone.c如下：
#include <eXosip2/eXosip.h>
#include <osip2/osip_mt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include <mediastreamer2/mediastream.h>
#ifdef _MSC_VER
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#endif // _MSC_VER

#define WAIT_TIMER 200
#define REG_TIMER 30*1000

int doing = 1; /* 事件循环开关 */
int rtp_port = 9900; //rtp　媒体本地端口
int dtmfing = 0; 
int calling = 0;/* 是否正在被叫(判断ack类型) */
int picked = 0;
int call_id = 0;
int dialog_id = 0;

char *dtmf_str = "this is dtmf";
//CString dtmf_str; 
//char *server_url="192.168.18.249"; 
char *server_url = "192.168.6.80";
char localip[128] = { 0 };
char *server_port = "5060";
int i_server_port = 0;
char *local_port = "5000";
int i_local_port = 0;
char *username = "200";
char *password = "200";
char *telNum = "300";

struct eXosip_t *excontext = NULL;

AudioStream *audio = NULL;
RtpProfile *profile = NULL;
RtpSession *session = NULL;
OrtpEvQueue *q = NULL;

int my_sip_init(bool isAsServer);
void OnHang();
int sip_ua_monitor();
int real_send_register(int expires);
int build_media(int local_port, const char *remote_ip, int remote_port, int payload, const char *fmtp, int jitter, int ec, int bitrate);

int main3(int argc, char* argv[])
{
    //char *payload_str;
    char tmp[4096];
    char command;
    char dtmf[50] = { 0 };

    int reg_remain = REG_TIMER;
    char out_str[100] = { 0 };

    osip_message_t *invite = NULL;
    osip_message_t *info = NULL;
    osip_message_t *message = NULL;

    char source_call[100] = { 0 };
    char dest_call[100] = { 0 };

    bool isAsServer = false;
    if (argc >= 2) {
        isAsServer = (strchr(argv[1], 's') || strchr(argv[1], 'S'));
    }

    i_server_port = atoi(server_port);
    i_local_port = atoi(local_port);

    if (0 != my_sip_init(isAsServer)) {
        printf("Quit!\n");
        return -1;
    }

    eXosip_guess_localip(excontext, AF_INET, localip, sizeof(localip));

    printf("start as %s : %d\n", isAsServer ? "server" : "client", isAsServer ? i_server_port : i_local_port);
    printf("server: %s:%d    local: %s:%d\n\n\n", server_url, i_server_port, localip, i_local_port);

    //编码 G711uLaw 0


    printf("r 向服务器注册\n\n");
    printf("c 取消注册\n\n");
    printf("i 发起呼叫请求\n\n");
    printf("h 挂断\n\n");
    printf("a 接听电话\n\n");
    printf("q 推出程序\n\n");
    printf("s 执行方法INFO\n\n");
    printf("m 执行方法MESSAGE\n\n");

    int flag = 1;

    while (flag)
    {
        //输入命令 
        printf("Please input the command:\n");
        scanf("%c", &command);
        getchar();
        switch (command)
        {
        case 'r':// 先清除鉴权信息，再应用新输入的鉴权信息 
        {
            real_send_register(1800);
            Sleep(3000);
            real_send_register(1800);
        } break;
        case 'i':
        {
            sprintf(source_call, "sip:%s@%s:%s", username, localip, local_port);
            sprintf(dest_call, "sip:%s@%s:%s", telNum, server_url, server_port);

            //char tmp[4096]; 
            memset(tmp, 0, 4096);

            int i = eXosip_call_build_initial_invite(excontext, &invite, dest_call, source_call, NULL, "This is a call invite");
            if (i != 0)
            {
                printf("Intial INVITE failed!\n");
            }

            snprintf(tmp, 4096,
                "v=0\r\n"
                "o=youtoo 1 1 IN IP4 %s\r\n"
                "s=##youtoo demo\r\n"
                "c=IN IP4 %s\r\n"
                "t=0 0\r\n"
                "m=audio %d RTP/AVP 0 8 101\r\n"
                "a=rtpmap:0 PCMU/8000\r\n"
                "a=rtpmap:8 PCMA/8000\r\n"
                "a=rtpmap:101 telephone-event/8000\r\n"
                "a=fmtp:101 0-15\r\n", localip, localip, rtp_port);

            osip_message_set_body(invite, tmp, strlen(tmp));
            osip_message_set_content_type(invite, "application/sdp");

            eXosip_lock(excontext);
            i = eXosip_call_send_initial_invite(excontext, invite);
            eXosip_unlock(excontext);

        } break;
        case 'h': //挂断 
        {
            OnHang();
        } break;
        case 'c':
        {
            real_send_register(0);
            //printf("This modal is not commpleted!\n"); 
        } break;
        case 's': //传输INFO方法 
        {
            eXosip_call_build_info(excontext, dialog_id, &info);
            snprintf(tmp, 4096, "\nThis is a sip message(Method:INFO)");
            osip_message_set_body(info, tmp, strlen(tmp));
            //格式可以任意设定，text/plain代表文本信息 
            osip_message_set_content_type(info, "text/plain");
            eXosip_call_send_request(excontext, dialog_id, info);
        } break;
        case 'm':
        {
            //传输MESSAGE方法，也就是即时消息，和INFO方法相比，我认为主要区别是： 
            //MESiSAGE不用建立连接，直接传输信息，而INFO消息必须在建立INVITE的基础上传输 
            memset(tmp, 0, 4096);
            printf("the method : MESSAGE\n");
            eXosip_message_build_request(excontext, &message, "MESSAGE", dest_call, source_call, NULL);
            //内容，方法， to ，from ，route 
            snprintf(tmp, 4096, "This is a sip message(Method:MESSAGE)");
            osip_message_set_body(message, tmp, strlen(tmp));
            //假设格式是xml 
            osip_message_set_content_type(message, "text/xml");
            eXosip_message_send_request(excontext, message);
        } break;
        case 'q':
        {
            doing = -1;
            flag = 0;
            Sleep(1000); //保证事件线程能退出(线程循环检测时间毫秒级<1000)  主动关闭媒体连接(本地端) 
            printf("Bye!\n");
        }  break;
        case 'a':
        {
            picked = 1;
            printf("已接听\n");
        } break;
        case 'd':
        {
            dtmfing = 1;
        } break;
        default:
        {
            printf("input error! please input again\n");
        } break;
        }
    }
    return(0);
}

int sip_ua_monitor()
{
    int ret = -1;
    char *payload_str; /* 服务器优先编码值 */
    char tmp[4096];
    char dtmf[50] = { 0 };

    int reg_remain = REG_TIMER;
    Sleep(2000);
    printf("Event monitor for uac/uas start!\n");

    eXosip_event_t *uac_e; /* 事件处理 */
    osip_message_t *ack = NULL; /* 响应消息 */
    osip_message_t *answer = NULL; /* 请求消息的回应 */

                                   /* 响应SDP(用于UAC) */
    sdp_message_t * msg_rsp = NULL;
    sdp_connection_t * con_rsp = NULL;
    sdp_media_t * md_rsp = NULL;

    /* 请求SDP(用于UAS) */
    sdp_message_t * msg_req = NULL;
    sdp_connection_t * con_req = NULL;
    sdp_media_t * md_req = NULL;

    char out_str[100] = { 0 };

    eXosip_lock(excontext);
    eXosip_automatic_action(excontext);
    eXosip_unlock(excontext);

    while (doing)
    {
        uac_e = eXosip_event_wait(excontext, 0, WAIT_TIMER);

        reg_remain = reg_remain - WAIT_TIMER;
        if (reg_remain < 0)
        {
            //超时，重新注册 
            eXosip_lock(excontext);
            //eXosip_automatic_refresh(excontext);
            eXosip_unlock(excontext);
            reg_remain = REG_TIMER;
            printf("register timeout, retry!");
        }

        if (dtmfing)
        {
            strcpy(dtmf, dtmf_str);
            int index;
            for (index = 0; index < 50; index++)
            {
                //依次读取字符 
                if (dtmf[index] == '\0') break;

                /* 发送DTMF */
                eXosip_lock(excontext);
                audio_stream_send_dtmf(audio, dtmf[index]);
                eXosip_unlock(excontext);

                sprintf(out_str, "DTMF send <%c> OK!", dtmf[index]);
                printf("%s", out_str);
                Sleep(500);
            }

            dtmfing = 0;
        }

        if (uac_e == NULL)
        {
            //DEBUG_INFO("nothing"); 
            continue;
        }

        eXosip_lock(excontext);
        eXosip_default_action(excontext, uac_e); /* 处理407加入鉴权信息 */
        eXosip_unlock(excontext);

        if (NULL != uac_e->response)
        {
            //UAC 消息处理前检查 
            sprintf(out_str, "%d %s\n", uac_e->response->status_code, uac_e->response->reason_phrase);
            printf(out_str);

            if (487 == uac_e->response->status_code)
            {
                printf("(取消呼叫成功)\n");
                continue;
            }

            if (480 == uac_e->response->status_code)
            {
                //480 无应答 
                printf("(无应答)\n");
                picked = 0;
                calling = 0;
                call_id = 0;
                dialog_id = 0;
                continue;
            }
            if (401 == uac_e->response->status_code)
            {
                eXosip_clear_authentication_info(excontext); //去除上次加入的错误认证信息 
                eXosip_add_authentication_info(excontext, username, username, password, "md5", NULL);
                printf("register again!\n");
            }
        }

        if (NULL != uac_e->request)
        {
        }

        if (NULL != uac_e->ack)
        {
        }

        switch (uac_e->type)
        {
        case EXOSIP_CALL_SERVERFAILURE:
        case EXOSIP_CALL_RELEASED:
        {
            /* 服务器错误或对方忙 */
            printf("(对方或服务器正忙!)");

            call_id = 0;
            dialog_id = 0;
            picked = 0;
            calling = 0;

            printf("Dest or Server Busy!");
        } break;

            /* UAS 处理事件 */
        case EXOSIP_MESSAGE_NEW: //新的消息到来
            printf("EXOSIP_MESSAGE_NEW!\n");
            if (MSG_IS_MESSAGE(uac_e->request)) //如果接受到的消息类型是MESSAGE 
            {
                osip_body_t *body;
                osip_message_get_body(uac_e->request, 0, &body);
                printf("Reveivc a msg : %s\n", body->body);
            }
            //按照规则，需要回复200 OK信息 
            eXosip_message_build_answer(excontext, uac_e->tid, 200, &answer);
            eXosip_message_send_answer(excontext, uac_e->tid, 200, answer);
            break;

        case EXOSIP_CALL_INVITE:
            sprintf(out_str, "收到来自 %s 的呼叫!", uac_e->request->from->url->string);
            printf("%s\n", out_str);

            eXosip_lock(excontext);
            eXosip_call_send_answer(excontext, uac_e->tid, 180, NULL);
            if (0 != eXosip_call_build_answer(excontext, uac_e->tid, 200, &answer))
            {
                eXosip_call_send_answer(excontext, uac_e->tid, 603, NULL);
                printf("error build answer!");
                continue;
            }
            eXosip_unlock(excontext);

            call_id = uac_e->cid; //供挂断电话上下文操作 
            dialog_id = uac_e->did;

            /* 构建本地SDP消息供媒体建立 */
            snprintf(tmp, 4096,
                "v=0\r\n"
                "o=youtoo 1 1 IN IP4 %s\r\n"
                "s=##youtoo demo\r\n"
                "c=IN IP4 %s\r\n"
                "t=0 0\r\n"
                "m=audio %d RTP/AVP 0 8 101\r\n"
                "a=rtpmap:0 PCMU/8000\r\n"
                "a=rtpmap:8 PCMA/8000\r\n"
                "a=rtpmap:101 telephone-event/8000\r\n"
                "a=fmtp:101 0-15\r\n", localip, localip, rtp_port);

            //设置回复的SDP消息体,下一步计划分析消息体 
            eXosip_lock(excontext);
            osip_message_set_body(answer, tmp, strlen(tmp));
            osip_message_set_content_type(answer, "application/sdp");

            /* 解析SDP */
            msg_req = eXosip_get_remote_sdp(excontext, uac_e->did);
            con_req = eXosip_get_audio_connection(msg_req);
            md_req = eXosip_get_audio_media(msg_req);
            eXosip_unlock(excontext);

            /*
            payload_str = (char *)osip_list_get(&md_req->m_payloads, 0); //取主叫媒体能力编码
            //暂时只支持0/8 711u/711a
            */

            calling = 1;

            while (!picked)
            {
                //未接通进入循环检测 
                Sleep(200);
            }

            eXosip_unlock(excontext);
            eXosip_call_send_answer(excontext, uac_e->tid, 200, answer);
            eXosip_unlock(excontext);

            printf("200 ok 发送");
            break;

        case EXOSIP_CALL_CANCELLED:
            /* 拒绝接听 */

            call_id = 0;
            dialog_id = 0;
            picked = 0;
            calling = 0;
            printf("被叫拒绝接听\n");
            break;

        case EXOSIP_CALL_ACK:
            /* 返回200后收到ack才建立媒体 */
            if (calling)
            {
                /* 建立RTP连接 */
                ret = build_media(rtp_port, con_req->c_addr, atoi(md_req->m_port), 0, NULL, 0, 0, 0);
                if (!ret)
                {
                    printf("媒体建立失败，无法创建通话，请挂断！");
                    //pMainWnd->OnHang(); 
                }
            }
            break;

            /* UAC 处理事件 */
        case EXOSIP_REGISTRATION_FAILURE:
            if (401 == uac_e->response->status_code)
            {
                //4o1 Unauthorized register again 
                printf("register again!\n");
                /* 先清除鉴权信息，再应用新输入的鉴权信息 */
                eXosip_clear_authentication_info(excontext);
                eXosip_add_authentication_info(excontext, username, username, password, "md5", NULL);

                osip_message_t *rereg;
                eXosip_lock(excontext);
                eXosip_register_build_register(excontext, uac_e->rid, 300, &rereg);// 
                                                                                   //取回认证的字符串authorization 
                                                                                   //osip_authorization_t *auth; 
                                                                                   ///char *strAuth; 
                                                                                   //osip_message_get_authorization((const osip_message_t *)rereg,0,&auth); 
                                                                                   //osip_authorization_to_str(auth,&strAuth); 
                                                                                   //strcpy(str_auth,strAuth);//保存认证字符串 
                eXosip_register_send_register(excontext, uac_e->rid, rereg);
                eXosip_unlock(excontext);
            }
            break;
        case EXOSIP_REGISTRATION_SUCCESS:
            printf("textinfo is %s\n", uac_e->textinfo);
            break;

        case EXOSIP_CALL_CLOSED:
            if (audio)
            {
                //被动关闭媒体连接(远端触发) 
                eXosip_lock(excontext);
                audio_stream_stop(audio);

                ortp_ev_queue_destroy(q);
                rtp_profile_destroy(profile);
                eXosip_unlock(excontext);

                audio = NULL;
                q = NULL;
                profile = NULL;
                printf("audio stream stoped!");
            }

            printf("(对方已挂断)");
            call_id = 0;
            dialog_id = 0;
            picked = 0;
            calling = 0;
            break;

        case EXOSIP_CALL_PROCEEDING:
            printf("(查找连接中..)");
            break;

        case EXOSIP_CALL_RINGING:
            printf("(对方振铃)");
            call_id = uac_e->cid;
            dialog_id = uac_e->did;

            /*
            RingStream *r;
            MSSndCard *sc;
            sc=ms_snd_card_manager_get_default_card(ms_snd_card_manager_get());
            r=ring_start("D://mbstudio//vcwork//YouToo//dial.wav",2000,sc);

            Sleep(10);
            ring_stop(r);
            */
            break;

        case EXOSIP_CALL_ANSWERED:
            //ring_stop(ring_p); 
            printf("(对方已接听)");

            call_id = uac_e->cid;
            dialog_id = uac_e->did;

            eXosip_lock(excontext);
            eXosip_call_build_ack(excontext, uac_e->did, &ack);
            eXosip_call_send_ack(excontext, uac_e->did, ack);

            /* 响应SIP消息中SDP分析 */
            msg_rsp = eXosip_get_sdp_info(uac_e->response);
            con_rsp = eXosip_get_audio_connection(msg_rsp);
            md_rsp = eXosip_get_audio_media(msg_rsp);

            /* 取服务器支持的最优先的编码方式 */
            payload_str = (char *)osip_list_get(&md_rsp->m_payloads, 0);
            eXosip_unlock(excontext);

            /* 建立RTP连接 */
            ret = build_media(rtp_port, con_rsp->c_addr, atoi(md_rsp->m_port), atoi(payload_str), NULL, 0, 0, 0);
            if (!ret)
            {
                printf("媒体建立失败，无法创建通话，请挂断！");
                //pMainWnd->OnHang(); 
            }
            break;

        default:
            break;
        }

        eXosip_event_free(uac_e);

        fflush(stdout);
    }

    return 0;
}

unsigned _stdcall sip_ua_monitor_thread(void*) {
    return sip_ua_monitor();
}




int build_media(int local_port, const char *remote_ip, int remote_port, int payload, const char *fmtp, int jitter, int ec, int bitrate)
{
    if (payload != 0 && payload != 8)
    {
        /* 目前仅支持0,8 711ulaw,711alaw */
        return -1;
    }

    PayloadType *pt;

    profile = rtp_profile_clone_full(&av_profile);
    q = ortp_ev_queue_new();

    pt = rtp_profile_get_payload(profile, payload);
    if (pt == NULL)
    {
        printf("codec error!");
        return -1;
    }

    if (fmtp != NULL) payload_type_set_send_fmtp(pt, fmtp);
    if (bitrate > 0) pt->normal_bitrate = bitrate;

    if (pt->type != PAYLOAD_VIDEO)
    {
        printf("audio stream start!");
        audio = audio_stream_start(profile, local_port, remote_ip, remote_port, payload, jitter, ec);
        if (audio)
        {
            //~session = audio->session;
            audio->ms.sessions.rtp_session;
        }
        else
        {
            printf("session error!");
            return -1;
        }
    }
    else
    {
        printf("codec select error!");
        return -1;
    }

    rtp_session_register_event_queue(session, q);
    return 0;
}

int real_send_register(int expires)
{
    char identity[100];
    char registerer[100];
    sprintf(identity, "sip:%s@%s:%s", username, localip, local_port);
    sprintf(registerer, "sip:%s:%s", server_url, server_port);

    osip_message_t *reg = NULL;

    eXosip_lock(excontext);
    int ret = eXosip_register_build_initial_register(excontext, identity, registerer, NULL, expires, &reg);
    eXosip_unlock(excontext);
    if (0 > ret)
    {
        printf("register init Failed!");
        return -1;
    }

    eXosip_lock(excontext);
    eXosip_clear_authentication_info(excontext); //去除上次加入的错误认证信息 
    eXosip_add_authentication_info(excontext, username, username, password, "md5", NULL);
    ret = eXosip_register_send_register(excontext, ret, reg);
    eXosip_unlock(excontext);
    if (0 != ret)
    {
        printf("register send Failed!");
        return -1;
    }

    return 0;
}


// 挂断
void OnHang()
{
    int ret;

    if (audio)
    {
        //主动关闭媒体连接(本地端) 
        eXosip_lock(excontext);
        audio_stream_stop(audio);

        ortp_ev_queue_destroy(q);
        rtp_profile_destroy(profile);
        eXosip_unlock(excontext);

        printf("audio stream stoped!");
        audio = NULL;
        q = NULL;
        profile = NULL;
    }

    eXosip_lock(excontext);
    ret = eXosip_call_terminate(excontext, call_id, dialog_id);
    eXosip_unlock(excontext);
    if (0 != ret)
    {
        printf("hangup terminate Failed!\n");
    }
    else
    {
        printf("(已挂断)");
        call_id = 0;
        dialog_id = 0;
        picked = 0;
        calling = 0;
    }
}


void __stdcall MyCbSipCallback(osip_message_t * msg, int received)
{
    char* dest;
    size_t length;
    osip_message_to_str(msg, &dest, &length);

    printf("\n\t\t\t\t\t <#######[%s] sip_method: %s, status_code: %d  %s \n%s\n#######>\n",
        (received == 1) ? "recv" : "send", msg->sip_method, msg->status_code, msg->reason_phrase,
        dest);
}

int my_sip_init(bool isAsServer)
{
    int ret = 0;

    excontext = eXosip_malloc();
    ret = eXosip_init(excontext);
    if (0 != ret)
    {
        printf("Couldn't initialize eXosip!\n");
        return -1;
    }

    eXosip_set_user_agent(excontext, "##YouToo0.1");


    eXosip_set_cbsip_message(excontext, MyCbSipCallback);

    int port = isAsServer ? i_server_port : i_local_port;
    ret = eXosip_listen_addr(excontext, IPPROTO_UDP, NULL, port, AF_INET, 0);
    if (0 != ret) {
        eXosip_quit(excontext);
        printf("Couldn't initialize transport layer!\n");
        return -1;
    }

    //AfxBeginThread(sip_ua_monitor,(void *)this); 
#ifdef _MSC_VER
    _beginthreadex(NULL, 0, sip_ua_monitor_thread, NULL, 0, NULL);
#else
    pthread_t id;
    pthread_create(&id, NULL, (void *)sip_ua_monitor, NULL);
#endif // _MSC_VER

    /* rtp */
    ortp_init();
    //ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR|ORTP_FATAL); 

    /* media */
    ms_init();

    return 0;
}
