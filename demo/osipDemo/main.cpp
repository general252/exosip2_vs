/** 网上找的测试示例(稍微改动) */

#include <eXosip2/eXosip.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <sys/types.h>

#define snprintf _snprintf

// 调用过程分析
// 参考:
// http://blog.sina.com.cn/s/blog_6a0a1ba30100juzx.html
// http://blog.csdn.net/bikeytang/article/details/53448735
int main1(int argc, char *argv[])
{
    struct eXosip_t * sip = NULL;
    eXosip_event_t *je;
    osip_message_t *reg = NULL;
    osip_message_t *invite = NULL;
    osip_message_t *ack = NULL;
    osip_message_t *info = NULL;
    osip_message_t *message = NULL;

    int call_id, dialog_id;
    int i, flag;
    int flag1 = 1;

    // char *identity = "sip:140@192.168.0.140";
    // char *registerer = "sip:192.168.0.32:5060";
    char *source_call = "sip:300@192.168.6.80";
    char *dest_call = "sip:200@192.168.6.80:5060";

    char command;
    char tmp[4096];

    printf("r     向服务器注册\n\n");
    printf("c     取消注册\n\n");
    printf("i     发起呼叫请求\n\n");
    printf("h     挂断\n\n");
    printf("q     退出程序\n\n");
    printf("s     执行方法INFO\n\n");
    printf("m     执行方法MESSAGE\n\n");
    //初始化
    sip = eXosip_malloc();
    i = eXosip_init(sip);
    if (i != 0) {
        printf("Couldn't initialize eXosip!\n");
        return -1;
    }
    else {
        printf("eXosip_init successfully!\n");
    }

    i = eXosip_listen_addr(sip, IPPROTO_UDP, NULL, 5080, AF_INET, 0);
    if (i != 0) {
        eXosip_quit(sip);
        fprintf(stderr, "Couldn't initialize transport layer!\n");
        return -1;
    }

    flag = 1;
    while (flag)
    {
        printf("please input the comand:\n");

        scanf("%c", &command);
        getchar();

        switch (command)
        {
        case 'r':
        {
            printf("This modal isn't commpleted!\n");
        } break;
        case 'i':/* INVITE */
        {
            i = eXosip_call_build_initial_invite(sip, &invite, dest_call, source_call, NULL, "This si a call for a conversation");
            if (i != 0) {
                printf("Intial INVITE failed!\n");
                break;
            }

            /**
            * SDP message definition.
            * @struct sdp_message
            */
            //struct sdp_message
            //{
            //    char *v_version;            /**< version header */
            //    char *o_username;           /**< Username */
            //    char *o_sess_id;            /**< Identifier for session */
            //    char *o_sess_version;       /**< Version of session */
            //    char *o_nettype;            /**< Network type */
            //    char *o_addrtype;           /**< Address type */
            //    char *o_addr;               /**< Address */
            //    char *s_name;               /**< Subject header */
            //    char *i_info;               /**< Information header */
            //    char *u_uri;                /**< Uri header */
            //    osip_list_t e_emails;      /**< list of mail address */
            //    osip_list_t p_phones;      /**< list of phone numbers * */
            //    sdp_connection_t *c_connection;   /**< Connection information */
            //    osip_list_t b_bandwidths;  /**< list of bandwidth info (sdp_bandwidth_t) */
            //    osip_list_t t_descrs;      /**< list of time description (sdp_time_descr_t) */
            //    char *z_adjustments;        /**< Time adjustment header */
            //    sdp_key_t *k_key;           /**< Key information header */
            //    osip_list_t a_attributes;  /**< list of global attributes (sdp_attribute_t) */
            //    osip_list_t m_medias;      /**< list of supported media (sdp_media_t) */
            //};
            /* In SDP, headers must be in the right order */
            /* This is a simple example
            v=0
            o=user1 53655765 2353687637 IN IP4 128.3.4.5
            s=Mbone Audio
            i=Discussion of Mbone Engineering Issues
            e=mbone@somewhere.com
            c=IN IP4 224.2.0.1/127
            t=0 0
            m=audio 3456 RTP/AVP 0
            a=rtpmap:0 PCMU/8000
            */
            //符合SDP格式,其中属性a是自定义格式,也就是说可以存放自己的信息,但是只能是两列,比如帐户信息
            //但是经测试,格式:v o t必不可少,原因未知,估计是协议栈在传输时需要检查的
            snprintf(tmp, 4096,
                "v=0\r\n"
                "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"
                "t=1 10\r\n"
                "a=username:300\r\n"
                "a=password:300\r\n");
            osip_message_set_body(invite, tmp, strlen(tmp));
            osip_message_set_content_type(invite, "application/sdp");

            eXosip_lock(sip);
            i = eXosip_call_send_initial_invite(sip, invite);
            eXosip_unlock(sip);
            flag1 = 1;
            while (flag1)
            {
                je = eXosip_event_wait(sip, 0, 10000);
                if (je == NULL) {
                    printf("No response or the time is over!\n");
                    break;
                }

                switch (je->type)
                {
                case EXOSIP_CALL_INVITE:
                    printf("a new invite reveived!\n");
                    break;
                case EXOSIP_CALL_PROCEEDING:
                    printf("proceeding!\n");
                    break;
                case EXOSIP_CALL_RINGING:
                    printf("ringing!\n");
                    // call_id = je->cid;
                    // dialog_id = je->did;
                    printf("call_id is %d, dialog_id is %d \n", je->cid, je->did);
                    break;
                case EXOSIP_CALL_ANSWERED:
                    printf("ok! connected!\n");
                    call_id = je->cid;
                    dialog_id = je->did;
                    printf("call_id is %d, dialog_id is %d \n", je->cid, je->did);

                    eXosip_call_build_ack(sip, je->did, &ack);
                    eXosip_call_send_ack(sip, je->did, ack);
                    flag1 = 0;
                    break;
                case EXOSIP_CALL_CLOSED:
                    printf("the other sid closed!\n");
                    break;
                case EXOSIP_CALL_ACK:
                    printf("ACK received!\n");
                    break;
                default:
                    printf("other response!\n");
                    break;
                }
                eXosip_event_free(je);

            }
        } break;
        case 'h':
        {
            printf("Holded !\n");

            eXosip_lock(sip);
            eXosip_call_terminate(sip, call_id, dialog_id);
            eXosip_unlock(sip);
        } break;
        case 'c':
        {
            printf("This modal isn't commpleted!\n");
        } break;
        case 's':
        {
            //传输INFO方法
            eXosip_call_build_info(sip, dialog_id, &info);
            snprintf(tmp, 4096, "hello,rainfish");
            osip_message_set_body(info, tmp, strlen(tmp));
            //格式可以任意设定,text/plain代表文本信息
            osip_message_set_content_type(info, "text/plain");
            eXosip_call_send_request(sip, dialog_id, info);
        } break;
        case 'm':
        {
            //传输MESSAGE方法,也就是即时消息，和INFO方法相比，我认为主要区别，是MESSAGE不用建立连接，直接传输信息，而INFO必须
            //在建立INVITE的基础上传输。
            printf("the mothed :MESSAGE\n");
            eXosip_message_build_request(sip, &message, "MESSAGE", dest_call, source_call, NULL);
            snprintf(tmp, 4096,
                "hellor rainfish");
            osip_message_set_body(message, tmp, strlen(tmp));
            //假设格式是xml
            osip_message_set_content_type(message, "text/xml");
            eXosip_message_send_request(sip, message);
        } break;
        case 'q':
        {
            eXosip_quit(sip);
            printf("Exit the setup!\n");
            flag = 0;
        } break;
        }
    }
    return (0);
}

