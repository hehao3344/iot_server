#
# Makefile
#
# Makefile for framework
#
TO_BASE             = ..

ifndef MKIT_BASE
MKIT_BASE=$(TO_BASE)
endif

include $(MKIT_BASE)/build/rules/global

CUR_DIR             = framework
SRCS_DIR            = $(MKIT_BASE)/$(CUR_DIR)
TO_OBJS             = $(TO_BASE)/build/linux

EXTERNAL_INC_FLAGS  =

# Componet Release/Debug root path
COM_OBJPATH_STATIC  = $(OBJS_DIR)/$(CUR_DIR)

# Source code directory
FRAMEWORK_SRC_DIR      = src
FRAMEWORK_CORE_DIR     = $(FRAMEWORK_SRC_DIR)/core
FRAMEWORK_DB_DIR       = $(FRAMEWORK_SRC_DIR)/db
FRAMEWORK_JSON_DIR     = $(FRAMEWORK_SRC_DIR)/json
FRAMEWORK_CRYPTO_DIR   = $(FRAMEWORK_SRC_DIR)/crypto
FRAMEWORK_LOG_DIR      = $(FRAMEWORK_SRC_DIR)/log
FRAMEWORK_XML_DIR      = $(FRAMEWORK_SRC_DIR)/xml

####################################################################################
# Component source
#
COM_FRAMEWORK_SRC =  \
                $(FRAMEWORK_CORE_DIR)/os_time.c   \
                $(FRAMEWORK_CORE_DIR)/ini.c  \
                $(FRAMEWORK_CORE_DIR)/rthread.c  \
                $(FRAMEWORK_CORE_DIR)/sock.c     \
                $(FRAMEWORK_CORE_DIR)/sock_core.c \
                $(FRAMEWORK_CORE_DIR)/tcp_sock.c \
                $(FRAMEWORK_DB_DIR)/sqlite3.c \
                $(FRAMEWORK_DB_DIR)/dev_db_mgr.c \
                $(FRAMEWORK_JSON_DIR)/jsonparse.c \
                $(FRAMEWORK_JSON_DIR)/cjson.c \
                $(FRAMEWORK_JSON_DIR)/jsontree.c \
                $(FRAMEWORK_LOG_DIR)/debug_util.c \
                $(FRAMEWORK_LOG_DIR)/msg_util.c \
                $(FRAMEWORK_LOG_DIR)/log.c \
                $(FRAMEWORK_CRYPTO_DIR)/sha1.c \
                $(FRAMEWORK_CRYPTO_DIR)/base64.c \
                $(FRAMEWORK_CRYPTO_DIR)/int_lib.c \
                $(FRAMEWORK_DB_DIR)/id_mgr.c \
                $(FRAMEWORK_XML_DIR)/tinystr.cpp \
                $(FRAMEWORK_XML_DIR)/tinyxml.cpp \
                $(FRAMEWORK_XML_DIR)/tinyxmlerror.cpp \
                $(FRAMEWORK_XML_DIR)/tinyxmlparser.cpp \
                $(FRAMEWORK_XML_DIR)/xml_msg.cpp \
                $(FRAMEWORK_XML_DIR)/xml_api.cpp

# Local include directory
LOCAL_INC           += -Iinclude -I$(FRAMEWORK_SRC_DIR)  $(EXTERNAL_INC_FLAGS)

# Add your Local include directory here
LOCAL_INC           +=

# Add your Local Define flags here
LOCAL_DEFINE        += -DHAVE_CONFIG_H -DSQLITE_OMIT_LOAD_EXTENSION

# CXX include directory
LOCAL_CPPINC        += $(LOCAL_INC)

CFLAGS              += $(LOCAL_INC)     $(LOCAL_DEFINE) -DJSON_FORMAT
CXXFLAGS            += $(LOCAL_CPPINC)  $(LOCAL_DEFINE)

SRCS                += $(COM_FRAMEWORK_SRC)

objects             = $(patsubst %.S,%,$(patsubst %.cpp,%,$(patsubst %.c,%,$(SRCS))))
#objects             = $(patsubst %.c,%,$(SRCS))

target              = framework
target_version      = 1.00
target_libs         =

include $(MKIT_BASE)/build/rules/lib
