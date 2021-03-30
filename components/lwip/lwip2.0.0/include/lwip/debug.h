/**
 * @file
 * Debug messages infrastructure
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef LWIP_HDR_DEBUG_H
#define LWIP_HDR_DEBUG_H

#include "lwip/arch.h"

/** lower two bits indicate debug level
 * - 0 all
 * - 1 warning
 * - 2 serious
 * - 3 severe
 */
#define LWIP_DBG_LEVEL_ALL     0x00
#define LWIP_DBG_LEVEL_OFF     LWIP_DBG_LEVEL_ALL /* compatibility define only */
#define LWIP_DBG_LEVEL_WARNING 0x01 /* bad checksums, dropped packets, ... */
#define LWIP_DBG_LEVEL_SERIOUS 0x02 /* memory allocation failures, ... */
#define LWIP_DBG_LEVEL_SEVERE  0x03
#define LWIP_DBG_MASK_LEVEL    0x03

/** flag for LWIP_DEBUGF to enable that debug message */
#define LWIP_DBG_ON            0x80U
/** flag for LWIP_DEBUGF to disable that debug message */
#define LWIP_DBG_OFF           0x00U

/** flag for LWIP_DEBUGF indicating a tracing message (to follow program flow) */
#define LWIP_DBG_TRACE         0x40U
/** flag for LWIP_DEBUGF indicating a state debug message (to follow module states) */
#define LWIP_DBG_STATE         0x20U
/** flag for LWIP_DEBUGF indicating newly added code, not thoroughly tested yet */
#define LWIP_DBG_FRESH         0x10U
/** flag for LWIP_DEBUGF to halt after printing this debug message */
#define LWIP_DBG_HALT          0x08U

/**
 * LWIP_NOASSERT: Disable LWIP_ASSERT checks.
 * -- To disable assertions define LWIP_NOASSERT in arch/cc.h.
 */
#ifndef LWIP_NOASSERT
#define LWIP_ASSERT(message, assertion) do { if (!(assertion)) { \
  LWIP_PLATFORM_ASSERT(message); }} while(0)
#ifndef LWIP_PLATFORM_ASSERT
#error "If you want to use LWIP_ASSERT, LWIP_PLATFORM_ASSERT(message) needs to be defined in your arch/cc.h"
#endif
#else  /* LWIP_NOASSERT */
#define LWIP_ASSERT(message, assertion)
#endif /* LWIP_NOASSERT */

/** if "expression" isn't true, then print "message" and execute "handler" expression */
#ifndef LWIP_ERROR
#ifndef LWIP_NOASSERT
#define LWIP_PLATFORM_ERROR(message) LWIP_PLATFORM_ASSERT(message)
#elif defined LWIP_DEBUG
#define LWIP_PLATFORM_ERROR(message) LWIP_PLATFORM_DIAG((message))
#else
#define LWIP_PLATFORM_ERROR(message)
#endif

#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
  LWIP_PLATFORM_ERROR(message); handler;}} while(0)
#endif /* LWIP_ERROR */

#ifdef LWIP_DEBUG
#ifndef LWIP_PLATFORM_DIAG
#error "If you want to use LWIP_DEBUG, LWIP_PLATFORM_DIAG(message) needs to be defined in your arch/cc.h"
#endif
/** print debug message only if debug message type is enabled...
 *  AND is of correct type AND is at least LWIP_DBG_LEVEL
 */
#define LWIP_DEBUGF(debug, message) do { \
                               if ( \
                                   ((debug) & LWIP_DBG_ON) && \
                                   ((debug) & LWIP_DBG_TYPES_ON) && \
                                   ((s16_t)((debug) & LWIP_DBG_MASK_LEVEL) >= LWIP_DBG_MIN_LEVEL)) { \
                                 LWIP_PLATFORM_DIAG(message); \
                                 if ((debug) & LWIP_DBG_HALT) { \
                                   while(1); \
                                 } \
                               } \
                             } while(0)

#ifdef WITH_LWIP_PKTPRINT
typedef struct pkt_stats_data {
    uint64_t rx_bytes;
    uint64_t rx_packets;
    uint64_t tx_bytes;
    uint64_t tx_packets;
    uint64_t rx_tcp_bytes;
    uint64_t rx_tcp_packets;
    uint64_t rx_udp_bytes;
    uint64_t rx_udp_packets;
    uint64_t rx_other_bytes;
    uint64_t rx_other_packets;
    uint64_t tx_tcp_bytes;
    uint64_t tx_tcp_packets;
    uint64_t tx_udp_bytes;
    uint64_t tx_udp_packets;
    uint64_t tx_other_bytes;
    uint64_t tx_other_packets;
} pkt_stats_data;

void lwip_pkt_stats(pkt_stats_data *data);

void lwip_pkt_print(char* note_ptr, void *pbuf, void* netif);
#define LWIP_PKTDEBUGF(note_ptr, pbuf, netif) do { \
                               s16_t debug = PKTPRINT_DEBUG; \
                               if ( \
                                   ((debug) & LWIP_DBG_ON) && \
                                   ((debug) & LWIP_DBG_TYPES_ON) && \
                                   ((s16_t)((debug) & LWIP_DBG_MASK_LEVEL) >= LWIP_DBG_MIN_LEVEL)) { \
                                 lwip_pkt_print(note_ptr, pbuf, netif); \
                                 if ((debug) & LWIP_DBG_HALT) { \
                                   while(1); \
                                 } \
                               } \
                             } while(0)

#else
#define LWIP_PKTDEBUGF(note_ptr, pbuf, netif)
#endif /* WITH_LWIP_PKTPRINT */
#else  /* LWIP_DEBUG */
#define LWIP_DEBUGF(debug, message)
#define LWIP_PKTDEBUGF(note_ptr, pbuf, netif)
#endif /* LWIP_DEBUG */

#endif /* LWIP_HDR_DEBUG_H */

