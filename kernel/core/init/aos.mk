NAME := kernel_init

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION    := 1.0.2
$(NAME)_SUMMARY    := kernel init unit

$(NAME)_SOURCES := aos_init.c

GLOBAL_INCLUDES += include
