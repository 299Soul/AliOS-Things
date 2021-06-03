/**
 * ant_soc.h
 *
 * �����׽��ֽӿ�
 */
#ifndef __ANT_SOC_H__
#define __ANT_SOC_H__

#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ANT_SOC_AF_INET         (0)
#define ANT_SOC_PF_INET         (0)
#define ANT_SOC_IPPROTO_IP      (0)
#define ANT_SOC_IPPROTO_ICMP    (1)
#define ANT_MAX_SOCKET_NUM      (10)
#define ANT_MAX_SOCK_ADDR_LEN   (28)
#define ANT_MAX_HOST_NAME_LEN   (64)
#define ANT_RESOLVE_HOST_NUM    (5)
#define ANT_DNS_HOST_NUM        (10)

/* Socket Type */ 
typedef enum {
    ANT_SOC_TYPE_TCP = 0,  /* TCP */ 
    ANT_SOC_TYPE_UDP,      /* datagram socket, UDP */ 
    ANT_SOC_TYPE_RAW,      /* RAW socket, ICMP, .. */     
} ant_sock_type_e;

typedef enum
{
    ANT_SOC_READ    = 0x01,  /* Notify for read */
    ANT_SOC_WRITE   = 0x02,  /* Notify for write */
    ANT_SOC_ACCEPT  = 0x04,  /* Notify for accept */
    ANT_SOC_CONNECT = 0x08,  /* Notify for connect */
    ANT_SOC_CLOSE   = 0x10   /* Notify for close */
} ant_soc_event_e;

/****************************************************************************
 * Return Codes Definition for TCP/IP
 ***************************************************************************/
typedef enum {
    ANT_SOC_SUCCESS             = 0,
    ANT_SOC_ERROR               = -1,
    ANT_SOC_WOULDBLOCK          = -2,
    ANT_SOC_LIMIT_RESOURCE      = -3,    /* limited resource */
    ANT_SOC_INVALID_SOCKET      = -4,    /* invalid socket */
    ANT_SOC_INVALID_ACCOUNT     = -5,    /* invalid account id */
    ANT_SOC_NAMETOOLONG         = -6,    /* address too long */
    ANT_SOC_ALREADY             = -7,    /* operation already in progress */
    ANT_SOC_OPNOTSUPP           = -8,    /* operation not support */
    ANT_SOC_CONNABORTED         = -9,    /* Software caused connection abort */
    ANT_SOC_INVAL               = -10,   /* invalid argument */
    ANT_SOC_PIPE                = -11,   /* broken pipe */
    ANT_SOC_NOTCONN             = -12,   /* socket is not connected */
    ANT_SOC_MSGSIZE             = -13,   /* msg is too long */
    ANT_SOC_BEARER_FAIL         = -14,   /* bearer is broken */
    ANT_SOC_CONNRESET           = -15,   /* TCP half-write close, i.e., FINED */
    ANT_SOC_DHCP_ERROR          = -16,
    ANT_SOC_IP_CHANGED          = -17,
    ANT_SOC_ADDRINUSE           = -18,
    ANT_CANCEL_ACT_BEARER       = -19,   /* cancel the activation of bearer */
    ANT_SOC_TIMEOUT             = -20,
    ANT_SOC_EINTR               = -100
} ant_soc_errcode_e;

/* Socket Options */
typedef enum
{
    ANT_SOC_OOBINLINE     = 0x01 << 0,  /* not support yet */
    ANT_SOC_LINGER        = 0x01 << 1,  /* linger on close */
    ANT_SOC_NBIO          = 0x01 << 2,  /* Nonblocking */
    ANT_SOC_ASYNC         = 0x01 << 3,  /* Asynchronous notification */   
    ANT_SOC_NODELAY       = 0x01 << 4,  /* disable Nagle algorithm or not */
    ANT_SOC_KEEPALIVE     = 0x01 << 5,  /* enable/disable the keepalive */
    ANT_SOC_RCVBUF        = 0x01 << 6,  /* set the socket receive buffer size */
    ANT_SOC_SENDBUF       = 0x01 << 7,  /* set the socket send buffer size */
    ANT_SOC_NREAD         = 0x01 << 8,  /* no. of bytes for read, only for soc_getsockopt */
    ANT_SOC_PKT_SIZE      = 0x01 << 9,  /* datagram max packet size */
    ANT_SOC_SILENT_LISTEN = 0x01 << 10, /* SOC_SOCK_SMS property */
    ANT_SOC_QOS           = 0x01 << 11, /* set the socket qos */
    ANT_SOC_TCP_MAXSEG    = 0x01 << 12, /* set the max segmemnt size */
    ANT_SOC_IP_TTL        = 0x01 << 13, /* set the IP TTL value */
    ANT_SOC_LISTEN_BEARER = 0x01 << 14, /* enable listen bearer */
    ANT_SOC_UDP_ANY_FPORT = 0x01 << 15, /* enable UDP any foreign port */
    ANT_SOC_WIFI_NOWAKEUP = 0x01 << 16, /* send packet in power saving mode */
    ANT_SOC_UDP_NEED_ICMP = 0x01 << 17, /* deliver NOTIFY(close) for ICMP error */
    ANT_SOC_IP_HDRINCL    = 0x01 << 18, /* IP header included for raw sockets */
    ANT_SOC_IPSEC_POLICY  = 0x01 << 19, /* ip security policy */
    ANT_SOC_TCP_ACKED_DATA  = 0x01 << 20, /* TCPIP acked data */
    ANT_SOC_TCP_DELAYED_ACK = 0x01 << 21, /* TCP delayed ack */
    ANT_SOC_TCP_SACK        = 0x01 << 22, /* TCP selective ack */
    ANT_SOC_TCP_TIME_STAMP  = 0x01 << 23, /* TCP time stamp */
    ANT_SOC_TCP_ACK_MSEG    = 0x01 << 24  /* TCP ACK multiple segment */
} ant_soc_option_e;

typedef struct {
    char *h_name;
    char **h_aliases;
    int  h_addrtype;
    int  h_length;
    char **h_addr_list;
} ant_hostent;

typedef struct {
    ant_u8     sin_len;
    ant_u8     sin_family;
    ant_u16    sin_port;
    ant_u32    sin_addr;
    ant_u8     sin_zero[8];
} ant_sockaddr;

typedef struct 
{
    ant_u32 tv_sec;  /* select timeout: no. of second */
    ant_u32 tv_usec; /* select timeout: no. of microsecond */
} ant_timeval;

typedef struct {
    ant_bool  onoff;       /* ANT_TRUE: enable linger timeout ANT_FALSE: disable linger timeout */
    ant_u16   linger_time; /* linger timeout (unit in seconds) */   
} ant_soc_linger;

typedef struct {
    ant_u8  fds_bits[ANT_MAX_SOCKET_NUM]; /* select set based on socket id */
} ant_fd_set;

#define ANT_SOC_FD_ZERO(p)    memset(p, 0x00, sizeof(*(p)))
#define ANT_SOC_FD_SET(s,p)   ((p)->fds_bits[s] |= 0x01)
#define ANT_SOC_FD_CLR(s,p)   ((p)->fds_bits[s] &= ~(0x01))
#define ANT_SOC_FD_ISSET(s,p) ((p)->fds_bits[s] & 0x02)

typedef struct {
    void (*callback_socket_connect)(ant_s32 sock, ant_s32 errcode, void *param);
    void (*callback_socket_close)(ant_s32 sock, ant_s32 errcode, void *param);
    void (*callback_socket_accept)(ant_s32 sock, ant_s32 errcode, void *param);
    void (*callback_socket_read)(ant_s32 sock, ant_s32 errcode, void *param);
    void (*callback_socket_write)(ant_s32 sock, ant_s32 errcode, void *param);
} ant_soc_callback_t;

/**
 * �����ʼ��
 *
 * @return  0: ��ʼ���ɹ�
 *         -1: ��ʼ��ʧ��
 */
ant_s32 ant_network_init(void);

/**
 * ��ȡ����ip��ַ
 *
 * @param host_name ��������
 * @param addr 32λIP��ַ�������ֽ���
 * @param time_out ��ʱʱ�䣬��λΪ����
 * @return    0�� �ɹ�����ȡ������IP��ַ
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_gethostbyname(const ant_char *host_name, ant_u32 *addr, ant_u32 time_out);

/**
 * ע��socket�����ص�����
 *
 * @param sock socket���
 * @param cb �ص�����
 * @param param �ص������û�����
 * @return    0�� �ɹ���ע��ɹ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_register_callback(ant_s32 sock, ant_soc_callback_t cb, void *param);

/**
 * ����socket
 *
 * @param stream_type socket���ͣ�ȡֵ�μ�ant_sock_type_e
 * @param protocol Э�����ͣ���ǰ�̶�ֵΪ0
 *
 * @return ���ڵ���0�� �����ɹ�������socket���
 *            С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_create(ant_u32 stream_type, ant_u32 protocol);
ant_s32 ant_soc_create_async(ant_u32 stream_type, ant_u32 protocol);

/**
 * ����socket����
 *
 * @param sock socket���
 * @param option Ҫ���õ�socket���ԣ�ȡֵ�μ�ant_soc_option_e����
 * @param val ����ֵ�����ַ
 * @param val_size ����ֵ�����С
 * @return   0�� �ɹ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_setsockopt(ant_s32 sock, ant_u32 option, void *val, ant_u32 val_size);

/**
 * ��ȡsocket����
 *
 * @param sock socket���
 * @param option Ҫ��ȡ��socket���ԣ�ȡֵ�μ�ant_soc_option_e����
 * @param val ����ֵ�����ַ
 * @param val_size ����ֵ�����С
 * @return   0�� �ɹ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_getsockopt(ant_s32 sock, ant_u32 option, void *val, ant_u32 val_size);

/**
 * ��������
 *
 * @param sock socket���
 * @param addr ������ַ
 * @return    0: �ɹ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_connect(ant_s32 sock, ant_sockaddr *addr);

/**
 * ��������(TCP)
 *
 * @param sock socket���
 * @param data �������ݻ����ַ
 * @param len �������ݵĳ���
 * @return ���ڵ���0: ���ͳɹ��������ѷ������ݵĳ���
 *            С��0: ����ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_send(ant_s32 sock, const ant_u8 *data, ant_s32 len);

/**
 * ��ָ����������������(UDP)
 *
 * @param sock socket���
 * @param data �������ݻ����ַ
 * @param len �������ݵĳ���
 * @param sock_addr ָ����������ַ����
 * @param size ָ����������ַ����
 * @return ���ڵ���0: ���ͳɹ��������ѷ������ݵĳ���
 *            С��0: ����ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_sendto(ant_s32 sock, ant_u8 *data, ant_s32 len, 
                       ant_sockaddr *sock_addr, ant_u32 size);

/**
 * �������ݣ�TCP��
 *
 * @param sock socket���
 * @param data �������ݻ����ַ
 * @param len �������ݻ���ĳ���
 * @return ���ڵ���0: ���ճɹ��������ѽ������ݵĳ���
 *            С��0: ����ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_recv(ant_s32 sock, ant_u8 *data, ant_s32 len);

/**
 * ��ָ����������������(UDP)
 *
 * @param sock socket���
 * @param data �������ݻ����ַ
 * @param len �������ݵĳ���
 * @param sock_addr ָ����������ַ����
 * @param size ָ����������ַ����
 * @return ���ڵ���0: ���ճɹ��������ѷ������ݵĳ���
 *            С��0: ����ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_recvfrom(ant_s32 sock, ant_u8 *data, ant_s32 len, 
                       ant_sockaddr *sock_addr, ant_u32 size);

/**
 * ��������ַ(�˿ں�)
 *
 * @param sock socket���
 * @param sock_addr �󶨵�������ַ(�˿ں�)����
 * @param len �󶨵�������ַ(�˿ں�)���泤��
 * @return ���ڵ���0: �󶨳ɹ�
 *            С��0: ��ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_bind(ant_s32 sock, ant_sockaddr *sock_addr, ant_u32 len);

/**
 * ����socket
 *
 * @param sock socket���
 * @return    0: �ɹ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_listen(ant_s32 sock);

/**
 * ������������
 *
 * @param sock ����socket���
 * @param sock_addr ����ɹ������ضԶ�������ַ
 *
 * @return ���ڵ���0: ���ܳɹ������������ӵ�socket���
 *            С��0: ����ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_accept(ant_s32 sock, ant_sockaddr *sock_addr);

/**
 * �ر�socket
 *
 * @param sock socket���
 * @return    0: �ɹ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_close(ant_s32 sock);

/**
 * ��ѯsocket�¼�
 *
 * @param ndesc  socket������ֵ
 * @param in ��ѯ�ɶ����ļ����
 * @param out ��ѯ��д���ļ����
 * @param ex ��ѯ����������ļ����
 * @param timeout ��ʱʱ�䣬��λΪ����
 * @return   0: �ɹ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_select(ant_u8 ndesc, 
                       ant_fd_set *in, 
                       ant_fd_set *out,
                       ant_fd_set *ex,
                       ant_timeval *timeout);

/**
 * ��ȡsocket�ϴη����Ĵ���
 *
 * @param sock socket���
 * @param error ���ش����룬ȡֵ�μ�ant_soc_errcode_e����
 * @return    0: �ɹ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_get_last_error(ant_s32 sock, ant_s32 *error);

/**
 * ����socket�ϴη����Ĵ���
 *
 * @param sock socket���
 * @param error ���õĴ����룬ȡֵ�μ�ant_soc_errcode_e����
 * @return    0: �ɹ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_set_last_error(ant_s32 sock, ant_s32 error);

/**
 * ��socket������ת�����ַ���
 *
 * @param errcode �����룬ȡֵ�μ�ant_soc_errcode_e����
 * @return  �ǿ�: ���ش������ַ���
 *         NULL: ʧ��
 */
ant_char* ant_soc_strerr(ant_s32 errcode);

/**
 * �ȴ�socket�ɶ�
 *
 * @param sock socket���
 * @param timeout ��ʱʱ�䣬��λΪ����
 * @return    0: �ɹ���socket�ɶ�
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_wait_for_read(ant_s32 sock, ant_u32 timeout);

/**
 * �ȴ�socket��д
 *
 * @param sock socket���
 * @param timeout ��ʱʱ�䣬��λΪ����
 * @return    0: �ɹ���socket��д
 *         С��0: ʧ�ܣ�ȡֵ�μ�ant_soc_errcode_e����
 */
ant_s32 ant_soc_wait_for_write(ant_s32 sock, ant_u32 timeout);

#ifdef __cplusplus
}
#endif

#endif

