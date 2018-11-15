// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <string.h>
#include <stdio.h>
#include <api_os.h>
#include <api_event.h>
#include <api_network.h>
#include <api_debug.h>
#include "ntp.h"
#include "azure_c_shared_utility/agenttime.h"
#include "azure_c_shared_utility/xlogging.h"

#define NTP_SERVER "cn.ntp.org.cn"

time_t sntp_get_current_timestamp()
{
    time_t timeNow = 0;
    while(1)
    {
        if (NTP_Update(NTP_SERVER, 5, &timeNow, true) != 0) {
            timeNow = 0;
            Trace(1, "ntp get time fail,timestamp:%d", timeNow);
        }
        if ( timeNow > 0) {
            Trace(1, "ntp get time success,timestamp:%d", timeNow);
            Trace(1, "timestamp:%d Time: %s", timeNow, ctime( ( const time_t * ) &timeNow ));
            break;
        }
    }
    return timeNow;
}

time_t get_time(time_t *currentTime)
{
    return sntp_get_current_timestamp();
}

double get_difftime(time_t stopTime, time_t startTime)
{
    return (double)stopTime - (double)startTime;
}

struct tm *get_gmtime(time_t *currentTime)
{
    return NULL;
}

time_t get_mktime(struct tm* cal_time)
{
    return mktime(cal_time);
}

char *get_ctime(time_t *timeToGet)
{
    return ctime(timeToGet);
}
