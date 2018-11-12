// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

#include "api_ssl.h"
#include "api_os.h"
#include "api_socket.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "socket_async.h"
#include "dns_async.h"
#include "tlsio_pal.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/tlsio_options.h"

#define MAX_VALID_PORT 0xffff
/* Codes_SRS_TLSIO_ARDUINO_21_027: [ The tlsio_arduino_open shall set the tlsio to try to open the connection for 10 times before assuming that connection failed. ]*/
#define MAX_TLS_OPENING_RETRY  10
/* Codes_SRS_TLSIO_ARDUINO_21_044: [ The tlsio_arduino_close shall set the tlsio to try to close the connection for 10 times before assuming that close connection failed. ]*/
#define MAX_TLS_CLOSING_RETRY  10

// The TLSIO_RECEIVE_BUFFER_SIZE has very little effect on performance, and is kept small
// to minimize memory consumption.
#define RECEIVE_BUFFER_SIZE    128

#define CallErrorCallback() do { if (tls_io_instance->on_io_error != NULL) (void)tls_io_instance->on_io_error(tls_io_instance->on_io_error_context); } while((void)0,0)
#define CallOpenCallback(status) do { if (tls_io_instance->on_io_open_complete != NULL) (void)tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, status); } while((void)0,0)
#define CallCloseCallback() do { if (tls_io_instance->on_io_close_complete != NULL) (void)tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context); } while((void)0,0)

typedef enum TLSIO_STATE_TAG
{
    TLSIO_STATE_OPENING,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_CLOSED,
    TLSIO_STATE_ERROR,
} TLSIO_STATE;

// This structure definition is mirrored in the unit tests, so if you change
// this struct, keep it in sync with the one in tlsio_openssl_compact_ut.c
typedef struct TLS_IO_INSTANCE_TAG
{
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    void* on_io_open_complete_context;

    ON_BYTES_RECEIVED on_bytes_received;
    void* on_bytes_received_context;

    ON_IO_ERROR on_io_error;
    void* on_io_error_context;

    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    void* on_io_close_complete_context;

    char* hostname;
    char* port;

    SSL_Config_t* config;
    char* trusted_certificates;

    TLSIO_STATE tlsio_state;
    int countTry;
    TLSIO_OPTIONS options;
} TLS_IO_INSTANCE;

static void tlsio_ssl_Init(TLS_IO_INSTANCE* tls_io_instance)
{
    tls_io_instance->config->caCert = tls_io_instance->trusted_certificates;
    tls_io_instance->config->caCrl  = NULL;
    tls_io_instance->config->clientCert = NULL;
    tls_io_instance->config->clientKey  = NULL;
    tls_io_instance->config->clientKeyPasswd = NULL;
    tls_io_instance->config->hostName   = tls_io_instance->hostname;
    tls_io_instance->config->minVersion = SSL_VERSION_TLSv1_2;
    tls_io_instance->config->maxVersion = SSL_VERSION_TLSv1_2;
    tls_io_instance->config->verifyMode = SSL_VERIFY_MODE_REQUIRED;
    tls_io_instance->config->entropyCustom = "GPRS_AZURE";
}

static int tlsio_ssl_setoption(CONCRETE_IO_HANDLE tlsio_handle, const char* optionName, const void* value);
static void tlsio_ssl_destroy(CONCRETE_IO_HANDLE tlsio_handle);
static void tlsio_ssl_dowork(CONCRETE_IO_HANDLE tlsio_handle);

/* Codes_SRS_TLSIO_OPENSSL_COMPACT_30_560: [ The  tlsio_retrieveoptions  shall do nothing and return an empty options handler. ]*/
static OPTIONHANDLER_HANDLE tlsio_ssl_retrieveoptions(CONCRETE_IO_HANDLE tlsio_handle)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tlsio_handle;
    /* Codes_SRS_TLSIO_30_160: [ If the tlsio_handle parameter is NULL, tlsio_openssl_compact_retrieveoptions shall do nothing except log an error and return FAILURE. ]*/
    OPTIONHANDLER_HANDLE result;
    if (tls_io_instance == NULL)
    {
        LogError("NULL tlsio");
        result = NULL;
    }
    else
    {
        result = tlsio_options_retrieve_options(&tls_io_instance->options, tlsio_ssl_setoption);
    }
    return result;
}

/* Codes_SRS_TLSIO_30_010: [ The tlsio_create shall allocate and initialize all necessary resources and return an instance of the tlsio_openssl_compact. ]*/
static CONCRETE_IO_HANDLE tlsio_ssl_create(void* io_create_parameters)
{
    TLS_IO_INSTANCE* tls_io_instance;

    /* Codes_SRS_TLSIO_30_012: [ The tlsio_create shall receive the connection configuration as a TLSIO_CONFIG* in io_create_parameters. ]*/
    TLSIO_CONFIG* tls_io_config = (TLSIO_CONFIG*)io_create_parameters;

    if (io_create_parameters == NULL || tls_io_config->hostname == NULL || tls_io_config->port < 0 || tls_io_config->port > MAX_VALID_PORT)
    {
        /* Codes_SRS_TLSIO_30_013: [ If the io_create_parameters value is NULL, tlsio_create shall log an error and return NULL. ]*/
        LogError("Invalid TLS parameters.");
        /* Codes_SRS_TLSIO_30_014: [ If the hostname member of io_create_parameters value is NULL, tlsio_create shall log an error and return NULL. ]*/
        LogError("NULL tls_io_config->hostname");
        /* Codes_SRS_TLSIO_30_015: [ If the port member of io_create_parameters value is less than 0 or greater than 0xffff, tlsio_create shall log an error and return NULL. ]*/
        LogError("tls_io_config->port out of range");
        tls_io_instance = NULL;
    }
    else
    {
        tls_io_instance = (TLS_IO_INSTANCE*)malloc(sizeof(TLS_IO_INSTANCE));
        if (tls_io_instance == NULL)
        {
            /* Codes_SRS_TLSIO_30_011: [ If any resource allocation fails, tlsio_create shall return NULL. ]*/
            LogError("There is not enough memory to create the TLS instance.");
            free(tls_io_instance);
            tls_io_instance = NULL;
        }
        else
        {
            int ret = 0;
            char port_str[10];
            /* SRS_TLSIO_30_010: [ The tlsio_create shall allocate and initialize all necessary resources and return an instance of the tlsio in TLSIO_STATE_EXT_CLOSED. ] */
            memset(tls_io_instance, 0, sizeof(TLS_IO_INSTANCE));
            tls_io_instance->on_io_open_complete = NULL;
            tls_io_instance->on_io_open_complete_context = NULL;
            tls_io_instance->on_bytes_received = NULL;
            tls_io_instance->on_bytes_received_context = NULL;
            tls_io_instance->on_io_error = NULL;
            tls_io_instance->on_io_error_context = NULL;
            tls_io_instance->on_io_close_complete = NULL;
            tls_io_instance->on_io_close_complete_context = NULL;
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSED;
            // No options are currently supported
            tlsio_options_initialize(&tls_io_instance->options, TLSIO_OPTION_BIT_NONE);
            /* Codes_SRS_TLSIO_30_016: [ tlsio_create shall make a copy of the hostname member of io_create_parameters to allow deletion of hostname immediately after the call. ]*/
            ret = mallocAndStrcpy_s(&tls_io_instance->hostname, tls_io_config->hostname);
            if (ret != 0) {
                /* Codes_SRS_TLSIO_30_011: [ If any resource allocation fails, tlsio_create shall return NULL. ]*/
                LogError("There is not enough memory to create tls_io_instance->hostname.");
                tlsio_ssl_destroy(tls_io_instance);
                tls_io_instance = NULL;
            } else {
                itoa(tls_io_config->port, port_str, 10);
                ret = mallocAndStrcpy_s(&tls_io_instance->port, port_str);
                if(ret != 0) {
                    /* Codes_SRS_TLSIO_30_011: [ If any resource allocation fails, tlsio_create shall return NULL. ]*/
                    LogError("There is not enough memory to create tls_io_instance->port.");
                    tlsio_ssl_destroy(tls_io_instance);
                    tls_io_instance = NULL;
                } else {
                    tlsio_ssl_Init(tls_io_instance);
                }
            }
        }
    }

    return (CONCRETE_IO_HANDLE)tls_io_instance;
}

static void tlsio_ssl_destroy(CONCRETE_IO_HANDLE tlsio_handle)
{
    if (tlsio_handle == NULL)
    {
        /* Codes_SRS_TLSIO_30_020: [ If tlsio_handle is NULL, tlsio_destroy shall do nothing. ]*/
        LogError("NULL TLS handle.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tlsio_handle;
        if (tls_io_instance->tlsio_state != TLSIO_STATE_CLOSED)
        {
            /* Codes_SRS_TLSIO_30_022: [ If the adapter is in any state other than TLSIO_STATE_EX_CLOSED when tlsio_destroy is called, the adapter shall enter TLSIO_STATE_EX_CLOSING and then enter TLSIO_STATE_EX_CLOSED before completing the destroy process. ]*/
            LogError("TLS destroyed with a SSL connection still active.");
        }
        /* Codes_SRS_TLSIO_30_021: [ The tlsio_destroy shall release all allocated resources and then release tlsio_handle. ]*/
        if (tls_io_instance->hostname != NULL)
        {
            free(tls_io_instance->hostname);
        }
        /* Codes_SRS_TLSIO_30_021: [ The tlsio_destroy shall release all allocated resources and then release tlsio_handle. ]*/
        if (tls_io_instance->port != NULL)
        {
            free(tls_io_instance->port);
        }

        tlsio_options_release_resources(&tls_io_instance->options);

        free(tls_io_instance);
    }
}

static int tlsio_ssl_open(CONCRETE_IO_HANDLE tlsio_handle,
    ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context,
    ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context,
    ON_IO_ERROR on_io_error, void* on_io_error_context)
{

    int result;
    if (on_io_open_complete == NULL || tlsio_handle == NULL || 
        on_bytes_received == NULL || on_io_error == NULL)
    {
        /* Codes_SRS_TLSIO_30_031: [ If the on_io_open_complete parameter is NULL, tlsio_open shall log an error and return FAILURE. ]*/
        LogError("Invalid parameter: NULL");
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tlsio_handle;
        /* Codes_SRS_TLSIO_30_034: [ The tlsio_open shall store the provided on_bytes_received, on_bytes_received_context, on_io_error, on_io_error_context, on_io_open_complete, and on_io_open_complete_context parameters for later use as specified and tested per other line entries in this document. ]*/
        tls_io_instance->on_bytes_received = on_bytes_received;
        tls_io_instance->on_bytes_received_context = on_bytes_received_context;

        tls_io_instance->on_io_error = on_io_error;
        tls_io_instance->on_io_error_context = on_io_error_context;

        tls_io_instance->on_io_open_complete = on_io_open_complete;
        tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_CLOSED)
        {
            /* Codes_SRS_TLSIO_30_037: [ If the adapter is in any state other than TLSIO_STATE_EXT_CLOSED when tlsio_open  is called, it shall log an error, and return FAILURE. ]*/
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_CLOSED.");
            result = __FAILURE__;
        }
        else
        {
            /* Codes_SRS_TLSIO_30_035: [ On tlsio_open success the adapter shall enter TLSIO_STATE_EX_OPENING and return 0. ]*/
            tls_io_instance->tlsio_state = TLSIO_STATE_OPENING;
            tls_io_instance->countTry = MAX_TLS_OPENING_RETRY;
            // All the real work happens in dowork
            tlsio_ssl_dowork(tlsio_handle);
            result = 0;
        }
        /* Codes_SRS_TLSIO_30_039: [ On failure, tlsio_open_async shall not call on_io_open_complete. ]*/
    }

    if (result != 0)
    {
        if (on_io_open_complete != NULL)
        {
            /* Codes_SRS_TLSIO_ARDUINO_21_002: [ The tlsio_arduino shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`.]*/
            /* Codes_SRS_TLSIO_ARDUINO_21_039: [ If the tlsio_arduino_open failed to open the tls connection, and the on_io_open_complete callback was provided, it shall call the on_io_open_complete with IO_OPEN_ERROR. ]*/
            (void)on_io_open_complete(on_io_open_complete_context, IO_OPEN_ERROR);
        }
        if (on_io_error != NULL)
        {
            /* Codes_SRS_TLSIO_ARDUINO_21_040: [ If the tlsio_arduino_open failed to open the tls connection, and the on_io_error callback was provided, it shall call the on_io_error. ]*/
            (void)on_io_error(on_io_error_context);
        }
    }
    else
    {
        /* Codes_SRS_TLSIO_ARDUINO_21_041: [ If the tlsio_arduino_open get success opening the tls connection, it shall call the tlsio_ssl_dowork. ]*/
        tlsio_ssl_dowork(tlsio_handle);
    }
    return result;
}

// This implementation does not have asynchronous close, but uses the _async name for consistency with the spec
static int tlsio_ssl_close(CONCRETE_IO_HANDLE tlsio_handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
{
    int result;
    if (tlsio_handle == NULL)
    {
        /* Codes_SRS_TLSIO_30_050: [ If the tlsio_handle parameter is NULL, tlsio_openssl_close_async shall log an error and return FAILURE. ]*/
        LogError("NULL tlsio");
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tlsio_handle;

        /* Codes_SRS_TLSIO_ARDUINO_21_045: [ The tlsio_arduino_close shall store the provided on_io_close_complete callback function address. ]*/
        tls_io_instance->on_io_close_complete = on_io_close_complete;
        /* Codes_SRS_TLSIO_ARDUINO_21_046: [ The tlsio_arduino_close shall store the provided on_io_close_complete_context handle. ]*/
        tls_io_instance->on_io_close_complete_context = on_io_close_complete_context;

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_CLOSED) || (tls_io_instance->tlsio_state == TLSIO_STATE_ERROR))
        {
            /* Codes_SRS_TLSIO_30_053: [ If the adapter is in any state other than TLSIO_STATE_EXT_OPEN or TLSIO_STATE_EXT_ERROR then tlsio_close_async shall log that tlsio_close_async has been called and then continue normally. ]*/
            // LogInfo rather than LogError because this is an unusual but not erroneous situation
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSED;
            result = 0;
        } 
        else if ((tls_io_instance->tlsio_state == TLSIO_STATE_OPENING) || (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
        {
            /* Codes_SRS_TLSIO_30_057: [ On success, if the adapter is in TLSIO_STATE_EXT_OPENING, it shall call on_io_open_complete with the on_io_open_complete_context supplied in tlsio_open_async and IO_OPEN_CANCELLED. This callback shall be made before changing the internal state of the adapter. ]*/
            LogError("Try to close the connection with an already closed TLS.");
            tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
            result = __FAILURE__;
        }
        else
        {
            /* Codes_SRS_TLSIO_30_056: [ On success the adapter shall enter TLSIO_STATE_EX_CLOSING. ]*/
            /* Codes_SRS_TLSIO_30_052: [ On success tlsio_close shall return 0. ]*/
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
            /* Codes_SRS_TLSIO_ARDUINO_21_044: [ The tlsio_arduino_close shall set the tlsio to try to close the connection for 10 times before assuming that close connection failed. ]*/
            tls_io_instance->countTry = MAX_TLS_CLOSING_RETRY;
            /* Codes_SRS_TLSIO_ARDUINO_21_044: [ The tlsio_arduino_close shall set the tlsio to try to close the connection for 10 times before assuming that close connection failed. ]*/
            /* Codes_SRS_TLSIO_ARDUINO_21_050: [ If the tlsio_arduino_close get success closing the tls connection, it shall call the tlsio_ssl_dowork. ]*/
            tlsio_ssl_dowork(tlsio_handle);
            result = 0;
        }
    }
    /* Codes_SRS_TLSIO_30_054: [ On failure, the adapter shall not call on_io_close_complete. ]*/

    return result;
}

static int tlsio_ssl_send(CONCRETE_IO_HANDLE tlsio_handle, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* on_send_complete_context)
{
    int result;
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tlsio_handle;

    if ((tlsio_handle == NULL) || (buffer == NULL) || (size == 0))
    {
        /* Codes_SRS_TLSIO_30_060: [ If the tlsio_handle parameter is NULL, tlsio_openssl_compact_send shall log an error and return FAILURE. ]*/
        /* Codes_SRS_TLSIO_30_061: [ If the buffer is NULL, tlsio_openssl_compact_send shall log the error and return FAILURE. ]*/
        /* Codes_SRS_TLSIO_30_067: [ If the  size  is 0,  tlsio_send  shall log the error and return FAILURE. ]*/
        LogError("Invalid parameter");
        result = __FAILURE__;
    }
    else if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
    {
        /* Codes_SRS_TLSIO_ARDUINO_21_058: [ If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
        LogError("TLS is not ready to send data");
        result = __FAILURE__;
    }
    else
    {
        int send_result;
        size_t send_size = size;
        const uint8_t* runBuffer = (const uint8_t *)buffer;
        result = __FAILURE__;
        /* Codes_SRS_TLSIO_ARDUINO_21_055: [ if the ssl was not able to send all data in the buffer, the tlsio_arduino_send shall call the ssl again to send the remaining bytes. ]*/
        while (send_size > 0)
        {
            // int         SSL_Write(SSL_Config_t* sslConfig, uint8_t* data, int length, int timeoutMs);
            send_result = SSL_Write(tls_io_instance->config, (uint8_t *)runBuffer, (int)send_size, 5000);

            if (send_result <= 0) /* Didn't transmit anything! Failed. */
            {
                /* Codes_SRS_TLSIO_ARDUINO_21_056: [ if the ssl was not able to send any byte in the buffer, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. ]*/
                LogError("TLS failed sending data");
                /* Codes_SRS_TLSIO_ARDUINO_21_053: [ The tlsio_arduino_send shall use the provided on_io_send_complete callback function address. ]*/
                /* Codes_SRS_TLSIO_ARDUINO_21_054: [ The tlsio_arduino_send shall use the provided on_io_send_complete_context handle. ]*/
                if (on_send_complete != NULL)
                {
                    /* Codes_SRS_TLSIO_ARDUINO_21_003: [ The tlsio_arduino shall report the send operation status using the IO_SEND_RESULT enumerator defined in the `xio.h`. ]*/
                    on_send_complete(on_send_complete_context, IO_SEND_ERROR);
                }
                send_size = 0;
            }
            else if (send_result >= send_size) /* Transmit it all. */
            {
                /* Codes_SRS_TLSIO_ARDUINO_21_057: [ if the ssl finish to send all bytes in the buffer, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_OK, and return 0 ]*/
                /* Codes_SRS_TLSIO_ARDUINO_21_053: [ The tlsio_arduino_send shall use the provided on_io_send_complete callback function address. ]*/
                /* Codes_SRS_TLSIO_ARDUINO_21_054: [ The tlsio_arduino_send shall use the provided on_io_send_complete_context handle. ]*/
                if (on_send_complete != NULL)
                {
                    /* Codes_SRS_TLSIO_ARDUINO_21_003: [ The tlsio_arduino shall report the send operation status using the IO_SEND_RESULT enumerator defined in the `xio.h`. ]*/
                    on_send_complete(on_send_complete_context, IO_SEND_OK);
                }
                result = 0;
                send_size = 0;
            }
            else /* Still have buffer to transmit. */
            {
                runBuffer += send_result;
                send_size -= send_result;
            }
        }
        /* Codes_SRS_TLSIO_30_066: [ On failure, on_send_complete shall not be called. ]*/
    }
    return result;
}

static void tlsio_ssl_dowork(CONCRETE_IO_HANDLE tlsio_handle)
{
    if (tlsio_handle == NULL)
    {
        /* Codes_SRS_TLSIO_30_070: [ If the tlsio_handle parameter is NULL, tlsio_dowork shall do nothing except log an error. ]*/
        LogError("Invalid parameter: tlsio NULL");
    }
    else
    {
        int received;
        SSL_Error_t error;
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tlsio_handle;
        /* Codes_SRS_TLSIO_ARDUINO_21_075: [ The tlsio_ssl_dowork shall create a buffer to store the data received from the ssl client. ]*/
        /* Codes_SRS_TLSIO_ARDUINO_21_076: [ The tlsio_ssl_dowork shall delete the buffer to store the data received from the ssl client. ]*/
        uint8_t RecvBuffer[RECEIVE_BUFFER_SIZE];

        // This switch statement handles all of the state transitions during the opening process
        switch (tls_io_instance->tlsio_state)
        {
        case TLSIO_STATE_OPENING:
            if ((tls_io_instance->countTry--) >= 0) 
            {
                error = SSL_Connect(tls_io_instance->config, tls_io_instance->hostname, tls_io_instance->port);
                if (error != SSL_ERROR_NONE)
                {
                    /* Codes_SRS_TLSIO_30_038: [ If tlsio_open fails to enter TLSIO_STATE_EX_OPENING it shall return FAILURE. ]*/
                    LogError("TLS failed to start the connection process.");
                } 
                else 
                {
                    /* Codes_SRS_TLSIO_ARDUINO_21_063: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, and ssl client is connected, the tlsio_ssl_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_OPEN, and call the on_io_open_complete with IO_OPEN_OK. ]*/
                    tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
                    /* Codes_SRS_TLSIO_ARDUINO_21_002: [ The tlsio_arduino shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`.]*/
                    CallOpenCallback(IO_OPEN_OK);
                }
            } 
            else 
            {
                /* Codes_SRS_TLSIO_ARDUINO_21_065: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, ssl client is not connected, and the counter to try becomes 0, the tlsio_ssl_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_ERROR, call on_io_open_complete with IO_OPEN_CANCELLED, call on_io_error. ]*/
                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                LogError("Timeout for TLS connect.");
                /* Codes_SRS_TLSIO_ARDUINO_21_002: [ The tlsio_arduino shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`.]*/
                /* Codes_SRS_TLSIO_ARDUINO_21_042: [ If the tlsio_arduino_open retry to open more than 10 times without success, it shall call the on_io_open_complete with IO_OPEN_CANCELED. ]*/
                CallOpenCallback(IO_OPEN_CANCELLED);
                CallErrorCallback();
            }
            break;
        case TLSIO_STATE_OPEN:
            /* Codes_SRS_TLSIO_ARDUINO_21_069: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, the tlsio_ssl_dowork shall read data from the ssl client. ]*/
            while ((received = SSL_Read(tls_io_instance->config, (uint8_t*)RecvBuffer, RECEIVE_BUFFER_SIZE, 5000)) > 0)
            {
                /* Codes_SRS_TLSIO_ARDUINO_21_070: [ If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, and there are received data in the ssl client, the tlsio_ssl_dowork shall read this data and call the on_bytes_received with the pointer to the buffer with the data. ]*/
                if (tls_io_instance->on_bytes_received != NULL)
                {
                    // explictly ignoring here the result of the callback
                    (void)tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, (const unsigned char*)RecvBuffer, received);
                }
            }
            break;
        case TLSIO_STATE_CLOSING:
            if ((tls_io_instance->countTry--) >= 0) {
                error = SSL_Close(tls_io_instance->config);
                if (error != SSL_ERROR_NONE)
                {
                    /* Codes_SRS_TLSIO_30_038: [ If tlsio_open fails to enter TLSIO_STATE_EX_OPENING it shall return FAILURE. ]*/
                    LogError("TLS failed to start the connection process.");
                }
                else
                {
                    /* Codes_SRS_TLSIO_ARDUINO_21_066: [ If the tlsio state is TLSIO_ARDUINO_STATE_CLOSING, and ssl client is not connected, the tlsio_ssl_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_CLOSE, and call the on_io_close_complete. ]*/
                    tls_io_instance->tlsio_state = TLSIO_STATE_CLOSED;
                    CallCloseCallback();
                }
            } 
            else 
            {
                /* Codes_SRS_TLSIO_ARDUINO_21_051: [ If the tlsio_arduino_close retry to close more than 10 times without success, it shall call the on_io_error. ]*/
                /* Codes_SRS_TLSIO_ARDUINO_21_068: [ If the tlsio state is TLSIO_ARDUINO_STATE_CLOSING, ssl client is connected, and the counter to try becomes 0, the tlsio_ssl_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_ERROR, call on_io_error. ]*/
                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                LogError("Timeout for close TLS");
                CallErrorCallback();
            }
            break;
        case TLSIO_STATE_CLOSED:
            /* Codes_SRS_TLSIO_30_075: [ If the adapter is in TLSIO_STATE_EXT_CLOSED then  tlsio_dowork  shall do nothing. ]*/
            // Waiting to be opened, nothing to do
            break;
        case TLSIO_STATE_ERROR:
            /* Codes_SRS_TLSIO_30_071: [ If the adapter is in TLSIO_STATE_EXT_ERROR then tlsio_dowork shall do nothing. ]*/
            // There's nothing valid to do here but wait to be retried
            break;
        default:
            LogError("Unexpected internal tlsio state");
            break;
        }
    }
}

static int tlsio_ssl_setoption(CONCRETE_IO_HANDLE tlsio_handle, const char* optionName, const void* value)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tlsio_handle;
    /* Codes_SRS_TLSIO_30_120: [ If the tlsio_handle parameter is NULL, tlsio_openssl_compact_setoption shall do nothing except log an error and return FAILURE. ]*/
    int result;
    if (tls_io_instance == NULL)
    {
        LogError("NULL tlsio");
        result = __FAILURE__;
    }
    else
    {
        /* Codes_SRS_TLSIO_30_121: [ If the optionName parameter is NULL, tlsio_openssl_compact_setoption shall do nothing except log an error and return FAILURE. ]*/
        /* Codes_SRS_TLSIO_30_122: [ If the value parameter is NULL, tlsio_openssl_compact_setoption shall do nothing except log an error and return FAILURE. ]*/
        /* Codes_SRS_TLSIO_OPENSSL_COMPACT_30_520 [ The tlsio_setoption shall do nothing and return FAILURE. ]*/
        if (strcmp(OPTION_TRUSTED_CERT, optionName) == 0) {
            if (tls_io_instance->trusted_certificates != NULL)
            {
                // Free the memory if it has been previously allocated
                free(tls_io_instance->trusted_certificates);
                tls_io_instance->trusted_certificates = NULL;
            }
            if (mallocAndStrcpy_s(&tls_io_instance->trusted_certificates, (const char*)value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s");
                result = __FAILURE__;
            }
        }
        TLSIO_OPTIONS_RESULT options_result = tlsio_options_set(&tls_io_instance->options, optionName, value);
        if (options_result != TLSIO_OPTIONS_RESULT_SUCCESS)
        {
            LogError("Failed tlsio_options_set");
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }
    return result;
}

/* Codes_SRS_TLSIO_30_008: [ The tlsio_get_interface_description shall return the VTable IO_INTERFACE_DESCRIPTION. ]*/
static const IO_INTERFACE_DESCRIPTION tlsio_ssl_interface_description =
{
    tlsio_ssl_retrieveoptions,  //done
    tlsio_ssl_create,  //done
    tlsio_ssl_destroy,  //done
    tlsio_ssl_open,
    tlsio_ssl_close,
    tlsio_ssl_send,
    tlsio_ssl_dowork,
    tlsio_ssl_setoption
};

/* Codes_SRS_TLSIO_30_001: [ The tlsio_openssl_compact shall implement and export all the Concrete functions in the VTable IO_INTERFACE_DESCRIPTION defined in the xio.h. ]*/
const IO_INTERFACE_DESCRIPTION* tlsio_pal_get_interface_description(void)
{
    return &tlsio_ssl_interface_description;
}
