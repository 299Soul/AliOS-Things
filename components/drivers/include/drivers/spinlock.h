/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <stdio.h>
#include <pthread.h>

#include <drivers/u_mode.h>

typedef struct spinlock {
	unsigned int flag;
	pthread_spinlock_t ps;
} spinlock_t;

#define __SPIN_LOCK_UNLOCKED(lock) { .flag = 0 }

#define DEFINE_SPINLOCK(lock) struct spinlock lock = { \
	.flag = 0 \
}

#define spin_lock_init(lock) do { \
	int ret = -1; \
	if ((lock)->flag == 1) \
		break; \
	ret = pthread_spin_init(&((lock)->ps), PTHREAD_PROCESS_PRIVATE); \
	if (ret) { \
		printf("pthread_spin_init failed, ret:%d\r\n", ret); \
	} \
	(lock)->flag = 1; \
} while (0)

//TODO: how to guarantee this procedure is atomic?
#define spin_lock_irqsave(lock, flags) do { \
	(void)flags; \
	if (!(lock)->flag) { \
		spin_lock_init(lock); \
		(lock)->flag = 1; \
	} \
	pthread_spin_lock(&(lock)->ps); \
} while (0)

#define spin_unlock_irqrestore(lock, flags) do { \
	(void)flags; \
	pthread_spin_unlock(&(lock)->ps); \
} while (0)


#define spin_lock(lock) do { \
	pthread_spin_lock(&((lock)->ps)); \
} while (0)

#define spin_unlock(lock) do { \
	pthread_spin_unlock(&((lock)->ps)); \
} while (0)

#define spin_lock_irq(lock) \
unsigned long flags; \
do { \
	pthread_spin_lock(&((lock)->ps)); \
} while(0)

#define spin_unlock_irq(lock) do { \
	pthread_spin_unlock(&((lock)->ps)); \
} while(0)

#define spin_lock_destroy(lock) do { \
	pthread_spin_destroy(&((lock)->ps)); \
} while(0)

typedef spinlock_t raw_spinlock_t;

#define __RAW_SPIN_LOCK_UNLOCKED(lock) { \
	.flag = 0 \
}
#define raw_spin_lock_init(lock) spin_lock_init(lock)

#define raw_spin_lock_irqsave(lock, flags) spin_lock_irqsave(lock, flags)
#define raw_spin_unlock_irqrestore(lock, flags)  spin_unlock_irqrestore(lock, flags)

#define raw_spin_lock_irq(lock) spin_lock_irq(lock)
#define raw_spin_unlock_irq(lock)  spin_unlock_irq(lock)


#endif //_SPINLOCK_H_
