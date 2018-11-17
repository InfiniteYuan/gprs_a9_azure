#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_mqtt.h"
#include "api_network.h"
#include "api_socket.h"
#include "api_info.h"
#include "demo_mqtt.h"


const char* ca_crt = "-----BEGIN CERTIFICATE-----\n\
MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n\
RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n\
VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n\
DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n\
ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n\
VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n\
mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n\
IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n\
mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n\
XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n\
dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n\
jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n\
BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n\
DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n\
9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n\
jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n\
Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n\
ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n\
R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n\
-----END CERTIFICATE-----";

const char* client_crt = "-----BEGIN CERTIFICATE-----\n\
MIIDpDCCAowCAQEwDQYJKoZIhvcNAQEFBQAwgZExCzAJBgNVBAYTAkNOMRIwEAYD\n\
VQQIDAlHVUFOR0RPTkcxETAPBgNVBAcMCFNIRU5aSEVOMRQwEgYDVQQKDAtORVVD\n\
UkFDS19DQTELMAkGA1UECwwCQ0ExFDASBgNVBAMMC25ldWNyYWNrX2NhMSIwIAYJ\n\
KoZIhvcNAQkBFhNjemQ2NjY2NjZAZ21haWwuY29tMB4XDTE4MDExODAyMDYzN1oX\n\
DTE5MDExODAyMDYzN1owgZ0xCzAJBgNVBAYTAkNOMRIwEAYDVQQIDAlHVUFOR0RP\n\
TkcxETAPBgNVBAcMCFNIRU5aSEVOMRgwFgYDVQQKDA9ORVVDUkFDS19DTElFTlQx\n\
DzANBgNVBAsMBkNMSUVOVDEYMBYGA1UEAwwPbmV1Y3JhY2tfY2xpZW50MSIwIAYJ\n\
KoZIhvcNAQkBFhNjemQ2NjY2NjZAZ21haWwuY29tMIIBIjANBgkqhkiG9w0BAQEF\n\
AAOCAQ8AMIIBCgKCAQEA3YCUbkQE/91KIY75icVpFJFyzmDSg6aW2t9IoyFkoD9A\n\
4f+U4Sgw9eEWG+fsPlxYNaUD4vKfcUTMYUnUFxyXMha3d29SkIORUaDLHQ9uJTvw\n\
pfT6bEOtDfYLT2Y9SqwOU7MpmClobX1agamxOnGJ40vsUxGJhz9FAeXRp1b6QTop\n\
q2l1M9U+jbjN46V0Z7JQ4QLpDcBNLxsjOlVVeaRsPmlPHGQh5GCKuTpnmU3Hajhz\n\
k8m4DT+M3ZP8faeE7x8vUTvLuBj5J3wdEBmjGNNEkWgQsDCP+Rt0Rk5CZQRKiayF\n\
45cyivCSf7SmudpFafi4b+VTSfwKKg/YFZEYh2jP9QIDAQABMA0GCSqGSIb3DQEB\n\
BQUAA4IBAQAykMvzSybPpuwOpQgVe/hVNmPfAE7FdkomNTjlhp7oTtwa9FM/aktU\n\
S3IbPTyvcYNmpMaVKpF6OWttfKRwJMeLtSxZTFVlKDyaVS7qE1dYzQkNYC9BUe1x\n\
ZQyhvGq5/H4321avOQcI69hfWCKF6LOsYv/Y4VscAPk+cfn/kH1AMzyJEeo/PzmD\n\
/RETj/WFNb3aiDOhjg2w3KYsO1oRYe8rmeniIT5Ki7hfuPig57KNNcsOeigirhhw\n\
iUIwmt0qlUyNFhC6eYzj0suB8jzPflxfxalOE8aby3JOlZjw1dLo3sLKBQr+oTGM\n\
i4p5Nqr1zFeWVTozqGwCtKz20UIbZdrW\n\
-----END CERTIFICATE-----";

const char* client_key = "-----BEGIN RSA PRIVATE KEY-----\n\
MIIEogIBAAKCAQEA3YCUbkQE/91KIY75icVpFJFyzmDSg6aW2t9IoyFkoD9A4f+U\n\
4Sgw9eEWG+fsPlxYNaUD4vKfcUTMYUnUFxyXMha3d29SkIORUaDLHQ9uJTvwpfT6\n\
bEOtDfYLT2Y9SqwOU7MpmClobX1agamxOnGJ40vsUxGJhz9FAeXRp1b6QTopq2l1\n\
M9U+jbjN46V0Z7JQ4QLpDcBNLxsjOlVVeaRsPmlPHGQh5GCKuTpnmU3Hajhzk8m4\n\
DT+M3ZP8faeE7x8vUTvLuBj5J3wdEBmjGNNEkWgQsDCP+Rt0Rk5CZQRKiayF45cy\n\
ivCSf7SmudpFafi4b+VTSfwKKg/YFZEYh2jP9QIDAQABAoIBABhuv3CuRhLnGO88\n\
sx/US7P+GzjSi0YEUS2RZZTA8TWP5JtJMuXnJgdpLaQkX/W8/+ni6tvUwWugGB8/\n\
IoruF2tqABDHkcbpDFWBtAXfltm6k9/qdtkMWgFpO3QlpesfsXDgGV10a9Vv91TI\n\
IAl/TqNo04eSLqZhfz4nWInp2S7qg+kTQ03baJR9/Nv2sitnMbs8ODp3ITRdDP0W\n\
/shI5x6hxMQ/t60G7cS3doDCRPPh7qcm3Wy7qLMu49zsNy+5NdrRUs5M2YIt2gYZ\n\
Lb+l/vuzi6r/SiJkaj6eWClgXNuDfIJgVxcydSrqTyNuPFrmcsy36j7ER5zVWe3P\n\
XWXuYIECgYEA+W+5nXFF9vbo3UawiB7t2b8jfIDIg5MsDn1c97TRTTYg8U2qzh73\n\
NeXUwRCUQ53QSm/c0Y64HKrpnwCY0z0Mf3evf+xPd5ex9yPUJ+lDHngxuxV2QHNp\n\
txH9X/1aH85G6hbG8FwqxdX0vA8+pbpdJfX+m62hGhP5L2HkMB7ztVUCgYEA41Su\n\
qScAOzvH7JPoIls0Zp1b/YHDxxPML9L2UR3UGTSotfwtTqUzHxTeC/Lmkf5g6uGm\n\
RD8Z/zx9SGyojzSgWoDwR7OTr2uey6g8a8B7Vs1NurBm6JaRGeoIwY04IQiDmj8q\n\
yva64R5Y+CsAfMOWaZqP4tNnWrXohj1HlYgEsCECgYBGkGDGNYFBeGAwGQtftsA6\n\
1WPImPYgosXX44clXDXJzgJLwVkMrp3o1Xf2liYbas2+AR9K+UEEeyVUIesZ9y9N\n\
8CHp4oQopPVnoO8ny0l9mSsYbd3iJcRGUgSH3zhbvxM4rfDhlnFTPk/Z08Y5RBuN\n\
3aWFjP0CT5wy11Kiel8fcQKBgCSAZWcdrwjTyX8wJJbXpCtR2wJygpgFksSfEBPo\n\
QTUm4mwOKMw29GVqOVkb0G/is3cjckoFA8yaztcBM3ODFLhTecFZkT4afq3evozi\n\
pCtE3dU7G9r+yh+qF2vUnA2Zy1/KAroCMfGRqOF4LiTaVt1wBuh531+OU96yGjtl\n\
UrNhAoGAM9DCdo8Nh5B6F9NRI+HldKzTeM7+r0sdNNIeCUsC0ZRjPMEOOicWsAN+\n\
lXWLozguJdHoMGwkCA7GcFA1R23XlLbYFzeZ/9PhmIFpLbE9peSn51l3eGN2nScx\n\
gL/UvvJdYSZP+xBDC79I1WGRmwB0tB672ApBq7P0qP9W7Mf7JZ8=\n\
-----END RSA PRIVATE KEY-----";


#define MAIN_TASK_STACK_SIZE    (2048 * 2)
#define MAIN_TASK_PRIORITY      0
#define MAIN_TASK_NAME          "Main Test Task"

#define SECOND_TASK_STACK_SIZE    (2048 * 2)
#define SECOND_TASK_PRIORITY      1
#define SECOND_TASK_NAME          "MQTT Test Task"


char willMsg[50] = "GPRS 123456789012345 disconnected!";
uint8_t imei[16] = "";

static HANDLE mainTaskHandle = NULL;
static HANDLE secondTaskHandle = NULL;

static HANDLE semMqttStart = NULL;


typedef enum{
    MQTT_EVENT_CONNECTED = 0,
    MQTT_EVENT_DISCONNECTED ,
    MQTT_EVENT_MAX
}MQTT_Event_ID_t;

typedef struct {
    MQTT_Event_ID_t id;
    MQTT_Client_t* client;
}MQTT_Event_t;

typedef enum{
    MQTT_STATUS_DISCONNECTED = 0,
    MQTT_STATUS_CONNECTED       ,
    MQTT_STATUS_MAX
}MQTT_Status_t;

MQTT_Status_t mqttStatus = MQTT_STATUS_DISCONNECTED;


static void EventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_NO_SIMCARD:
            Trace(1,"!!NO SIM CARD%d!!!!",pEvent->param1);
            break;

        case API_EVENT_ID_SYSTEM_READY:
            Trace(1,"system initialize complete");
            break;

        case API_EVENT_ID_NETWORK_REGISTERED_HOME:
        case API_EVENT_ID_NETWORK_REGISTERED_ROAMING:
        {
            uint8_t status;
            Trace(2,"network register success");
            bool ret = Network_GetAttachStatus(&status);
            if(!ret)
                Trace(1,"get attach staus fail");
            Trace(1,"attach status:%d",status);
            if(status == 0)
            {
                ret = Network_StartAttach();
                if(!ret)
                {
                    Trace(1,"network attach fail");
                }
            }
            else
            {
                Network_PDP_Context_t context = {
                    .apn        ="cmnet",
                    .userName   = ""    ,
                    .userPasswd = ""
                };
                Network_StartActive(context);
            }

            break;
        }
        case API_EVENT_ID_NETWORK_ATTACHED:
            Trace(1,"network attach success");
            Network_PDP_Context_t context = {
                .apn        ="cmnet",
                .userName   = ""    ,
                .userPasswd = ""
            };
            Network_StartActive(context);
            break;

        case API_EVENT_ID_NETWORK_ACTIVATED:
            Trace(1,"network activate success.."); 
            OS_ReleaseSemaphore(semMqttStart);
            break;
        
        case API_EVENT_ID_SOCKET_CONNECTED:
            Trace(1,"socket connected");
            break;
        
        case API_EVENT_ID_SOCKET_CLOSED:
            Trace(1,"socket closed");
                      
            break;

        case API_EVENT_ID_SIGNAL_QUALITY:
            Trace(1,"CSQ:%d",pEvent->param1);
            break;

        default:
            break;
    }
}

void OnMqttReceived(void* arg, const char* topic, uint32_t payloadLen)
{
    Trace(1,"MQTT received publish data request, topic:%s, payload length:%d",topic,payloadLen);
}

void OnMqttReceiedData(void* arg, const uint8_t* data, uint16_t len, MQTT_Flags_t flags)
{
    Trace(1,"MQTT recieved publish data,  length:%d,data:%s",len,data);
    if(flags == MQTT_FLAG_DATA_LAST)
        Trace(1,"MQTT data is last frame");
}

 void OnMqttSubscribed(void* arg, MQTT_Error_t err)
 {
     if(err != MQTT_ERROR_NONE)
        Trace(1,"MQTT subscribe fail,error code:%d",err);
     else
        Trace(1,"MQTT subscribe success,topic:%s",(const char*)arg);
 }

void OnMqttConnection(MQTT_Client_t *client, void *arg, MQTT_Connection_Status_t status)
{
    Trace(1,"MQTT connection status:%d",status);
    MQTT_Event_t* event = (MQTT_Event_t*)OS_Malloc(sizeof(MQTT_Event_t));
    if(!event)
    {
        Trace(1,"MQTT no memory");
        return ;
    }
    if(status == MQTT_CONNECTION_ACCEPTED)
    {
        Trace(1,"MQTT succeed connect to broker");
        //!!! DO NOT suscribe here(interrupt function), do MQTT suscribe in task, or it will not excute
        event->id = MQTT_EVENT_CONNECTED;
        event->client = client;
        OS_SendEvent(secondTaskHandle,event,OS_TIME_OUT_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
    }
    else
    {
        event->id = MQTT_EVENT_DISCONNECTED;
        event->client = client;
        OS_SendEvent(secondTaskHandle,event,OS_TIME_OUT_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
        Trace(1,"MQTT connect to broker fail,error code:%d",status);
    }
    Trace(1,"MQTT OnMqttConnection() end");
}

void StartTimerPublish(uint32_t interval,MQTT_Client_t* client);
void OnPublish(void* arg, MQTT_Error_t err)
{
    if(err == MQTT_ERROR_NONE)
        Trace(1,"MQTT publish success");
    else
        Trace(1,"MQTT publish error, error code:%d",err);
}

void OnTimerPublish(void* param)
{
    MQTT_Error_t err;
    MQTT_Client_t* client = (MQTT_Client_t*)param;
    if(mqttStatus != MQTT_STATUS_CONNECTED)
    {
        Trace(1,"MQTT not connected to broker! can not publish");
        return;
    }
    Trace(1,"MQTT OnTimerPublish");
    err = MQTT_Publish(client,"devices/gprsa/messages/events/",PUBLISH_PAYLOEAD,strlen(PUBLISH_PAYLOEAD),1,1,0,OnPublish,NULL);
    if(err != MQTT_ERROR_NONE)
        Trace(1,"MQTT publish error, error code:%d",err);
    StartTimerPublish(PUBLISH_INTERVAL,client);
}

void StartTimerPublish(uint32_t interval,MQTT_Client_t* client)
{
    OS_StartCallbackTimer(mainTaskHandle,interval,OnTimerPublish,(void*)client);
}

void SecondTaskEventDispatch(MQTT_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case MQTT_EVENT_CONNECTED:
            mqttStatus = MQTT_STATUS_CONNECTED;
            Trace(1,"MQTT connected, now subscribe topic:%s","devices/gprsa/messages/events/");
            MQTT_Error_t err;
            MQTT_SetInPubCallback(pEvent->client, OnMqttReceived, OnMqttReceiedData, NULL);
            err = MQTT_Subscribe(pEvent->client,"devices/gprsa/messages/events/",1,OnMqttSubscribed,(void*)"devices/gprsa/messages/events/");
            if(err != MQTT_ERROR_NONE)
                Trace(1,"MQTT subscribe error, error code:%d",err);
            StartTimerPublish(PUBLISH_INTERVAL,pEvent->client);
            break;
        case MQTT_EVENT_DISCONNECTED:
            mqttStatus = MQTT_STATUS_DISCONNECTED;
            break;
        default:
            break;
    }
}

void SecondTask(void *pData)
{
    MQTT_Event_t* event=NULL;

    INFO_GetIMEI(imei);
    Trace(1,"IMEI:%s",imei);

    semMqttStart = OS_CreateSemaphore(0);
    OS_WaitForSemaphore(semMqttStart,OS_WAIT_FOREVER);
    OS_DeleteSemaphore(semMqttStart);
    Trace(1,"start mqtt test");
    MQTT_Client_t* client = MQTT_ClientNew();
    MQTT_Connect_Info_t ci;
    MQTT_Error_t err;
    memset(&ci,0,sizeof(MQTT_Connect_Info_t));
    ci.client_id = "gprsa";
    ci.client_user = "GPRSA9.azure-devices.net/gprsa/api-version=2016-11-14";
    ci.client_pass = "SharedAccessSignature sr=GPRSA9.azure-devices.net%2Fdevices%2Fgprsa&sig=KymHOMHiyGt%2F1nn2XNtQh6YkraUYW3zddpXJNoD4AP0%3D&se=37542392914";
    ci.keep_alive = 60;
    ci.clean_session = 1;
    ci.use_ssl = true;
    ci.ssl_verify_mode = MQTT_SSL_VERIFY_MODE_REQUIRED;
//ca_certs=path_to_root_cert, certfile=None, keyfile=None, cert_reqs=ssl.CERT_REQUIRED, tls_version=ssl.PROTOCOL_TLSv1, ciphers=None)
    ci.ca_cert = ca_crt;
    ci.ca_crl = NULL;
    ci.client_cert = NULL;
    ci.client_key  = NULL;
    ci.client_key_passwd = NULL;
    ci.broker_hostname = "GPRSA9.azure-devices.net";
    ci.ssl_min_version   = MQTT_SSL_VERSION_TLSv1;
    ci.ssl_max_version   = MQTT_SSL_VERSION_TLSv1;
    ci.entropy_custom    = "GPRS_A9";
    ci.will_qos = 1;
    ci.will_topic = "will";
    ci.will_retain = 1;
    memcpy(strstr(willMsg,"GPRS")+5,imei,15);
    ci.will_msg = willMsg;

    err = MQTT_Connect(client,"GPRSA9.azure-devices.net",8883,OnMqttConnection,NULL,&ci);
    if(err != MQTT_ERROR_NONE)
        Trace(1,"MQTT connect fail,error code:%d",err);
    
    while(1)
    {
        if(OS_WaitEvent(secondTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            SecondTaskEventDispatch(event);
            OS_Free(event);
        }
    }
}

void MainTask(void *pData)
{
    API_Event_t* event=NULL;

    secondTaskHandle = OS_CreateTask(SecondTask,
        NULL, NULL, SECOND_TASK_STACK_SIZE, SECOND_TASK_PRIORITY, 0, 0, SECOND_TASK_NAME);

    while(1)
    {
        if(OS_WaitEvent(mainTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            EventDispatch(event);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}

void mqtt_ssl_Main(void)
{
    mainTaskHandle = OS_CreateTask(MainTask,
        NULL, NULL, MAIN_TASK_STACK_SIZE, MAIN_TASK_PRIORITY, 0, 0, MAIN_TASK_NAME);
    OS_SetUserMainHandle(&mainTaskHandle);
}



