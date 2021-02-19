/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#ifndef __UVOICE_EVENT_H__
#define __UVOICE_EVENT_H__


#define UVOICE_EV_PLAYER				0x0111
#define UVOICE_CODE_PLAYER_STATE		1
#define UVOICE_CODE_PALYER_CACHE_CPLT	2
#define UVOICE_CODE_PALYER_DLOAD_CPLT	3

#define UVOICE_EV_RECORDER				0x0112
#define UVOICE_CODE_RECORDER_STATE		1

#define UVOICE_EV_SW					0x0113
#define UVOICE_CODE_HEADPHONE			1
#define UVOICE_CODE_HEADSET				2

#define UVOICE_EV_ST					0x0114
#define UVOICE_CODE_VAD_START			1
#define UVOICE_CODE_VAD_END				2
#define UVOICE_CODE_VOICE_WAKEUP		3

#define UVOICE_EV_ASR_RESULT			0x0115

#ifdef UVOICE_EVENT_BY_BUFFER_QUEUE
typedef struct {
	uint16_t type;
	uint16_t code;
	int value;
} uvoice_event_t;
#elif defined(UVOICE_EVENT_BY_MESSAGE_QUEUE)
typedef struct {
	long msgtype;
	uint16_t type;
	uint16_t code;
	int value;
} uvoice_event_t;
#else
typedef input_event_t uvoice_event_t;
#endif

typedef void (*uvoice_event_cb)(uvoice_event_t *event, void *data);
int uvoice_event_post(uint16_t type, uint16_t code, int value);
int uvoice_event_register(uint16_t type, uvoice_event_cb cb, void *data);
int uvoice_event_unregister(uint16_t type, uvoice_event_cb cb, void *data);


#endif /* __UVOICE_EVENT_H__ */

