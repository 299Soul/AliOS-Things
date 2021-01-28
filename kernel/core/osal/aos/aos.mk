NAME := osal_aos

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION    := 1.0.0
$(NAME)_SUMMARY    := os interface adaptation layer

#default gcc
ifeq ($(COMPILER),)
$(NAME)_CFLAGS += -Wall
else ifeq ($(COMPILER),gcc)
$(NAME)_CFLAGS += -Wall
endif

ifeq ($(HOST_ARCH),ARM968E-S)
$(NAME)_CFLAGS += -marm
endif

osal            ?= rhino
$(NAME)_SOURCES += common.c

ifeq ($(osal),rhino_sxr)
GLOBAL_DEFINES  += OSAL_SXR
$(NAME)_SOURCES += ext/sxr.c
endif

ifeq ($(osal),rhino_sxr_mutios)
GLOBAL_DEFINES  += OSAL_SXR_MUTIOS
$(NAME)_SOURCES += ext/rhino_rda8955.c
endif

ifeq ($(osal),rhino)
GLOBAL_DEFINES     += OSAL_RHINO AOS_HAL
$(NAME)_COMPONENTS += rhino debug

$(NAME)_SOURCES += rhino.c

ifeq ($(AOS_COMP_KV),y)
$(NAME)_SOURCES += kv.c
endif

ifeq ($(AOS_COMP_MBMASTER),y)
$(NAME)_SOURCES += mbmaster.c
endif

ifeq ($(AOS_COMP_CANOPEN),y)
$(NAME)_SOURCES += canopen.c
endif

ifeq ($(AOS_COMP_CLI),y)
$(NAME)_SOURCES += cli.c
endif

ifeq ($(AOS_COMP_VFS),y)
$(NAME)_SOURCES += vfs.c
endif

ifeq ($(AOS_COMP_PWRMGMT),y)
$(NAME)_SOURCES += pwrmgmt.c
endif

endif

