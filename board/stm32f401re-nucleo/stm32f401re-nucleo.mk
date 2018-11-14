NAME := stm32f401re-nucleo


$(NAME)_TYPE := kernel
MODULE               := 1062
HOST_ARCH            := Cortex-M4
HOST_MCU_FAMILY      := stm32f4xx_cube
SUPPORT_BINS         := no
HOST_MCU_NAME        := STM32F401RET
ENABLE_VFP           := 1

$(NAME)_SOURCES += aos/board.c \
                   aos/soc_init.c

$(NAME)_SOURCES += Src/stm32f4xx_hal_msp.c \
                   Src/main.c \
	           Src/stm32f4xx_it.c 

sal ?= 1
ifeq (1,$(sal))
$(NAME)_COMPONENTS += linkkit/sdk-c/src/services/mdal/sal
module ?= wifi.mk3060
else
GLOBAL_DEFINES += CONFIG_NO_TCPIP
endif
                   
ifeq ($(COMPILER), armcc)
$(NAME)_SOURCES += startup_stm32f401xe_keil.s    
$(NAME)_LINK_FILES := startup_stm32f401xe_keil.o
else ifeq ($(COMPILER), iar)
$(NAME)_SOURCES += startup_stm32f401xe_iar.s  
else
$(NAME)_SOURCES += startup_stm32f401xe.s
endif

GLOBAL_INCLUDES += . \
                   aos/ \
                   Inc/
				   
GLOBAL_CFLAGS += -DSTM32F401xE -DSRAM1_SIZE_MAX=0x18000 -DCENTRALIZE_MAPPING



ifeq ($(COMPILER),armcc)
GLOBAL_LDFLAGS += -L --scatter=board/stm32f401re-nucleo/stm32f401xe.sct
else ifeq ($(COMPILER),iar)
GLOBAL_LDFLAGS += --config board/stm32f401re-nucleo/STM32F401.icf
else
GLOBAL_LDFLAGS += -T board/stm32f401re-nucleo/STM32F401RETx_FLASH.ld
endif



CONFIG_SYSINFO_PRODUCT_MODEL := ALI_AOS_f401-nucleo
CONFIG_SYSINFO_DEVICE_NAME := f401-nucleo

GLOBAL_CFLAGS += -DSYSINFO_PRODUCT_MODEL=\"$(CONFIG_SYSINFO_PRODUCT_MODEL)\"
GLOBAL_CFLAGS += -DSYSINFO_DEVICE_NAME=\"$(CONFIG_SYSINFO_DEVICE_NAME)\"
