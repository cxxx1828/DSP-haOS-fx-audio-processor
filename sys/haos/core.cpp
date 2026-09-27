

#include <assert.h>

#include "haos.h"
#include "bitripper_sim.h"

namespace Core
{

    void initFIFO(uint32_t id, uint32_t* addr, int32_t size)
    {
        assert(id < MAX_FIFO_CNT && "Invalid FIFO ID!");

        pHAOS_Core_t pCore = (pHAOS_Core_t)HAOS::getActiveCore();

        pCore->FIFODescArray[id].ID = id;
        pCore->FIFODescArray[id].startAddr = addr;
        pCore->FIFODescArray[id].size = size;

    }
   
    void initBitripper(uint32_t id)
    {
        assert(id < MAX_FIFO_CNT && "Invalid FIFO ID!");

        pHAOS_Core_t pCore = (pHAOS_Core_t)HAOS::getActiveCore();

        pCore->pBitRipper = &pCore->bitRipperArray[id];

        /* Initialize the BitRipper state for this core */
        bitripper_internal::init(&pCore->FIFODescArray[id], pCore->pBitRipper);
    }

    
    void switchBitripperFIFO(uint32_t id)
    {
        assert(id < MAX_FIFO_CNT && "Invalid FIFO ID!");

        pHAOS_Core_t pCore = (pHAOS_Core_t)HAOS::getActiveCore();

        pCore->pBitRipper = &pCore->bitRipperArray[id];
    }

}


