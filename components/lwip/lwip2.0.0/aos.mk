NAME := lwip

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION := 1.0.0
$(NAME)_SUMMARY := lightweight tcp/ip stack

ifneq ($(no_with_lwip),1)
GLOBAL_DEFINES += WITH_LWIP
with_lwip := 1
endif

ifneq ($(use_private_lwip),1)
include kernel/network/lwip/Filelists.mk

GLOBAL_INCLUDES += include port/include

GLOBAL_DEFINES += CONFIG_NET_LWIP

$(NAME)_INCLUDES += port/include

$(NAME)_SOURCES := $(COREFILES)
$(NAME)_SOURCES += $(CORE4FILES)
$(NAME)_SOURCES += $(CORE6FILES)
$(NAME)_SOURCES += $(APIFILES)
$(NAME)_SOURCES += $(NETIFFILES)

ifeq (y,$(AF_PACKET_ENABLED))
#define LWIP_PACKET  1
$(NAME)_SOURCES += core/af_packet.c
endif

ifeq (y,$(SNMP_ENABLED))
$(NAME)_SOURCES += $(SNMPFILES)
endif

ifeq (y,$(HTTPD_ENABLED))
$(NAME)_SOURCES += $(HTTPDFILES)
endif

ifeq (y,$(IPERF_ENABLED))
GLOBAL_DEFINES += WITH_LWIP_IPERF
$(NAME)_SOURCES += $(IPERFFILES)
endif

ifeq (y,$(SNTP_ENABLED))
$(NAME)_SOURCES += $(SNTPFILES)
endif

ifeq (y,$(MDNS_ENABLED))
$(NAME)_SOURCES += $(MDNSFILES)
endif

ifeq (y,$(NETBIOSNS_ENABLED))
$(NAME)_SOURCES += $(NETBIOSNSFILES)
endif


ifeq (y,$(TFTP_ENABLED))
use_lwip_tftp ?= 0
ifeq ($(use_lwip_tftp), 1)
GLOBAL_DEFINES += WITH_LWIP_TFTP

ifeq ($(TFTP_FS), y)

ifneq (,$(filter vfs,$(COMPONENTS)))
GLOBAL_DEFINES += WITH_LWIP_TFTP_FS
else
$(warning ### miss TFTP_FS as no vfs supports)
endif

endif

$(NAME)_SOURCES += $(TFTPFILES)
endif
endif

ifeq (y,$(TELNETD_ENABLED))
GLOBAL_DEFINES += WITH_LWIP_TELNETD
$(NAME)_SOURCES += $(TELNETDFILES)
endif

ifeq (y,$(DHCPD_ENABLED))
$(NAME)_SOURCES += $(DHCPDFILES)
endif

ifeq (y,$(PING_ENABLED))
GLOBAL_DEFINES += WITH_LWIP_PING
$(NAME)_SOURCES += $(PINGFILES)
endif

ifeq (y,$(SENDFILE_ENABLED))
GLOBAL_DEFINES += WITH_LWIP_SENDFILE
$(NAME)_SOURCES += $(SENDFILEFILES)
endif

ifeq (y,$(IFCONFIG_ENABLED))
GLOBAL_DEFINES += WITH_LWIP_IFCONFIG
$(NAME)_SOURCES += $(IFCONFIGFILES)
endif

ifeq (y,$(DNSCLI_ENABLED))
GLOBAL_DEFINES += WITH_LWIP_DNSCLI
$(NAME)_SOURCES += $(DNSCLIFILES)
endif

ifeq (y,$(ARPCLI_ENABLED))
GLOBAL_DEFINES += WITH_LWIP_ARPCLI
$(NAME)_SOURCES += $(ARPCLIFILES)
endif

ifeq (y,$(LSFDCLI_ENABLED))
GLOBAL_DEFINES += WITH_LWIP_LSFDCLI
$(NAME)_SOURCES += $(LSFDCLIFILES)
endif

$(NAME)_SOURCES += port/sys_arch.c

endif
