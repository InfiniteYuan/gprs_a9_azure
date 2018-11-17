DEFAULT_BLD             := src/configs/config.Ai_Thinker.gprs_a9
POST_FINAL_OUT_HOOK     := Post_Distro
SUBDIRS                 := directory-not-exist-actually

ifeq (Darwin,$(shell uname))
POST_FINAL_OUT_HOOK     :=
endif

FEATURE_MQTT_SHADOW         ?= $(FEATURE_MQTT_COMM_ENABLED)
FEATURE_COAP_DTLS_SUPPORT   ?= $(FEATURE_COAP_COMM_ENABLED)
FEATURE_MQTT_ID2_AUTH       ?= n
FEATURE_MQTT_ID2_CRYPTO     ?= n
FEATURE_OTA_ENABLED         ?= y
FEATURE_OTA_FETCH_CHANNEL   ?= HTTP
FEATURE_OTA_SIGNAL_CHANNEL  ?= MQTT
FEATURE_MQTT_ID2_ENV        ?= online
FEATURE_MQTT_COMM_ENABLED   ?= n
FEATURE_SERVICE_COTA_ENABLED   ?= n

CONFIG_LIB_EXPORT           ?= static

# gateway & subdevice
FEATURE_SUBDEVICE_STATUS    ?= gateway   
# MQTT & CLOUD_CONN
FEATURE_SUBDEVICE_CHANNEL   ?= MQTT 

FEATURE_CMP_VIA_MQTT_DIRECT ?= y
# MQTT & COAP & HTTP
FEATURE_CMP_VIA_CLOUD_CONN  ?= MQTT

FEATURE_SUPPORT_PRODUCT_SECRET ?= n

#CFLAGS  += -DFORCE_SSL_VERIFY

