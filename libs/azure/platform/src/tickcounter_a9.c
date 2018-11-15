// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/gballoc.h"

#include <stdint.h>
#include <time.h>
#include <api_os.h>
#include <api_debug.h>
#include <api_event.h>
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

typedef struct TICK_COUNTER_INSTANCE_TAG {
    clock_t init_clock;
} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    printf("tickcounter_create\n");
    TICK_COUNTER_INSTANCE *result = (TICK_COUNTER_INSTANCE *)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (result != NULL) {
        result->init_clock = clock();
    }
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    printf("tickcounter_destroy\n");
    if (tick_counter != NULL) {
        free(tick_counter);
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t *current_ms)
{
    printf("tickcounter_get_current_ms\n");
    int result;
    clock_t current_clock;

    if (tick_counter == NULL || current_ms == NULL) {
        printf("tickcounter failed: Invalid Arguments.\n");
        result = __FAILURE__;
    } else {
        current_clock = clock();
        *current_ms = (tickcounter_ms_t)((current_clock - tick_counter->init_clock) / CLOCKS_PER_MSEC);
        result = 0;
    }
    return result;
}
