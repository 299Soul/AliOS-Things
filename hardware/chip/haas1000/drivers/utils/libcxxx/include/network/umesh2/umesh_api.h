
/*!
* @filename umesh_api.h
* This is uMesh APIs header file
*
* @copyright  Copyright (C) 2015-2020 Alibaba Group Holding Limited
*/
#ifndef __UMESH_MESH_H__
#define __UMESH_MESH_H__

#include "network/umesh2/mdns/mdns.h"
#include "network/umesh2/utils/list.h"
#include "core/umesh.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SERVICE_NAME_LEN_MAX 24
#define SERVICE_TYPE_LEN_MAX 24
#define AUTH_DATA_MAX_LEN    128

#define UMESH_SEND_BROADCAST NULL

/* User Input: 0x0100 ~ 0x01FF */
#define UMESH_SERVICE_ERR_BASE                       (-0x2000)

#define UMESH_SRV_ERR_NULL_POINTER               (UMESH_SERVICE_ERR_BASE - 0x0001)
#define UMESH_SRV_ERR_MALLOC_FAILED              (UMESH_SERVICE_ERR_BASE - 0x0002)
#define UMESH_SRV_ERR_OUT_OF_BOUNDS              (UMESH_SERVICE_ERR_BASE - 0x0003)
#define UMESH_SRV_NO_ACTION_REQUIRED             (UMESH_SERVICE_ERR_BASE - 0x0004)
#define UMESH_SRV_ERR_NOT_INIT                   (UMESH_SERVICE_ERR_BASE - 0x0005)
#define UMESH_SRV_ERR_MDNS                       (UMESH_SERVICE_ERR_BASE - 0x0006)
#define UMESH_SRV_ERR_MDNS_FORMAT                (UMESH_SERVICE_ERR_BASE - 0x0007)
#define UMESH_SRV_ERR_INIT                       (UMESH_SERVICE_ERR_BASE - 0x0008)
#define UMESH_SRV_ERR_AUTH                       (UMESH_SERVICE_ERR_BASE - 0x0009)
#define UMESH_SRV_ERR_AUTH_REFUSE                (UMESH_SERVICE_ERR_BASE - 0x000E)
#define UMESH_SRV_ERR_NO_AUTH_DATA               (UMESH_SERVICE_ERR_BASE - 0x000A)
#define UMESH_SRV_ERR_CREATE_SOCKET              (UMESH_SERVICE_ERR_BASE - 0x000B)
#define UMESH_SRV_ERR_BIND                       (UMESH_SERVICE_ERR_BASE - 0x000C)
#define UMESH_SRV_ERR_READ_DATA                  (UMESH_SERVICE_ERR_BASE - 0x000D)
#define UMESH_SRV_ERR_AUTH_NO_MATCH              (UMESH_SERVICE_ERR_BASE - 0x000F)
#define UMESH_SRV_ERR_SERVICE_INCOMPLETE         (UMESH_SERVICE_ERR_BASE - 0x0010)
#define UMESH_SRV_ERR_PORT_OCCUPIED              (UMESH_SERVICE_ERR_BASE - 0x0011)

typedef enum {
    NETWORK_UMESH = 0,       /** < umesh LAN */
    NETWORK_ROUTER = 1,      /** < Router LAN */
} network_type_t;
typedef enum {
    PEER_FOUND = 0,       /** < a peer is found */
    PEER_LOST = 1,        /** < a peer is lost */
} peer_state_t;

typedef enum {
    SESSION_MEMBER_JOIN = 0,       /** < session start */
    SESSION_MEMBER_LEAVE = 1,      /** < session end */
} session_state_t;

typedef enum {
    MODE_RELIABLE = 0,    /** < reliable delivery */
    MODE_UNRELIABLE = 1,  /** < unreliable delivery */
} data_mode_t;

typedef enum {
    MODE_AUTH_NONE = 0,
    NODE_AUTH_PWD = 1,
} umesh_auth_mode_t;

typedef enum {
    INVITE_ACCEPT = 0,    /** < accept invitation */
    INVITE_REFUSE = 1,    /** < refuse invitation  */
} invite_resp_t;

typedef struct peer_id_s {
    struct in6_addr ip6;       /** < ip v6 address */
    uint16_t port;             /** < service port */
} peer_id_t;

typedef struct umesh_auth_data_s {
    uint8_t data[AUTH_DATA_MAX_LEN];       /** < ip v6 address */
    uint8_t real_len;
} umesh_auth_data_t;

typedef struct mdns_data_txt txt_item_t;
typedef struct session_s session_t;

typedef struct service_s {
    char srv_name[SERVICE_NAME_LEN_MAX + 1];
    char srv_type[SERVICE_TYPE_LEN_MAX + 1];
    session_t *session;
    uint16_t ttl;
    uint64_t last_update;
    txt_item_t *txt_items;
    peer_id_t id;
    struct list_head linked_list;
    struct list_head linked_list2;
} service_t;

typedef void *net_interface_t;

/**
* Communication changed callback
* @param[in] session         pointer to the session
* @param[in] peer             pointer to the peer  who joined or leave the session
* @param[in] state           join or leave
*
* return void
*/
typedef void (* umesh_session_state_changed_cb)(session_t *session, peer_id_t *peer, session_state_t state,
        void *context);

/**
* Recieve a invitation from a peer
*
* @param[out] session pointer to the session
* @param[out] peer_id  pointer to the inviter
* return 0 to accept the invitation, others to refuse the invitation
*/
typedef int (* umesh_peer_invite_cb)(session_t *session, peer_id_t *peer_id, void *context);
/**
* Data receive callback
*
* @param[out] session pointer to the session
* @param[in] from       source peer
* @param[in] data       pointer to the data
* @param[in] len        data length
* @param[in] user_data  context
*
* return void
*/
typedef void (*umesh_receive_cb)(session_t *session, peer_id_t *from, uint8_t *data, uint16_t len, void *user_data);

typedef struct session_s {
    service_t *self;
    umesh_session_state_changed_cb state_cb;
    umesh_receive_cb  recieve_cb;
    umesh_peer_invite_cb    invite_cb;
    void *recieve_cb_ctx;
    void *state_cb_ctx;
    void *invite_cb_ctx;
    struct list_head peers_list;
    umesh_auth_mode_t auth_type;
    umesh_auth_data_t *auth_data;
    int fd_auth;
    int fd_udp;
    int fd_tcp;
    void *queue;
    void *lock;
    uint8_t *queue_buf;
} session_t;
/**
* Service found callback
*
* @param[out] service pointer to the service
* @param[out] service found or lost service
*/
typedef void (* umesh_service_found_cb)(service_t *service, peer_state_t state, void *context);

typedef struct service_state_s {
    void *network;
    void *mdns;
    struct list_head self_service_list;
    struct list_head found_service_list;
    void *lock;
    void *timer;
    void *leave_semp;
    umesh_service_found_cb   found_cb;
    void *found_cb_ctx;
    uint8_t announced;
    uint8_t stop;
} service_state_t;

/**
* Initialize a service
*
* @param[in] interface  network interface
* @param[in] srv_name   pointer to the service name
* @param[in] srv_type   pointer to the service type
* @param[in] port       service port for communication
*
* @return pointer to the service, NULL indicates that initialization fails
*/
service_t *umesh_service_init(net_interface_t interface, const char *srv_name, const char *srv_type, uint16_t port);
/**
* Deinitialize a service
*
* @param[in] service pointer to the service name
*
* @return 0 is success, others are failure
*/
int umesh_service_deinit(service_t *service);
/**
* Add additional txt to service
*
* @param[in] txt string append to service
*
* @return 0 is success, others are failure
*/
int umesh_service_add_txt(service_t *service, const char *txt);
/**
* Start advertising service
*
* @param[in] service pointer to self service
*
* @return 0 is success, others are failure
*/
int umesh_start_advertise_service(service_t *service);
/**
* Stop advertising service
*
* @param[in] service pointer to self service
*
* @return 0 is success, others are failure
*/
int umesh_stop_advertise_service(service_t *self);

/**
* Start browsing service, this is a asynchronous function
*
* @param[in] service pointer to the service
* @param[out] user_data  context to cb
*
* @return 0 is success, others are failure
*/
int umesh_start_browse_service(service_t *self, umesh_service_found_cb found, void *user_data);
/**
* Stop browsing service
*
* return 0 is success, others are failure
*/
int umesh_stop_browse_service(void);
/**
* Initialize a communication session
*
* @param[in] service         pointer to the service
*
* return  the handle of session
*/
session_t *umesh_session_init(service_t *service);

/**
* deinitialize a communication session
*
* @param[in] session         pointer to the session
*
* return 0 is success, others are failure
*/
int umesh_session_deinit(session_t *session);

/**
* Invite peers to join a session with a specified service
* @param[in] session         pointer to the session
* @param[in] dst             pointer to the peerid
* @param[in] timeout         timeout for establishing the session with the peer
*
* return 0 is success, others are failure
*/
int umesh_invite_peer(session_t *session, peer_id_t *dst, int timeout);

/**
* Delete a peer from a specified session
* @param[in] session         pointer to the session
* @param[in] dst             pointer to the peer service
*
* return 0 is success, others are failure
*/
int umesh_delete_peer(session_t *session, peer_id_t *dst);

/**
* Send data to the peer
*
* @param[in] session pointer to the session
* @param[in] dst     destination service
* @param[in] data    pointer to data
* @param[in] len     data length
* @param[in] mode    send data reliable or unreliable delivery
*
* return 0 is success, others are failure
*/
int umesh_send(session_t *session, peer_id_t *dest, const uint8_t *data,  uint16_t len, data_mode_t mode);
/**
* Register data receive callback
*
* @param[out] session    pointer to the session
* @param[out] cb         point to callback func
* @param[out] user_data  context to cb
*
* return 0 is success, others are failure
*/
int umesh_register_receiver(session_t *session, umesh_receive_cb cb, void *user_data);

/**
* Register session state callback
*
* @param[out] session    pointer to the session
* @param[out] cb         point to callback func
* @param[out] user_data  context to cb
*
* return 0 is success, others are failure
*/
int umesh_register_state(session_t *session, umesh_session_state_changed_cb cb, void *user_data);
/**
* Register session invite callback
*
* @param[out] session    pointer to the session
* @param[out] cb         point to callback func
* @param[out] user_data  context to cb
*
* return 0 is accept the invitation, others are refused
*/
int umesh_register_inviter(session_t *session, umesh_peer_invite_cb cb, void *user_data);

/** @} */
#ifdef __cplusplus
}
#endif
#endif  /* __UMESH_MESH_H__ */
