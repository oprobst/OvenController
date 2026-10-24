#pragma once

// Selects the hardware configuration of the PCB revision chosen by the build environment.

#if defined(OVEN_BOARD_REV_DEV)
#include "board/board_rev_dev.h"
#else
#error "unsupported PCB revision"
#endif
