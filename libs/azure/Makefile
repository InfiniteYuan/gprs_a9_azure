



# Name of the this module
LOCAL_NAME := libs/azure


# Space-separated list of modules (libraries) your module depends upon.
# These should include the toplevel name, e.g. "libs/gps"
LOCAL_MODULE_DEPENDS := libs/azure/platform
                        


# Add includes from other modules we do not wish to link to
LOCAL_API_DEPENDS := \


# include folder
LOCAL_ADD_INCLUDE := 	include \
						include/std_inc \
						include/api_inc \
						libs/azure/platform \
						libs/azure/platform/include \
						libs/azure/src/c-utility/inc \
						libs/azure/src/c-utility/inc/azure_c_shared_utility \
						libs/azure/src/c-utility/inc/azure_c_shared_utility/windowsce \
						libs/azure/src/c-utility/pal/inc \
						libs/azure/src/c-utility/pal/generic \
						libs/azure/src/iothub_client/inc \
						libs/azure/src/serializer/inc \
						libs/azure/src/uamqp/inc \
						libs/azure/src/umqtt/inc \
						libs/azure/src/umqtt/inc/azure_umqtt_c \
						libs/azure/src/deps/parson 

LOCAL_OBJS := 	\
# 				iothub_client/src/iothub_client_ll.o \
# 				iothub_client/src/iothub_client_core_ll.o \
# 				iothub_client/src/iothub_client_ll_uploadtoblob.o \
# 				iothub_client/src/iothub_client_authorization.o \
# 				iothub_client/src/iothub_client_retry_control.o \
# 				iothub_client/src/iothub_client_diagnostic.o \
# 				iothub_client/src/iothub_message.o \
# 				iothub_client/src/iothubtransport.o \
# 				iothub_client/src/iothubtransportmqtt.o \
# 				iothub_client/src/iothubtransport_mqtt_common.o \
# 				iothub_client/src/version.o \
# # 				\
                    
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
# C_SRC := ${notdir ${wildcard src/*.c}}
C_SRC_FILES +=  src/c-utility/pal/tlsio_options.c \
				\
				src/c-utility/src/xlogging.c \
				src/c-utility/src/singlylinkedlist.c \
				src/c-utility/src/buffer.c \
				src/c-utility/src/consolelogger.c \
				src/c-utility/src/constbuffer.c \
				src/c-utility/src/constmap.c \
				src/c-utility/src/crt_abstractions.c \
				src/c-utility/src/doublylinkedlist.c \
				src/c-utility/src/gballoc.c \
				src/c-utility/src/gb_stdio.c \
				src/c-utility/src/gb_time.c \
				src/c-utility/src/hmac.c \
				src/c-utility/src/hmacsha256.c \
				src/c-utility/src/httpapiex.c \
				src/c-utility/src/httpapiexsas.c \
				src/c-utility/src/httpheaders.c \
				src/c-utility/src/map.c \
				src/c-utility/src/optionhandler.c \
				src/c-utility/src/sastoken.c \
				src/c-utility/src/sha1.c \
				src/c-utility/src/sha224.c \
				src/c-utility/src/sha384-512.c \
				src/c-utility/src/strings.c \
				src/c-utility/src/string_tokenizer.c \
				src/c-utility/src/urlencode.c \
				src/c-utility/src/usha.c \
				src/c-utility/src/vector.c \
				src/c-utility/src/xio.c \
				src/c-utility/src/base64.c \
				\
				\
				src/iothub_client/src/iothub_client_ll.c \
				src/iothub_client/src/iothub_client_core_ll.c \
				src/iothub_client/src/iothub_client_ll_uploadtoblob.c \
				src/iothub_client/src/iothub_client_authorization.c \
				src/iothub_client/src/iothub_client_retry_control.c \
				src/iothub_client/src/iothub_client_diagnostic.c \
				src/iothub_client/src/iothub_message.c \
				src/iothub_client/src/iothubtransportmqtt.c \
				src/iothub_client/src/iothubtransport_mqtt_common.c \
				src/iothub_client/src/version.c \
				\
				\
				src/umqtt/src/mqtt_client.c \
				src/umqtt/src/mqtt_codec.c \
				src/umqtt/src/mqtt_message.c \
				\
				\
				\
				src/serializer/src/codefirst.c \
				src/serializer/src/commanddecoder.c \
				src/serializer/src/datamarshaller.c \
				src/serializer/src/datapublisher.c \
				src/serializer/src/dataserializer.c \
				src/serializer/src/iotdevice.c \
				src/serializer/src/jsondecoder.c \
				src/serializer/src/jsonencoder.c \
				src/serializer/src/methodreturn.c \
				src/serializer/src/multitree.c \
				src/serializer/src/schema.c \
				src/serializer/src/schemalib.c \
				src/serializer/src/schemaserializer.c \
				\
				\
				# src/iothub_client/src/iothubtransport.c \
				# src/deps/parson/parson.c \
				# src/serializer/src/agenttypesystem.c \
				# ${wildcard src/umqtt/src/*.c} \
				# ${wildcard src/iothub_client/src/*.c} \
				# ${wildcard src/serializer/src/*.c} \
				# ${wildcard src/deps/parson/*.c}	\
				

# AZURE_PATH = $(SOFT_WORKDIR)/libs/azure/src
# AZURE_LIB_PATH = $(AZURE_PATH)/output/release/lib
# AZURE_LIB_NAME = libazureiot_sdk.a

# all:
# 	@echo "========================"
# 	@echo " start build azure lib "
# 	@echo "========================"
# 	@if [ ! -f "$(AZURE_PATH)/src/configs/default_settings.mk.bak" ]; then \
# 		cp $(AZURE_PATH)/src/configs/default_settings.mk $(AZURE_PATH)/src/configs/default_settings.mk.bak; \
# 	fi
# 	cp $(SOFT_WORKDIR)/libs/azure/config/default_settings.mk $(AZURE_PATH)/src/configs/
# 	cp $(SOFT_WORKDIR)/libs/azure/config/config.Ai_Thinker.gprs_a9 $(AZURE_PATH)/src/configs/
# 	# make -C $(AZURE_PATH) distclean
# 	make -C $(AZURE_PATH) SOFT_WORKDIR=${SOFT_WORKDIR}
# 	rm $(AZURE_PATH)/src/configs/config.Ai_Thinker.gprs_a9
# 	cp $(AZURE_PATH)/src/configs/default_settings.mk.bak $(AZURE_PATH)/src/configs/default_settings.mk
# 	@echo "lib $(AZURE_LIB_PATH)/$(AZURE_LIB_NAME)"
# 	@if [ -f "$(AZURE_LIB_PATH)/$(AZURE_LIB_NAME)" ]; then \
# 		echo "========================";\
# 		echo "build azure lib success";\
# 		echo "========================";\
# 	else \
# 		echo "========================";\
# 		echo "build azure lib fail!!!";\
# 		echo "========================";\
# 	fi


## ------------------------------------------------------------------- ##
##  Do Not touch below this line unless you know what you're doing.    ##
## ------------------------------------------------------------------- ##
include ${SOFT_WORKDIR}/platform/compilation/cust_rules.mk
