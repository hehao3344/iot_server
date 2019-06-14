#
# Makefile
#
# The global Makefile
#

TO_BASE = .

ifndef MKIT_BASE
export MKIT_BASE=$(shell pwd)
endif

include $(MKIT_BASE)/build/rules/global

VERSION = V1
ifneq ($(PROJECT_CFG),)
all: all_targets
.PHONY: all clean distclean cleanlib help config
ifneq ($(filter config, $(MAKECMDGOALS)),)
config: config_project
endif
else
info: help
.PHONY: all clean distclean cleanlib help config
ifneq ($(filter config, $(MAKECMDGOALS)),)
config: config_project
endif
endif

all_targets: pre_build exec_build post_build
	@$(ECHO) "Compile Done!!!"

pre_build:
	@$(ECHO)  "Pre build starting ..."
	@$(MKDIR) $(OUT_REF)

exec_build: libs exec
	@$(ECHO) "Compiling library Done"

exec:
	@$(ECHO) "Creating executable $(OUT_DIR)/linux/webs... "

$(OBJS_DIR)/project/$(PROJECT_CFG)/src/main.o: project/$(PROJECT_CFG)/src/main.cpp
	$(CC) -c $(CFLAGS)  $< -o $@

post_build: $(OBJS_DIR)/project/$(PROJECT_CFG)/src/main.o
	@$(ECHO) "Post build starting ..."
	@$(MKDIR) $(WEBS_BIN_DIR)/linux
	@$(MKDIR) $(OUT_DIR)/linux
	@$(CP) project/$(PROJECT_CFG)/src/libwebsockets/lib/libwebsockets.a $(LIB_DIR)/ -rf
#	@$(CXX) -o $(EXEC_NAME) -lproject -lframework -lwebsockets $(LDFLAGS)
	@$(CXX) -o $(EXEC_NAME) -lproject -lframework -lwebsockets -lssl -lcrypto $(LDFLAGS)
	@$(CROSS_COMPILE)strip $(EXEC_NAME)
#	@$(CP) $(EXEC_NAME)   /share/romfs/usr/bin/webs/linux/

# Clean commands
clean:
	@$(ECHO)    "Cleaning librarys and objects ..."
	@$(RM)      $(DEP_DIR) $(LIB_DIR) $(OBJS_DIR) $(EXEC_NAME)
	@$(ECHO)    "Clean Done !!!"

distclean:
	@$(ECHO)    "Cleaning depend files, objects, librarys, resource ..."
	@$(RM)      $(DEP_DIR) $(LIB_DIR) $(OBJS_DIR) $(EXEC_NAME)
	@$(RM)      $(OUT_DIR)
	@$(ECHO)    "Distclean Done !!!"

cleanlib:
	@ECHO       "Cleaning all librarys..."
	@$(RM)      $(LIB_DIR)
	@$(ECHO)    "Cleanlib Done !!!"

tar:
	@tar zcvf mkit.tar.gz ../mkit

rebuild: distclean  all_targets

help:
	@if [ -f ./build/Config/Config ]; then \
		$(CAT) ./build/Config/Help; \
	else \
		echo "    You haven't specify your project !!!"; \
		echo "    You can type \"make config\" to get more information;"; \
	fi

config_project:
ifeq ($(PROJECT),)
	@echo "You haven't specify the project."
	@echo "For example:"
	@echo "  Type  \"make config PROJECT=Hankx100\" for Hankx100      project"
else
	@if [ -f project/$(PROJECT)/config.$(VERSION) ]; then \
		rm -rf build/config/Config; \
		cp project/$(PROJECT)/config.$(VERSION) build/config/Config; \
		echo "Config PROJECT: [$(PROJECT)] "; \
		echo "       VERSION: [$(VERSION)]      Successful "; \
		echo "You can type \"make\" to compile your project "; \
	else \
		echo "   ERROR: "; \
		echo "   The file project/$(PROJECT)/config.$(VERSION) not exist"; \
	fi
endif

# Fast library compile options
$(LIB_DIR):
	$(MKDIR) $(subst $(MKIT_BASE),.,$(LIB_DIR))

libframework:
	$(MAKE) -C framework
libproject:
	$(MAKE) -C project/$(PROJECT_CFG)

libs: $(LIB_DIR) libframework libproject
	@$(ECHO) "Compile all libs Done!"
