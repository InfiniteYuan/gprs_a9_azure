// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/xlogging.h"

#include "api_os.h"

LOCK_HANDLE Lock_Init(void)
{
    /* Codes_SRS_LOCK_10_002: [Lock_Init on success shall return a valid lock handle which should be a non NULL value] */
    /* Codes_SRS_LOCK_10_003: [Lock_Init on error shall return NULL ] */
    // xSemaphoreCreateMutex is the obvious choice, but it returns a recursive
    // sync object, which we do not want for the lock adapter.
    HANDLE result = OS_CreateSemaphore(0);
    if (result != NULL)
    {
        int gv = OS_ReleaseSemaphore(result);
        if (gv != true)
        {
            LogError("OS_ReleaseSemaphore failed after create.");
            OS_DeleteSemaphore(result);
            result = NULL;
        }
    }
    else
    {
        LogError("OS_CreateSemaphore failed.");
    }

    return (LOCK_HANDLE)result;
}

LOCK_RESULT Lock_Deinit(LOCK_HANDLE handle)
{
    LOCK_RESULT result;
    if (handle == NULL)
    {
        /* Codes_SRS_LOCK_10_007: [Lock_Deinit on NULL handle passed returns LOCK_ERROR] */
        LogError("Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        /* Codes_SRS_LOCK_10_012: [Lock_Deinit frees the memory pointed by handle] */
        OS_DeleteSemaphore((HANDLE)handle);
        result = LOCK_OK;
    }

    return result;
}

LOCK_RESULT Lock(LOCK_HANDLE handle)
{
    LOCK_RESULT result;
    if (handle == NULL)
    {
        /* Codes_SRS_LOCK_10_007: [Lock on NULL handle passed returns LOCK_ERROR] */
        LogError("Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else 
    {
        int rv = OS_WaitForSemaphore((HANDLE)handle, OS_WAIT_FOREVER);
        switch (rv)
        {
            case true:
                /* Codes_SRS_LOCK_10_005: [Lock on success shall return LOCK_OK] */
                result = LOCK_OK;
                break;
            default:
                LogError("OS_WaitForSemaphore failed.");
                /* Codes_SRS_LOCK_10_006: [Lock on error shall return LOCK_ERROR] */
                result = LOCK_ERROR;
                break;
        }
    }    

    return result;
}

LOCK_RESULT Unlock(LOCK_HANDLE handle)
{
    LOCK_RESULT result;
    if (handle == NULL)
    {
        /* Codes_SRS_LOCK_10_007: [Unlock on NULL handle passed returns LOCK_ERROR] */
        LogError("Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        int rv = OS_ReleaseSemaphore((HANDLE)handle);
        switch (rv)
        {
            case true:
                /* Codes_SRS_LOCK_10_005: [Lock on success shall return LOCK_OK] */
                result = LOCK_OK;
                break;
            default:
                LogError("OS_ReleaseSemaphore failed.");
                /* Codes_SRS_LOCK_10_006: [Lock on error shall return LOCK_ERROR] */
                result = LOCK_ERROR;
                break;
        }
    }
    
    return result;
}
