



# Name of the this module
LOCAL_NAME := libs/azure/platform


# Space-separated list of modules (libraries) your module depends upon.
# These should include the toplevel name, e.g. "libs/gps"
LOCAL_MODULE_DEPENDS := 
                        


# Add includes from other modules we do not wish to link to
LOCAL_API_DEPENDS := \


# include folder
LOCAL_ADD_INCLUDE := 	include \
						include/std_inc \
						include/api_inc \
						libs/utils/include \
						libs/azure/platform \
						libs/azure/platform/include \
						libs/azure/azure-iot-sdk-c/c-utility/inc \
						libs/azure/azure-iot-sdk-c/c-utility/inc/azure_c_shared_utility \
						libs/azure/azure-iot-sdk-c/c-utility/pal/inc \
						libs/azure/azure-iot-sdk-c/c-utility/pal/generic \
						libs/azure/azure-iot-sdk-c/iothub_client/inc \
						libs/azure/azure-iot-sdk-c/serializer/inc \
						libs/azure/azure-iot-sdk-c/umqtt/inc \
						libs/azure/azure-iot-sdk-c/umqtt/inc/azure_umqtt_c \
						libs/azure/azure-iot-sdk-c/deps/parson 


                    
# `yes` if have submodule, or left empty  or `no`
IS_CONTAIN_SUB_MODULE := no

## ------------------------------------ ##
## 	Add your custom flags here          ##
## ------------------------------------ ##
MYCFLAGS += 

## ------------------------------------- ##
##	List all your sources here           ##
## ------------------------------------- ##
S_SRC := ${notdir ${wildcard src/*.s}}
C_SRC := ${notdir ${wildcard src/*.c}}


## ------------------------------------------------------------------- ##
##  Do Not touch below this line unless you know what you're doing.    ##
## ------------------------------------------------------------------- ##
include ${SOFT_WORKDIR}/platform/compilation/cust_rules.mk
