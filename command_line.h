#ifndef __ALS_COMMAND_LINE_H__
#define __ALS_COMMAND_LINE_H__

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "em_system.h"
#include "em_cmu.h"
#include "sl_cli.h"
#include "sl_cli_instances.h"
#include "sl_cli_arguments.h"
#include "app_log.h"

#include "buzzer.h"
#include "led.h"
#include "adc.h"
#include "tip_id_validate.h"

void app_cli_setup(void);

#endif
