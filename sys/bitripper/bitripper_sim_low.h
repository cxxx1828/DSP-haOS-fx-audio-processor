/*
 * bitripper_sim.h
 *
 *  Created on: Aug 12, 2024
 *      Author: katarinac
 */

#ifndef __BITRIPPER_H__
#define __BITRIPPER_H__


#include "haos/haos_api.h"


// Bitmask for internal control flags used in bitripper state
#define BITRIPPER_IN_AUX_STATE_FLAG             BIT_00_SET 			// flag for aux state
#define BITRIPPER_IN_AUX_STATE_CLR              BIT_00_CLR

#define BITRIPPER_FIFO_FULL_FLAG             	BIT_01_SET 			// flag for full FIFO
#define BITRIPPER_FIFO_FULL_CLR             	BIT_01_CLR

#define MAX_BITS 	32


namespace bitripper_internal
{
    void init(pBitRipperFIFO_t pFIFOCfg, pBitRipperState_t pState);
}

#endif /* __BITRIPPER_H__ */
