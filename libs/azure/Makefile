



# Name of the this module
LOCAL_NAME := libs/azure


# Space-separated list of modules (libraries) your module depends upon.
# These should include the toplevel name, e.g. "libs/gps"
LOCAL_MODULE_DEPENDS := libs/azure/platform
                        


# Add includes from other modules we do not wish to link to
LOCAL_API_DEPENDS := \


# include folder
LOCAL_ADD_INCLUDE := \


                    
# `yes` if have submodule, or left empty  or `no`
IS_CONTAIN_SUB_MODULE := yes

## ------------------------------------ ##
## 	Add your custom flags here          ##
## ------------------------------------ ##
MYCFLAGS += 

## ------------------------------------- ##
##	List all your sources here           ##
## ------------------------------------- ##
S_SRC := ${notdir ${wildcard src/*.s}}
C_SRC := ${notdir ${wildcard src/*.c}}

AZURE_PATH = $(SOFT_WORKDIR)/libs/azure/azure-iot-sdk-c
AZURE_LIB_PATH = $(AZURE_PATH)/output/release/lib
AZURE_LIB_NAME = libazureiot_sdk.a

all:
	@echo "========================"
	@echo " start build azure lib "
	@echo "========================"
	@if [ ! -f "$(AZURE_PATH)/src/configs/default_settings.mk.bak" ]; then \
		cp $(AZURE_PATH)/src/configs/default_settings.mk $(AZURE_PATH)/src/configs/default_settings.mk.bak; \
	fi
	cp $(SOFT_WORKDIR)/libs/azure/config/default_settings.mk $(AZURE_PATH)/src/configs/
	cp $(SOFT_WORKDIR)/libs/azure/config/config.Ai_Thinker.gprs_a9 $(AZURE_PATH)/src/configs/
	# make -C $(AZURE_PATH) distclean
	make -C $(AZURE_PATH) SOFT_WORKDIR=${SOFT_WORKDIR}
	rm $(AZURE_PATH)/src/configs/config.Ai_Thinker.gprs_a9
	cp $(AZURE_PATH)/src/configs/default_settings.mk.bak $(AZURE_PATH)/src/configs/default_settings.mk
	@echo "lib $(AZURE_LIB_PATH)/$(AZURE_LIB_NAME)"
	@if [ -f "$(AZURE_LIB_PATH)/$(AZURE_LIB_NAME)" ]; then \
		echo "========================";\
		echo "build azure lib success";\
		echo "========================";\
	else \
		echo "========================";\
		echo "build azure lib fail!!!";\
		echo "========================";\
	fi


## ------------------------------------------------------------------- ##
##  Do Not touch below this line unless you know what you're doing.    ##
## ------------------------------------------------------------------- ##
include ${SOFT_WORKDIR}/platform/compilation/cust_rules.mk
