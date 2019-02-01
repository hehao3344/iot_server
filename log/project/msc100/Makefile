#
# Makefile
#
# Makefile for
#
TO_BASE             = ../..

ifndef MKIT_BASE
MKIT_BASE=$(TO_BASE)
endif

include $(MKIT_BASE)/build/rules/global

CUR_DIR             = project/$(PROJECT_CFG)
SRCS_DIR            = $(MKIT_BASE)/$(CUR_DIR)
TO_OBJS             = $(TO_BASE)/build/linux

EXTERNAL_INC_FLAGS  =

# Componet Release/Debug root path
COM_OBJPATH_STATIC  = $(OBJS_DIR)/$(CUR_DIR)

# Source code directory
PROJECT_SRC_DIR       = src
PROJECT_DEVMGR_DIR    = $(PROJECT_SRC_DIR)/device_manage
PROJECT_UDP_SVR_DIR   = $(PROJECT_SRC_DIR)/udp_server
PROJECT_TCP_SVR_DIR   = $(PROJECT_SRC_DIR)/tcp_server
PROJECT_MSGH_DIR      = $(PROJECT_SRC_DIR)/msg_handle
PROJECT_LOG_DIR       = $(PROJECT_SRC_DIR)/log

####################################################################################
# Component source
####################################################################################
COM_PROJECT_SRC =  \
                $(PROJECT_LOG_DIR)/debug_util.c \
                $(PROJECT_LOG_DIR)/msg_util.c \
                $(PROJECT_LOG_DIR)/log.c

# Local include directory
LOCAL_INC           += -Iinclude -I$(PROJECT_SRC_DIR)  $(EXTERNAL_INC_FLAGS) -I$(LINUX_KERNEL)/drivers/char -I$(LINUX_KERNEL)/include

# Add your Local include directory here
LOCAL_INC           +=

# Add your Local Define flags here
LOCAL_DEFINE        += -D_GNU_SOURCE

# CXX include directory
LOCAL_CPPINC        += $(LOCAL_INC)

CFLAGS              += $(LOCAL_INC)     $(LOCAL_DEFINE)
CXXFLAGS            += $(LOCAL_CPPINC)  $(LOCAL_DEFINE)
SRCS                += $(COM_PROJECT_SRC)

objects             = $(patsubst %.S,%,$(patsubst %.cpp,%,$(patsubst %.c,%,$(SRCS))))

target              = project
target_version      = 1.00
target_libs         =

include $(MKIT_BASE)/build/rules/lib
