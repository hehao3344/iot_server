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
PROJECT_HTTPD_DIR     = $(PROJECT_SRC_DIR)/httpd
PROJECT_UDP_SVR_DIR   = $(PROJECT_SRC_DIR)/udp_server
PROJECT_TCP_SVR_DIR   = $(PROJECT_SRC_DIR)/tcp_server
PROJECT_HTTPS_C_DIR   = $(PROJECT_SRC_DIR)/https_client
PROJECT_MSGH_DIR      = $(PROJECT_SRC_DIR)/msg_handle

####################################################################################
# Component source
####################################################################################
COM_PROJECT_SRC =  \
                $(PROJECT_HTTPD_DIR)/action.c           \
                $(PROJECT_HTTPD_DIR)/alloc.c            \
                $(PROJECT_HTTPD_DIR)/auth.c             \
                $(PROJECT_HTTPD_DIR)/cgi.c              \
                $(PROJECT_HTTPD_DIR)/crypt.c            \
                $(PROJECT_HTTPD_DIR)/file.c             \
                $(PROJECT_HTTPD_DIR)/fs.c               \
                $(PROJECT_HTTPD_DIR)/gbk_utf8.c         \
                $(PROJECT_HTTPD_DIR)/goahead.c          \
                $(PROJECT_HTTPD_DIR)/http.c             \
                $(PROJECT_HTTPD_DIR)/js.c               \
                $(PROJECT_HTTPD_DIR)/jst.c              \
                $(PROJECT_HTTPD_DIR)/options.c          \
                $(PROJECT_HTTPD_DIR)/osdep.c            \
                $(PROJECT_HTTPD_DIR)/rom.c              \
                $(PROJECT_HTTPD_DIR)/route.c            \
                $(PROJECT_HTTPD_DIR)/runtime.c          \
                $(PROJECT_HTTPD_DIR)/socket.c           \
                $(PROJECT_HTTPD_DIR)/time.c             \
                $(PROJECT_HTTPD_DIR)/upload.c           \
                $(PROJECT_HTTPD_DIR)/goahead_apps.c     \
                $(PROJECT_DEVMGR_DIR)/hash_table.c \
                $(PROJECT_DEVMGR_DIR)/hash_table_itr.c \
                $(PROJECT_DEVMGR_DIR)/hash_value.c \
                $(PROJECT_DEVMGR_DIR)/dev_param.c \
                $(PROJECT_DEVMGR_DIR)/clt_param.c \
                $(PROJECT_DEVMGR_DIR)/dev_mgr.c \
                $(PROJECT_DEVMGR_DIR)/clt_mgr.c \
                $(PROJECT_DEVMGR_DIR)/ws.c \
                $(PROJECT_MSGH_DIR)/json_msg_handle.c \
                $(PROJECT_MSGH_DIR)/json_msg_clt.c \
                $(PROJECT_DEVMGR_DIR)/common_tbl.c \
                $(PROJECT_HTTPS_C_DIR)/https_client.c \
                $(PROJECT_HTTPS_C_DIR)/openid.c \
                $(PROJECT_SRC_DIR)/main.cpp

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
