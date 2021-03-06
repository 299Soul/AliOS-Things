##
 # Copyright (C) 2017 C-SKY Microsystems Co., All rights reserved.
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #   http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
##

L_PATH := $(call cur-dir)

include $(DEFINE_LOCAL)

L_MODULE := libmesh_node

L_CFLAGS += -Wall

L_INCS += $(L_PATH)

ifeq ($(CONFIG_BT_MESH_MODULE_NODE),y)
       L_SRCS += mesh_node.c
endif


ifeq ($(CONFIG_BT_MESH), y)
include $(BUILD_MODULE)
endif
