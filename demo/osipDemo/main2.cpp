/** osip直接调用示例 */
/*
* File:   main.c
* Author: Soufiane Tarari
*
* Created on May 24, 2012, 2:04 PM
*/

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif


#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#ifdef _MSC_VER
#include <WinSock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sysexits.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <err.h>
#include <arpa/inet.h>
#endif // _MSC_VER

#include <osip2/osip.h>
#include <osip2/osip_fifo.h>
#include <osipparser2/osip_const.h>

/* Defines */
#if 1
#define debug(fmt, ...) do { printf("%s() : %04d\n", __FUNCTION__, __LINE__); } while (0)
#else
#define debug(...)
#endif

#define BUFFSIZE 1600

/* Types */
typedef struct _udp_transport_layer_t {
    int in_port;
    int in_socket;

    int out_port;
    int out_socket;

    int control_fds[2];
} udp_transport_layer_t;

/* Variables */
static udp_transport_layer_t	*_udp_tl;

/* Prototypes */
int  cb_udp_snd_message(osip_transaction_t *tr, osip_message_t *sip, char *host, int port, int out_socket);
void cb_RcvICTRes(int i, osip_transaction_t *tr, osip_message_t *message);
void cb_RcvNICTRes(int i, osip_transaction_t *tr, osip_message_t *message);
void cb_RcvNot(int i, osip_transaction_t *tr, osip_message_t *message);
void cb_ISTTranKill(int i, osip_transaction_t *osip_transaction);
void *Notify(void* arg);
void cb_RcvSub(int i, osip_transaction_t *tran, osip_message_t *msg);
void cb_RcvRegisterReq(int i, osip_transaction_t *tran, osip_message_t *msg);
void cb_RcvISTReq(int i, osip_transaction_t *tran, osip_message_t *msg);
int  BuildReq(const osip_message_t *RCVrequest, osip_message_t **SNDrequest);
int  BuildResponse(const osip_message_t *request, osip_message_t **response);
void ProcessNewReq(osip_t *osip, osip_event_t *evt);
void TransportProcess(osip_t *osip, int g_sock);
int  InitNet(void);
void SetCallbacks(osip_t *osip);

int main2(int argc, char** argv)
{
    struct timeval  sleept;
    osip_t          *osip = NULL;
    int             g_sock, rc;
    fd_set          rdset, erset;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    /* Initialise the parser and the state machine */
    rc = osip_init(&osip);
    assert(rc == OSIP_SUCCESS);

    g_sock = InitNet();

    /* Set the callback functions */
    SetCallbacks(osip);

    while (true)
    {
        osip_ict_execute(osip);
        osip_ist_execute(osip);
        osip_nict_execute(osip);
        osip_nist_execute(osip);
        osip_timers_ict_execute(osip);
        osip_timers_ist_execute(osip);
        osip_timers_nict_execute(osip);
        osip_timers_nist_execute(osip);
        osip_timers_gettimeout(osip, &sleept);

        FD_ZERO(&rdset);
        FD_ZERO(&erset);
        FD_SET(g_sock, &rdset);
        FD_SET(g_sock, &erset);

        debug("Waiting up to %d seconds %d msec for a packet", (int)sleept.tv_sec, sleept.tv_usec / 1000);

        rc = select(g_sock + 1, &rdset, NULL, &erset, &sleept);
        assert(rc == -1);

        /* Timed out */
        if (rc == 0) {
            debug("Timed out");
            continue;
        }

        /* Ready for read */
        if (FD_ISSET(g_sock, &rdset)) {
            TransportProcess(osip, g_sock);
        }

        /* Socket error ? */
        if (FD_ISSET(g_sock, &erset)) {
            debug("Error on socket");
        }
    }

    closesocket(g_sock);

    WSACleanup();
    return (EXIT_SUCCESS);
}



/* Code */
int cb_udp_snd_message(osip_transaction_t *tr, osip_message_t *sip, char *host, int port, int out_socket) {
    int			num, i, sock;
    char		*message;
    struct sockaddr_in addr;
    size_t		bufLen;
    osip_via_t		*via;

    num = 0;

    debug("cb_udp_snd_message");

    i = osip_message_to_str(sip, &message, &bufLen);

    if (i != 0)
        return -1;

    via = (osip_via_t *)osip_list_get(&sip->vias, 0);
    addr.sin_port = htons(atoi(via->port));
    addr.sin_family = AF_INET;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    //~inet_aton(via->host, &addr.sin_addr);

    if (0 > sendto(sock, message, BUFFSIZE, 0, (struct sockaddr *) &addr, sizeof(addr))) {
        if (ECONNREFUSED == errno) {
            OSIP_TRACE(osip_trace(__FILE__, __LINE__, OSIP_INFO3, stdout, "SIP_ECONNREFUSED - No remote server.\n"));
            /* This can be considered as an error, but for the moment,
            I prefer that the application continue to try sending
            message again and again... so we are not in a error case.
            Nevertheless, this error should be announced!
            ALSO, UAS may not have any other options than retry always
            on the same port.
            */
            return 1;
        }
        else {
            OSIP_TRACE(osip_trace(__FILE__, __LINE__, OSIP_INFO3, stdout, "SIP_NETWORK_ERROR - Network error.\n"));
            /* SIP_NETWORK_ERROR; */
            return -1;
        }
    }

    debug("<--- Transmitting to %s:%s --->", via->host, via->port);
    //osip_message_to_str(response, &buf, &bufLen);
    debug("%s", message);

    if (strncmp(message, "INVITE", 6) == 0) {
        num++;
        debug("number of message sent: %i", num);
    }

    OSIP_TRACE(osip_trace(__FILE__, __LINE__, OSIP_INFO2, NULL, "SND UDP MESSAGE.\n"));
    debug("end cb_udp_snd_message");
    return 0;
}

void cb_RcvICTRes(int i, osip_transaction_t *tr, osip_message_t *message) {
    debug("cb_RcvICTRes fired");
}

void cb_RcvNICTRes(int i, osip_transaction_t *tr, osip_message_t *message) {
    debug("cb_RcvNICTRes fired");
}

void cb_RcvNot(int i, osip_transaction_t *tr, osip_message_t *message) {
    //OSIP_TRACE (osip_trace(__FILE__, __LINE__, OSIP_INFO1, NULL, "psp_core_cb_rcvregister!\n"));   
    //psp_core_event_add_sfp_inc_traffic(tr);
    debug("NOTIFY SIP message received");
}

void cb_ISTTranKill(int i, osip_transaction_t *osip_transaction) {
    debug("cb_ISTTranKill fired");
}

void * Notify(void* arg) {
    osip_transaction_t	*tran = (osip_transaction_t*)arg;
    osip_message_t	*response = NULL;
    osip_event_t	*evt = NULL;
    char		act, *sdp, *mime;

    debug("Notify");

    BuildResponse(tran->orig_request, &response);

    printf("incoming call from %s\n [a]nswer or [d]ecline?\n", (char *)tran->from->url);
    scanf("%c", &act);
    if ('a' == act) {
        //accept call
        osip_message_set_status_code(response, SIP_OK);
        osip_message_set_reason_phrase(response, osip_strdup("OK"));
        mime = "application/sdp";
        osip_message_set_body_mime(response, mime, strlen(mime));
        osip_message_set_content_type(response, mime);
        sdp = "v=0\r\n\
            o=raymond 323456 654323 IN IP4 127.0.0.1\r\n\
            s=A conversation\r\n\
            c=IN IP4 127.0.0.1\r\n\
            t=0 0\r\n\
            m=audio 7078 RTP/AVP 111 110 0 8 101\r\n\
            a=rtpmap:111 speex/16000/1\r\n\
            a=rtpmap:110 speex/8000/1\r\n\
            a=rtpmap:0 PCMU/8000/1\r\n\
            a=rtpmap:8 PCMA/8000/1\r\n\
            a=rtpmap:101 telephone-event/8000\r\n\
            a=fmtp:101 0-11\r\n";

        osip_message_set_body(response, osip_strdup(sdp), strlen(sdp));
    }
    else {
        // decline call
        osip_message_set_status_code(response, SIP_DECLINE);
        osip_message_set_reason_phrase(response, osip_strdup("Decline\n"));
        osip_message_set_contact(response, NULL);
    }

    evt = osip_new_outgoing_sipmessage(response);
    evt->transactionid = tran->transactionid;
    osip_transaction_add_event(tran, evt);
    return NULL;
}

void cb_RcvSub(int i, osip_transaction_t *tran, osip_message_t *msg) {
    debug("SUBSCRIBE SIP message received");
}

void cb_RcvRegisterReq(int i, osip_transaction_t *tran, osip_message_t *msg) {
    osip_via_t		*via;
    char		*buf;
    size_t		bufLen;
    osip_message_t	*response = NULL;
    osip_event_t	*evt = NULL;

    via = (osip_via_t *)osip_list_get(&msg->vias, 0);
    debug("<--- Received from %s:%s --->", via->host, via->port);
    osip_message_to_str(msg, &buf, &bufLen);
    debug("%s", buf);

    BuildResponse(msg, &response);
    osip_message_set_status_code(response, SIP_TRYING);
    evt = osip_new_outgoing_sipmessage(response);
    osip_message_set_reason_phrase(response, osip_strdup("Trying"));
    osip_transaction_add_event(tran, evt);

    /*BuildReq(msg, &response);
    osip_message_set_method(response, osip_strdup("OPTIONS"));
    //osip_call_id_init(&response->call_id);
    //osip_call_id_set_host(response->call_id, osip_strdup("172.16.156.1"));
    //osip_call_id_set_number(response->call_id, osip_strdup("1c6d6ad5155cc32b4407635d08c6ff13"));
    osip_message_set_content_length(response, 0);
    evt = osip_new_outgoing_sipmessage (response);
    osip_transaction_add_event(tran, evt);*/

    BuildResponse(msg, &response);
    osip_message_set_status_code(response, SIP_OK);
    evt = osip_new_outgoing_sipmessage(response);
    osip_message_set_reason_phrase(response, osip_strdup("OK"));
    osip_transaction_add_event(tran, evt);

    debug("end RcvRegisterReq");
}


void cb_RcvISTReq(int i, osip_transaction_t *tran, osip_message_t *msg) {
    osip_via_t		*via;
    char		*buf;
    size_t		bufLen;
    osip_message_t	*response = NULL;
    osip_event_t	*evt = NULL;

    via = (osip_via_t *)osip_list_get(&msg->vias, 0);
    debug("<--- INVITE SIP message received from %s:%s --->", via->host, via->port);
    osip_message_to_str(msg, &buf, &bufLen);
    debug("%s", buf);

    BuildResponse(msg, &response); //trying
    osip_message_set_status_code(response, SIP_TRYING);
    evt = osip_new_outgoing_sipmessage(response);
    osip_message_set_reason_phrase(response, osip_strdup("Trying\n"));
    osip_transaction_add_event(tran, evt);

    BuildResponse(msg, &response); //dialog establishement
    osip_message_set_status_code(response, 101);
    evt = osip_new_outgoing_sipmessage(response);
    osip_message_set_reason_phrase(response, osip_strdup("Dialog Establishement\n"));
    osip_transaction_add_event(tran, evt);

    BuildResponse(msg, &response); //ringing
    osip_message_set_status_code(response, SIP_RINGING);
    evt = osip_new_outgoing_sipmessage(response);
    osip_message_set_reason_phrase(response, osip_strdup("Ringing\n"));
    osip_transaction_add_event(tran, evt);
}

int BuildReq(const osip_message_t *RCVrequest, osip_message_t **SNDrequest) {
    debug("BuildReq");
    osip_via_t		*viaReq, *via;
    osip_message_t	*msg = NULL;
    osip_uri_t		*uri;
    osip_uri_t		*uri2;
    int			pos;
    struct tm		*loctime;
    time_t		curtime;

    osip_message_init(&msg);

    viaReq = (osip_via_t *)osip_list_get(&RCVrequest->vias, 0);
    osip_from_init(&msg->from); //allocating and setting the From field
    osip_uri_init(&uri);
    osip_uri_set_host(uri, osip_strdup("translator@172.16.156.1"));
    osip_from_set_url(msg->from, uri);
    osip_from_set_displayname(msg->from, osip_strdup("\"Translator\""));
    osip_from_set_tag(msg->from, osip_strdup("123456"));

    osip_to_clone(RCVrequest->to, &msg->to);
    osip_message_set_contact(msg, osip_strdup("<sip:translator@172.16.156.1>"));

    osip_via_init(&via);
    via->host = osip_strdup("172.16.156.1");
    via->protocol = osip_strdup("UDP");
    via->version = osip_strdup("2.0");
    via->port = osip_strdup("5040");
    osip_list_add(&(msg->vias), via, -1);

    osip_message_set_version(msg, osip_strdup("SIP/2.0"));

    osip_uri_init(&uri2);
    osip_uri_set_host(uri2, viaReq->host);
    osip_uri_set_port(uri2, viaReq->port);
    osip_message_set_uri(msg, uri2);

    osip_message_set_user_agent(msg, osip_strdup("Soufiane/1.0.0 PBX"));
    osip_message_set_status_code(msg, 0);
    osip_message_set_reason_phrase(msg, NULL);
    osip_message_set_max_forwards(msg, "70");

    pos = 0;//copy vias from request to response
    while (!osip_list_eol(&RCVrequest->allows, pos)) {
        osip_allow_t *allow;
        osip_allow_t *allow2;

        allow = (osip_allow_t *)osip_list_get(&RCVrequest->allows, pos);
        int i = osip_allow_clone(allow, &allow2);
        if (i != 0)
        {
            osip_message_free(msg);
            return i;
        }
        osip_list_add(&(msg->allows), allow2, -1);
        pos++;
    }

    time(&curtime);
    loctime = localtime(&curtime);
    osip_message_set_date(msg, asctime(loctime));

    osip_message_set_supported(msg, "replaces");

    *SNDrequest = msg;
    debug("end BuildReq");
    return 0;
}

int BuildResponse(const osip_message_t *request, osip_message_t **response) {
    osip_message_t	*msg = NULL;
    int			pos;
    osip_contact_t	*contact;

    debug("BuildResponse");
    osip_message_init(&msg);

    osip_from_clone(request->from, &msg->from);
    osip_to_clone(request->to, &msg->to);
    osip_cseq_clone(request->cseq, &msg->cseq);
    osip_call_id_clone(request->call_id, &msg->call_id);

    pos = 0; //copy vias from request to response
    while (!osip_list_eol(&request->vias, pos)) {
        osip_via_t *via;
        osip_via_t *via2;

        via = (osip_via_t *)osip_list_get(&request->vias, pos);
        int i = osip_via_clone(via, &via2);
        if (i != 0)
        {
            osip_message_free(msg);
            return i;
        }
        osip_list_add(&(msg->vias), via2, -1);
        pos++;
    }

    osip_to_set_tag(msg->to, osip_strdup("4893693")); // set to tag in response
    osip_message_set_version(msg, osip_strdup("SIP/2.0"));
    osip_contact_clone((osip_contact_t*)osip_list_get(&(request->contacts), 0), &contact);
    osip_list_add(&(msg->contacts), contact, 0);
    osip_message_set_user_agent(msg, osip_strdup("Soufiane/1.0.0 PBX"));
    *response = msg;

    debug("end BuildResponse");

    return 0;
}

void ProcessNewReq(osip_t *osip, osip_event_t *evt) {
    osip_transaction_t	*tran;
    //int		g_sock;

    debug("ProcessNewReq");
    osip_transaction_init(&tran, NIST, osip, evt->sip);
    //osip_transaction_set_in_socket (tran, g_sock);
    //osip_transaction_set_out_socket (tran, g_sock);
    osip_transaction_set_your_instance(tran, osip);// store osip in transaction for later usage
    osip_transaction_add_event(tran, evt);
    debug("end ProcessNewReq");
}

void TransportProcess(osip_t *osip, int g_sock) {
    int         rc;
    int         len;
    char        buf[BUFFSIZE];
    struct sockaddr	from;
    int      addrSize;
    osip_event_t    *evt;

    debug("TransportProcess");

    /* Loop processing packets */
    while (true)
    {
        len = recvfrom(g_sock, buf, BUFFSIZE, 0, &from, &addrSize);
        if (len == -1) {
            /* Nothing to do */
            if (errno == EAGAIN)
                return;
            else
                debug("Error reading from socket: %s", strerror(errno));
        }

        if (len == 0) {
            debug("Runt packet");
            continue;
        }

        buf[len] = 0;
        if ((evt = osip_parse(buf, len)) == NULL) {
            debug("Unable to parse buffer");
            continue;
        }

        rc = osip_find_transaction_and_add_event(osip, evt);
        if (0 != rc) {
            debug("this event has no transaction, creating a new one.");
            ProcessNewReq(osip, evt);
        }
        debug("******** this is the osip after: %d, method: %s, tr id: %d", osip->osip_nist_transactions.nb_elt, evt->sip->sip_method, evt->transactionid);
    }
    return;
}

int InitNet(void)
{
    struct sockaddr_in  param;
    int                 sock/*, fl*/;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(sock != -1);

    //设置socket为非阻塞模式
    /*if ((fl = fcntl(sock, F_GETFL, NULL)) == -1)
    err(EX_OSERR, "fcntl(.., F_GETFL)");

    fl |= O_NONBLOCK;
    if ((fl = fcntl(sock, F_SETFL, fl)) == -1)
    err(EX_OSERR, "fcntl(.., F_SETFL)");*/

    unsigned long argp = 1;
    int result = ioctlsocket(sock, FIONBIO, (unsigned long*)&argp);
    assert(result != SOCKET_ERROR);


    param.sin_family = AF_INET;
    param.sin_port = htons(5040);
    param.sin_addr.s_addr = inet_addr("0.0.0.0");

    result = bind(sock, (struct sockaddr*)&param, sizeof(param));
    assert(result != SOCKET_ERROR);

    debug("Network ready");
    return(sock);
}

void SetCallbacks(osip_t *osip)
{
    debug("SetCallbacks");

    osip_set_cb_send_message(osip, &cb_udp_snd_message);
    osip_set_message_callback(osip, OSIP_IST_INVITE_RECEIVED, &cb_RcvISTReq);
    osip_set_message_callback(osip, OSIP_IST_INVITE_RECEIVED_AGAIN, &cb_RcvISTReq);
    osip_set_message_callback(osip, OSIP_ICT_STATUS_1XX_RECEIVED, &cb_RcvICTRes);
    osip_set_message_callback(osip, OSIP_NICT_STATUS_1XX_RECEIVED, &cb_RcvNICTRes);
    osip_set_message_callback(osip, OSIP_NIST_NOTIFY_RECEIVED, &cb_RcvNot);
    osip_set_message_callback(osip, OSIP_NIST_SUBSCRIBE_RECEIVED, &cb_RcvSub);
    osip_set_message_callback(osip, OSIP_NIST_REGISTER_RECEIVED, &cb_RcvRegisterReq);
    osip_set_kill_transaction_callback(osip, OSIP_IST_KILL_TRANSACTION, &cb_ISTTranKill);
}