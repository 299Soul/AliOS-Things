NAME := ota_demo

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.2
$(NAME)_SUMMARY := OTA demo for developers

OTA_DEMO_CONFIG_TEST_LOOP ?= n
loop ?= $(OTA_DEMO_CONFIG_TEST_LOOP)

$(NAME)_SOURCES := otaappdemo.c maintask.c
$(NAME)_COMPONENTS := netmgr ota libiot_mqtt cjson cli
GLOBAL_DEFINES += CLI_CONFIG_STACK_SIZE=3072
ifeq ($(OTA_DEMO_CONFIG_USE_LWIP),y)
$(NAME)_COMPONENTS  += lwip
endif

ifeq ($(loop),1)
$(NAME)_DEFINES     += TEST_LOOP
endif

#ifeq ($(strip $(CONFIG_SYSINFO_DEVICE_NAME)), $(filter $(CONFIG_SYSINFO_DEVICE_NAME), developerkit))
#AOS_DEVELOPERKIT_ENABLE_OTA := 1
#endif

$(NAME)_INCLUDES += ./
