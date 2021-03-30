/**
 * @file httpdns.h
 * httpdns API header file.
 *
 * @version   V1.0
 * @date      2019-11-08
 * @copyright Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef HTTPDNS_H
#define HTTPDNS_H

/** @addtogroup aos_httpdns httpdns
 *  Interfaces of httpdns
 *
 *  @{
 */

#include <network/network.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYNC (0)
#define ASYNC (1)

/**
 *
 * type of httpdns result cache
 *
 */
struct dns_cache;
typedef struct dns_cache dns_cache_t;

/******************* httpdns dns operation **********************/

/**
 *
 * Check httpdns is disabled
 *
 * @return 0 on available, 1 on disabled
 */
int httpdns_is_disabled(void);

/**
 *
 * Invoke host resolving action
 *
 * @param[in] host_name host domain
 * @param[in] asyn 1: asynchronized 0: synchronized
 *
 * @return none
 */
void httpdns_resolv_host(char *host_name, int asyn);


/**
 *
 * Invoke host resolving action using local dns
 *
 * @param[in] host_name host domain
 * @param[in] asyn 1: asynchronized 0: synchronized
 *
 * @return none
 */
void httpdns_resolv_host_local(char *host_name, int async);

/**
 *
 * Fetch DNS result from cache
 *
 * @param[in]  host_name host domain
 * @param[out] cache DNS result
 *
 * @return 1 on success, 0 on failure
 */
int httpdns_prefetch(char * host_name, dns_cache_t ** cache);

/**
 *
 * Fetch DNS result from cache with timeout
 *
 * @param[in]  host_name host domain
 * @param[out] cache DNS result
 * @param[in]  timeout fetch timeout in millisecond
 *
 * @return 1 on success, 0 on failure
 */
int httpdns_prefetch_timeout(char * host_name, dns_cache_t ** cache, int ms);

/**
 *
 * Asynchronizd getaddrinfo
 *
 * @param[in]  nodename  descriptive name or address string of the host
 * @param[in]  servname  port number as string
 * @param[in]  hints     structure containing input values that set socktype and protocol
 * @param[out] res       pointer to a pointer where to store the result (set to NULL on failure)
 * @param[in]  timeout   fetch timeout in millisecond
 * @param[in]  local     flag 1: use local dns 0: use remote dns
 *
 * @return 0 on success, non-zero on failure on failure
 */
int getaddrinfo_async(const char *nodename,
                      const char *servname,
                      const struct addrinfo *hints,
                      struct addrinfo **res,
                      int timeout_ms,
                      int local);

/******************* dns cache operation **********************/

/**
 *
 * Get the first ip address from cache
 *
 * @param[in]  cache DNS result
 * @param[out] ipaddr ip address
 *
 * @return 1 on success, 0 on failure
 */
int httpdns_get_ipaddr_from_cache(dns_cache_t * cache, char * ipaddr);

/**
 *
 * Free cache
 *
 * @param[in] cache DNS result
 *
 * @return none
 */
void httpdns_free_cache(dns_cache_t * cache);

/**
 *
 * Rotate ip list in cache
 *
 * @param[in] cache DNS result
 *
 * @return 1 on success, 0 on failure
 */
int httpdns_rotate_cache(dns_cache_t * dns);

#ifdef __cplusplus
}
#endif

/** @} */

#endif

