#pragma once

#include <stdbool.h>

#include "system_types.h"

bool display_init(void);
void display_set_dimmed(bool dimmed);
void display_update(const system_status_t *status);

