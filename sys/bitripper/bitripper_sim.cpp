/*
 * bitripper_sim.cpp
 *
 *  Created on: Aug 12, 2024
 *      Author: katarinac
 */

#include <iostream>
#include <cstdint>
#include <cstring>
#include <math.h>
#include <assert.h>
#include <cstring>

#include "../haos/haos_api.h"
#include "bitripper_sim.h"



namespace bitripper_internal
{
void init(pBitRipperFIFO_t pFIFOCfg, pBitRipperState_t pState)
{
	pState->ctrlFlags = ALL_BITS_CLR;
	pState->alignmentInfo = 0;
	pState->overflowCntFIFO = 0;

	pState->writePtr = pFIFOCfg->startAddr;

	pBitRipper_t pCurrState = &pState->currState;

	pCurrState->currentWord = 0;
	pCurrState->bitsRemaining = 0;
	pCurrState->baseAddr = pFIFOCfg->startAddr;
	pCurrState->readPtr = pFIFOCfg->startAddr;
	pCurrState->size = pFIFOCfg->size;
	pCurrState->endAddrPlus1 = &pFIFOCfg->startAddr[pFIFOCfg->size];
	pCurrState->pWritePtr = &pState->writePtr;
	pCurrState->reserved = 0;

#if 0

	for (int i = 0; i < 16; i++) bitripper::extractBits(32);

	//skipBits(-16*32);

	uint32_t word_1 = bitripper::extractBits(32);
/*
	uint32_t word_2p = peek(8);
	uint32_t word_2 = extractBits(8);

	uint32_t word_3p = peek(4);
	uint32_t word_3 = extractBits(4);

	uint32_t word_4p = peek(32);
	uint32_t word_4 = extractBits(32);

	uint32_t word_5p = peek(32);
	uint32_t word_5 = extractBits(20);

	uint32_t word_6 = extractBits(32);
	uint32_t word_7 = extractBits(32);
	uint32_t word_8 = extractBits(32);
*/

	bitripper::skipBits(8);
	uint32_t sanja1 = bitripper::peek(32);

	bitripper::skipBits(32);
	uint32_t sanja2 = bitripper::peek(32);


	bitripper::skipBits(72);
	uint32_t sanja3 = bitripper::peek(32);

	uint32_t availBits = bitripper::readDipstick();

	bitripper::saveMainState();

	uint32_t word = bitripper::extractBits(24);

	BitRipper_t aux4;
	bitripper::saveAuxState(&aux4);
	int32_t distance4 = bitripper::bitCntMainState(&aux4);


	word = bitripper::extractBits(24);

	bitripper::saveAlignment(0);

	word = bitripper::extractBits(1);

	bitripper::alignToByte();
	bitripper::alignToWord();

	word = bitripper::extractBits(9);

	bitripper::alignToDWord();

	word = bitripper::peek(32);

	availBits = bitripper::readDipstick();

	BitRipper_t aux3;
	bitripper::saveAuxState(&aux3);
	int32_t distance3 = bitripper::bitCntMainState(&aux3);


	bitripper::skipBits(180);

	availBits = bitripper::readDipstick();

	BitRipper_t aux2;
	bitripper::saveAuxState(&aux2);

	int32_t distance2 = bitripper::bitCntMainState(&aux2);

	int32_t distance32 = bitripper::bitCntStates(&aux3, &aux2);


	bitripper::restoreMainState();


	BitRipper_t aux1;

	bitripper::saveAuxState(&aux1);

	int32_t distance1 = bitripper::bitCntMainState(&aux1);

	int32_t distance12 = bitripper::bitCntStates(&aux1, &aux2);


	availBits = bitripper::readDipstick();

	word = bitripper::peek(24);



	bitripper::skipBits(-72);
	uint32_t sanja4 = bitripper::peek(32);

	bitripper::skipBits(-32);
	uint32_t sanja5 = bitripper::peek(32);

	bitripper::skipBits(-8);
	uint32_t sanja6 = bitripper::peek(32);

	bitripper::skipBits(-32);





	uint32_t bitsInFIFO = bitripper::readDipstick();

	bitripper::skipBits(bitsInFIFO - 85);

	bitsInFIFO = bitripper::readDipstick();

	bitripper::waitOnDipstick(100);

	bitsInFIFO = bitripper::readDipstick();


	int sanja = 1;

/*
	//skipBits(-10);
	skipBits(2048*32);

	waitOnDipstick(76);
*/
#endif // 0
}
}



namespace BitRipper
{


/*
 * @brief Returns a pointer to the write position in the BitRipper FIFO buffer.
 *
 * This function provides access to the `writePtr` used by the BitRipper instance
 * associated with the currently active HaOS DSP core. The write pointer indicates
 * the next available location in the FIFO buffer where new input data can be written.
 *
 * It is typically used by refill or input simulation routines to inject data
 * into the bitstream processing pipeline.
 *
 * @return Pointer to the current write position in the BitRipper FIFO buffer.
 */
uint32_t* getWritePtr()
{
    /* Get the BitRipper core instance for the currently active HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Return the pointer to the next write location in the FIFO */
    return pBitRipperState->writePtr;
}

/*
 * Returns the number of available 32-bit word slots in the input FIFO.
 *
 * This function calculates how many 32-bit words of space are currently free in the
 * BitRipper input FIFO buffer. It does so by reading the number of occupied bits using
 * `readDipstick()`, converting that value to 32-bit word count (via right shift by 5),
 * and subtracting it from the total FIFO capacity (FIFO_SIZE).
 *
 * @return Number of free 32-bit word slots in the FIFO.
 */
uint32_t getFreeSpaceInWords()
{

    /* Get the BitRipper core instance for the currently active HaOS core */
    pBitRipperState_t bitRipperInstance = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

	/* Get pointer to current state structure */
    pBitRipper_t pBitripper = &bitRipperInstance->currState;

    /* Convert occupied bit count to number of 32-bit words */
    uint32_t wordsInFIFO = readDipstick() >> 5;

    /* Calculate number of free words in FIFO */
    uint32_t freeWordsCnt = pBitripper->size - wordsInFIFO;

    return freeWordsCnt;
}


/*
 * Advances the FIFO write pointer and updates FIFO full flag.
 *
 * This function advances the write pointer of the BitRipper's input FIFO
 * by the specified number of 32-bit words. It handles circular wrap-around
 * and updates the internal full flag if the FIFO becomes full after advancement.
 *
 * If the system is currently operating in auxiliary state mode, the comparison
 * is made against the `mainStateBackup.readPtr` to maintain consistent behavior.
 *
 * @param wordsWritten Number of 32-bit words to advance the write pointer.
 */
void advanceWritePtr(uint32_t wordsWritten)
{
    /* Get the BitRipper core instance for the currently active HaOS core */
    pBitRipperState_t bitRipperInstance = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Advance the write pointer by the specified number of words */
    bitRipperInstance->writePtr += wordsWritten;

    /* Get pointer to current state structure */
    pBitRipper_t pBitripper = &bitRipperInstance->currState;

    /* If in auxiliary state, use main state backup for comparison */
    if (bitRipperInstance->ctrlFlags & BITRIPPER_IN_AUX_STATE_FLAG)
    {
        pBitripper = &bitRipperInstance->mainStateBackup;
    }

    /* Wrap around the FIFO if the pointer exceeds the end address */
    if (bitRipperInstance->writePtr >= pBitripper->endAddrPlus1)
    {
        bitRipperInstance->writePtr -= pBitripper->size;
    }

    /* Clear the FIFO full flag by default */
    bitRipperInstance->ctrlFlags &= BITRIPPER_FIFO_FULL_CLR;

    /* If write pointer has caught up with the read pointer, set FIFO full flag */
    if (bitRipperInstance->writePtr == pBitripper->readPtr)
    {
        bitRipperInstance->ctrlFlags |= BITRIPPER_FIFO_FULL_FLAG;
    }
}



/*
 * Extracts a specified number of bits from the BitRipper input stream.
 *
 * This function extracts `bitsNeeded` bits (in the range [1, MAX_BITS]) from the current BitRipper input stream,
 * managing bit-level access across FIFO words. If the number of requested bits exceeds the remaining bits in the
 * current word, the function refills the internal word by reading the next 32-bit value from the FIFO and continues
 * extracting.
 *
 * The BitRipper state is updated accordingly: `currentWord` is shifted left, and `bitsRemaining` is decremented.
 * If all bits are consumed from the current word, the internal value is cleared.
 *
 * @param bitsNeeded Number of bits to extract (must be between 1 and MAX_BITS).
 * @return The extracted bits packed into the least significant bits of the return value.
 */
int32_t extractBits(uint32_t bitsNeeded)
{
    /* bitsNeeded must be in the range [1, MAX_BITS] */
    assert((bitsNeeded >= 1) && (bitsNeeded <= MAX_BITS) && "Invalid extract bits argument!");

    /* Get BitRipper core and current state for the active HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();
    pBitRipper_t pCurrState = &pBitRipperState->currState;

    /* Extract the first part from the current word */
    uint32_t result = pCurrState->currentWord >> (MAX_BITS - bitsNeeded);
    int32_t extractRemainingBits = bitsNeeded - pCurrState->bitsRemaining;

    /* If the current word doesn't have enough bits, read the next one */
    if (extractRemainingBits > 0)
    {
        /* Wait until the next word is available (FIFO not empty) */
        while ((pCurrState->readPtr == *pCurrState->pWritePtr) &&
               (!(pBitRipperState->ctrlFlags & BITRIPPER_FIFO_FULL_FLAG)))
        {
			/* Refill FIFO if needed */
            HAOS::fillInputFIFO();
        }

        /* Load the next word into currentWord */
        pCurrState->currentWord = *pCurrState->readPtr++;
        pBitRipperState->ctrlFlags &= BITRIPPER_FIFO_FULL_CLR;

        /* Wrap read pointer if end of FIFO is reached */
        if (pCurrState->readPtr == pCurrState->endAddrPlus1)
        {
            pCurrState->readPtr = pCurrState->baseAddr;
        }

        /* Extract the remaining bits from the newly loaded word */
        bitsNeeded = extractRemainingBits;
        result |= pCurrState->currentWord >> (MAX_BITS - bitsNeeded);

        /* Reset bitsRemaining after loading new word */
        pCurrState->bitsRemaining = MAX_BITS;
    }

    /* Shift currentWord to discard the extracted bits */
    pCurrState->currentWord <<= bitsNeeded;

    /* Decrease bitsRemaining accordingly */
    pCurrState->bitsRemaining -= bitsNeeded;

    /* If no bits are left, clear the word */
    if (pCurrState->bitsRemaining == 0)
        pCurrState->currentWord = 0;

    return result;
}


/*
 * Peeks the specified number of bits from the BitRipper input stream without advancing the read pointer.
 *
 * This function allows non-destructive reading of `bitsNeeded` bits (1–32) from the current input position.
 * It behaves similarly to `extractBits()` but does not modify the BitRipper state. If the requested bits
 * span across two FIFO words, the second word is temporarily read but not committed to the decoder state.
 *
 * This is useful in parsing situations where upcoming bits must be examined before a decoding decision
 * is made, without consuming the input stream.
 *
 * @param bitsNeeded Number of bits to peek (must be between 1 and MAX_BITS).
 * @return Extracted bits packed into the least significant bits of the return value.
 */
uint32_t peek(uint32_t bitsNeeded)
{
    /* bitsNeeded must be in the range [1, MAX_BITS] */
    assert((bitsNeeded >= 1) && (bitsNeeded <= MAX_BITS) && "Invalid peek bits argument!");

    /* Get BitRipper core and current state for the active HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();
    pBitRipper_t pCurrState = &pBitRipperState->currState;

    /* Extract the first part from the current word */
    uint32_t result = pCurrState->currentWord >> (MAX_BITS - bitsNeeded);
    int32_t peekRemainingBits = bitsNeeded - pCurrState->bitsRemaining;

    /* If more bits are needed beyond current word */
    if (peekRemainingBits > 0)
    {
        /* Wait until FIFO has data */
        while ((pCurrState->readPtr == *pCurrState->pWritePtr) &&
               (!(pBitRipperState->ctrlFlags & BITRIPPER_FIFO_FULL_FLAG)))
        {
            /* Refill FIFO if needed */
            HAOS::fillInputFIFO();
        }

        /* Load the next word for temporary inspection */
        uint32_t currentWord = *pCurrState->readPtr;

        /* Extract the remaining bits from the next word */
        bitsNeeded = peekRemainingBits;
        result |= currentWord >> (MAX_BITS - bitsNeeded);
    }

    return result;
}



/*
 * Skips a specified number of bits in the BitRipper input stream.
 *
 * This function moves the read pointer forward or backward by the specified number
 * of bits. It updates the internal state (`readPtr`, `currentWord`, and `bitsRemaining`)
 * to reflect the new bitstream position. Both positive (forward) and negative (rewind)
 * skips are supported.
 *
 * When skipping forward, the FIFO is refilled as needed (ignoring auxiliary state locking),
 * and the pointer wrapping is handled for both forward and backward cases. The resulting
 * state is consistent with the new bit-aligned input position.
 *
 * @param skipBits Signed number of bits to skip. Positive value skips forward,
 *                 negative value skips backward.
 */
void skipBits(int32_t skipBits)
{

    /* Get BitRipper core and current state for the active HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();
    pBitRipper_t pCurrState = &pBitRipperState->currState;

	/* Forward skipping is allowed only up to FIFO_SIZE * MAX_BITS */
	assert(skipBits <= pCurrState->size * MAX_BITS && "Invalid argument in skipBits() routine!");

	/* No skipping needed */
	if (!skipBits)
	{
		return;
	}

	/* Backup current read pointer and bit count */
	uint32_t *newReadPtr = pCurrState->readPtr;
	int32_t newBitsRemaining = pCurrState->bitsRemaining;

	if (skipBits > 0)
	{
		/* Save and clear AUX flag (we refill FIFO using current RdPtr) */
		HAOS_CtrlFlags_t auxFlagBckp = pBitRipperState->ctrlFlags & BITRIPPER_IN_AUX_STATE_FLAG;
		pBitRipperState->ctrlFlags &= BITRIPPER_IN_AUX_STATE_CLR;

		/* Wait until FIFO contains enough bits to skip */
		while (skipBits > readDipstick())
		{
			HAOS::fillInputFIFO();
		}

		/* Restore previous AUX flag */
		pBitRipperState->ctrlFlags |= auxFlagBckp;

		/* Calculate how many words and bits to skip */
		uint32_t bitsToSkip = skipBits & (MAX_BITS - 1);   /* skipBits % 32 */
		uint32_t wordsToSkip = skipBits >> 5;              /* skipBits / 32 */

		newReadPtr += wordsToSkip;
		newBitsRemaining -= bitsToSkip;

		/* If underflow in bit count, skip an extra word */
		if (newBitsRemaining < 0)
		{
			newBitsRemaining += MAX_BITS;
			newReadPtr++;
		}

		/* Wrap pointer if exceeded FIFO bounds */
		if (newReadPtr >= pCurrState->endAddrPlus1)
		{
			newReadPtr -= pCurrState->size;
		}
	}
	else
	{
		/* Negative skipping (rewind) */
		skipBits = -skipBits;

		uint32_t bitsToSkip = skipBits & (MAX_BITS - 1);
		uint32_t wordsToSkip = skipBits >> 5;

		newReadPtr -= wordsToSkip;
		newBitsRemaining += bitsToSkip;

		/* If overflow in bit count, go back one word */
		if (newBitsRemaining > MAX_BITS)
		{
			newBitsRemaining -= MAX_BITS;
			newReadPtr--;
		}

		/* Wrap pointer if underflowed below base */
		if (newReadPtr < pCurrState->baseAddr)
		{
			newReadPtr += pCurrState->size;
		}

		if ( (!(newBitsRemaining & 0x1f)) && (newReadPtr - (newBitsRemaining >> 5) == *pCurrState->pWritePtr)  )
		{
			pBitRipperState->ctrlFlags |= BITRIPPER_FIFO_FULL_FLAG;
		}

	}

	/* Fetch the word from which the new read begins */


	uint32_t newCurrentWord = 0;

	if (newBitsRemaining)
	{
		uint32_t *newCurrWordPtr = newReadPtr;

		newCurrWordPtr--;
		/* Wrap if underflow */
		if (newCurrWordPtr < pCurrState->baseAddr)
		{
			newCurrWordPtr += pCurrState->size;
		}

		/* Shift word left to align with new bit offset */
		newCurrentWord = *newCurrWordPtr;
		newCurrentWord <<= MAX_BITS - newBitsRemaining;
	}


	/* Commit new state */
	pCurrState->currentWord = newCurrentWord;
	pCurrState->bitsRemaining = newBitsRemaining;
	pCurrState->readPtr = newReadPtr;

}


/*
 * Returns the number of bits available in the input FIFO.
 *
 * This function calculates how many bits are currently available to be read from the
 * input FIFO by evaluating the distance between the current read and write pointers,
 * taking into account the number of bits remaining in the current read word.
 * If the BitRipper is operating in auxiliary state, the backup main state is used instead.
 *
 * @return Number of bits available in the FIFO buffer.
 */
uint32_t readDipstick()
{
	int32_t bitsInFIFO = 0;

	/* Get the active BitRipper core instance for the current HaOS core */
	pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

	/* Get a pointer to the current state of the active BitRipper */
	pBitRipper_t pCurrState = &pBitRipperState->currState;

	if (pBitRipperState->ctrlFlags & BITRIPPER_IN_AUX_STATE_FLAG)
	{
		/* Use main backup state if in AUX state */
		pCurrState = &pBitRipperState->mainStateBackup;
	}

	/* Compute number of full words between write and read pointers */
	bitsInFIFO = *pCurrState->pWritePtr - pCurrState->readPtr;

	if ((bitsInFIFO < 0) ||
		((!bitsInFIFO) && (pBitRipperState->ctrlFlags & BITRIPPER_FIFO_FULL_FLAG)))
	{
		/* Handle wrap-around of circular buffer */
		bitsInFIFO += pCurrState->size;
	}

	/* Convert word count to bit count */
	bitsInFIFO *= MAX_BITS;

	/* Add remaining bits in current read word */
	if (pCurrState->bitsRemaining)
	{
		bitsInFIFO += pCurrState->bitsRemaining;
	}

	return bitsInFIFO;
}




/*
 * Waits until the input FIFO contains at least the requested number of bits.
 *
 * This function monitors the number of valid bits currently present in the input FIFO,
 * using the `readDipstick()` function to determine the fill level. It blocks execution until the
 * specified threshold (`bitsRequired`) is reached.
 *
 * If the number of available bits is below the required level, the function triggers FIFO refilling
 * via `refillFIFO()` and rechecks the condition in a polling loop.
 *
 * The function asserts that `bitsRequired` does not exceed the total FIFO capacity (FIFO_SIZE * MAX_BITS),
 * ensuring that the request is valid and physically achievable.
 *
 * @param bitsRequired Minimum number of bits that must be present in the FIFO before continuing.
 */
void waitOnDipstick(uint32_t bitsRequired)
{
	/* Get the active BitRipper core instance for the current HaOS core */
	pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

	/* Get a pointer to the current state of the active BitRipper */
	pBitRipper_t pCurrState = &pBitRipperState->currState;

	 /* Requested number of bits must not exceed total FIFO capacity */
	assert(bitsRequired <= pCurrState->size * MAX_BITS && "Invalid requested dipstick depth!");

	/* Wait until FIFO has at least 'bitsRequired' bits available */
	while (bitsRequired > readDipstick())
	{
		/* Refill FIFO */
		HAOS::fillInputFIFO();
	}
}


/*
 * Freezes the FIFO read pointer by backing up the current BitRipper state.
 *
 * This function freezes the state of the main read pointer in the BitRipper,
 * allowing parsing routines to operate on the input stream without affecting
 * the primary decoding flow. It copies the current BitRipper state into the
 * `mainStateBackup` structure and sets the control flag indicating that the
 * system is now operating in an auxiliary state.
 *
 * Used when temporary bitstream inspection is required, such as CRC
 * routines, ensuring the original decoding position remains unaffected.
 */
void saveMainState()
{
    /* Get the active BitRipper core instance for the current HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Get a pointer to the current state of the active BitRipper */
    pBitRipper_t pCurrState = &pBitRipperState->currState;

    /* Get a pointer to the structure used for backing up the main BitRipper state */
    pBitRipper_t pMainStateBckp = &pBitRipperState->mainStateBackup;

    /* Save the current state into the backup location (excluding write pointers) */
    memcpy(pMainStateBckp, pCurrState, sizeof(BitRipper_t));

    /* Set control flag indicating the system is now operating in auxiliary mode */
    pBitRipperState->ctrlFlags |= BITRIPPER_IN_AUX_STATE_FLAG;
}


/*
 * Restores the previously backed-up FIFO read pointer state.
 *
 * This function restores the main BitRipper state (`currState`) from the backup
 * structure (`mainStateBackup`). It effectively resets the decoder's read position
 * to the point where `saveMainState()` was previously invoked.
 *
 * After restoring, the auxiliary state flag is cleared, signaling the end of
 * temporary parsing and the resumption of normal decoding flow.
 *
 * Typically used after speculative or non-destructive bitstream inspection
 * (e.g., CRC checks or sync probing), to return the system to its original state.
 */
void restoreMainState()
{
    /* Get the active BitRipper core instance for the current HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Get a pointer to the current state of the active BitRipper */
    pBitRipper_t pCurrState = &pBitRipperState->currState;

    /* Get a pointer to the structure used for backing up the main BitRipper state */
    pBitRipper_t pMainStateBckp = &pBitRipperState->mainStateBackup;

    /* Restore FIFO read pointer and other relevant state fields (excluding write pointers) */
    memcpy(pCurrState, pMainStateBckp, sizeof(BitRipper_t));

    /* Clear the AUX state flag to resume normal decoding */
    pBitRipperState->ctrlFlags &= BITRIPPER_IN_AUX_STATE_CLR;
}


/*
 * Loads an auxiliary BitRipper state as the new main state.
 *
 * This function replaces the current BitRipper state (`currState`) with the
 * state provided via the `pAuxState` pointer. This enables external routines
 * to set a custom decoding position and state context, effectively overriding
 * the default decoder progress.
 *
 * After loading the new state, the auxiliary mode flag is cleared to indicate
 * the system has exited temporary or speculative parsing.
 *
 * @param pAuxState Pointer to the BitRipper_t structure containing the alternate state.
 */
void loadMainState(pBitRipper_t pAuxState)
{
    /* Get the active BitRipper core instance for the current HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Get a pointer to the current state of the active BitRipper */
    pBitRipper_t pCurrState = &pBitRipperState->currState;

    /* Overwrite current state with auxiliary input state (excluding write pointers) */
    memcpy(pCurrState, pAuxState, sizeof(BitRipper_t));

    /* Clear the AUX state flag to resume normal decoding */
    pBitRipperState->ctrlFlags &= BITRIPPER_IN_AUX_STATE_CLR;
}


/*
 * Saves the current BitRipper state into an auxiliary structure.
 *
 * This function copies the current active BitRipper state into the structure
 * pointed to by `pAuxState`. This allows the decoder to preserve its current
 * bitstream parsing context and resume it later, for example after performing
 * temporary inspection or alternate parsing paths.
 *
 * @param pAuxState Pointer to a BitRipper_t structure where the current state will be saved.
 */
void saveAuxState(pBitRipper_t pAuxState)
{
    /* Get the active BitRipper core instance for the current HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Get a pointer to the current state of the active BitRipper */
    pBitRipper_t pCurrState = &pBitRipperState->currState;

    /* Copy current BitRipper state into the auxiliary structure (excluding write pointers) */
    memcpy(pAuxState, pCurrState, sizeof(BitRipper_t));
}


/*
 * Loads auxiliary BitRipper state into the active decoder context
 *
 * This function replaces the current BitRipper state with a previously saved
 * auxiliary state, excluding the write pointer fields. It is typically used to
 * resume decoding or parsing from an alternative position without disrupting
 * the main decoding flow. Only the portion of the state relevant for read
 * operations is copied to preserve system consistency.
 *
 * @param pAuxState Pointer to the auxiliary BitRipper state structure
 */
void loadAuxState(pBitRipper_t pAuxState)
{
    /* Get the active BitRipper core instance for the current HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Get a pointer to the current state of the active BitRipper */
    pBitRipper_t pCurrState = &pBitRipperState->currState;

    /* Copy auxiliary state into current BitRipper state (excluding write pointers) */
    memcpy(pCurrState, pAuxState, sizeof(BitRipper_t));
}


/*
 * Checks whether the BitRipper is currently operating in auxiliary state.
 *
 * This function reads the control flag BITRIPPER_IN_AUX_STATE_FLAG from the BitRipper
 * instance associated with the active HaOS core. It returns true if the system is currently
 * using an auxiliary state for bitstream parsing (e.g., during temporary inspection),
 * indicating that the main decoding state is frozen.
 *
 * @return true if auxiliary state is active, false otherwise.
 */
bool getAuxStateFlag()
{
	/* Get the active BitRipper core instance for the current HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

	return (pBitRipperState->ctrlFlags & BITRIPPER_IN_AUX_STATE_FLAG);
}


/*
 * Calculates the bit distance between two BitRipper states.
 *
 * This function computes the number of bits between two BitRipper parsing states:
 * `pStateStart` and `pStateEnd`. It accounts for both the byte-level offset between
 * the `readPtr` pointers and the bit-level offset within the individual 32-bit words.
 * If the read pointer has wrapped around the FIFO, the offset is corrected accordingly.
 *
 * The result represents how many bits would be consumed by the BitRipper in order to
 * advance from `pStateStart` to `pStateEnd`.
 *
 * @param pStateStart Pointer to the starting BitRipper state.
 * @param pStateEnd   Pointer to the ending BitRipper state.
 * @return            Number of bits between the two states.
 */
int32_t bitCntStates(pBitRipper_t pStateStart, pBitRipper_t pStateEnd)
{
	/* Calculate the bit difference within the two current 32-bit words
	 * Start may have bits remaining, while end has already consumed some bits */
	int32_t bitsInCurrWords = pStateStart->bitsRemaining - pStateEnd->bitsRemaining;


	/* Calculate the offset in 32-bit words between the two read pointers */
	int32_t readPtrsOffset = pStateEnd->readPtr - pStateStart->readPtr;

	/* Handle FIFO wrap-around by adjusting the offset to be positive */
	if (readPtrsOffset < 0)
	{
		readPtrsOffset += pStateStart->size;
	}

	/* Total bit difference is word offset multiplied by bits per word plus bit difference in the current words */
	int32_t retValue = readPtrsOffset * MAX_BITS + bitsInCurrWords;

	/* Return the total number of bits between start and end state */
	return retValue;
}

/*
 * Calculates the number of bits between the main BitRipper state and a given end state.
 *
 * This function computes the bitwise distance between the main decoding position (read pointer and bit offset)
 * and a specified auxiliary (end) state. If the BitRipper is currently operating in auxiliary mode,
 * it uses the backed-up main state as the starting reference.
 *
 * @param pStateEnd Pointer to the end BitRipper state to compare against.
 * @return Number of bits between the main state and the given end state.
 */
int32_t bitCntMainState(pBitRipper_t pStateEnd)
{
    /* Get the BitRipper state for the currently active HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Start from the current state of the BitRipper */
    pBitRipper_t pStateStart = &pBitRipperState->currState;

    /* If in AUX state, use the backed-up main state as the reference point */
    if (pBitRipperState->ctrlFlags & BITRIPPER_IN_AUX_STATE_FLAG)
    {
        pStateStart = &pBitRipperState->mainStateBackup;
    }

    /* Delegate to bitCntStates() to compute the bitwise distance */
    return bitCntStates(pStateStart, pStateEnd);
}

/*
 * Saves the bitstream alignment offset for future reference.
 *
 * This function computes the bit-level alignment between the current read pointer
 * position in the BitRipper and a bit located at a signed offset (`bitsOffset`)
 * from the current bit position. The result represents how far from an aligned
 * boundary (e.g., 8, 16, or 31 bits) the referenced position is.
 *
 * The computed alignment is stored in the `alignmentInfo` field,
 * which may be used later for restoring alignment or validating bit boundaries.
 *
 * @param bitsOffset Signed number of bits ahead or behind the current bit position
 *                   used to compute alignment relative to that position.
 */
void saveAlignment(int32_t bitsOffset)
{
    /* Get the BitRipper state for the currently active HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Get a pointer to the current BitRipper state */
    pBitRipper_t pCurrState = &pBitRipperState->currState;

    /* Calculate the alignment offset relative to the current position and target bit */
    int32_t alignment = pCurrState->bitsRemaining - bitsOffset;

    /* Take absolute value if result is negative */
    if (alignment < 0)
    {
        alignment = -alignment;
    }

    /* Keep alignment within one word (MAX_BITS = bits per FIFO word) */
    alignment &= MAX_BITS - 1;

    /* Store the computed alignment value for later use */
    pBitRipperState->alignmentInfo = alignment;
}


/*
 * Computes the number of bits required to reach the previously saved alignment position.
 *
 * This function is used internally by alignment routines (e.g., alignToByte(), alignToWord())
 * to determine how many bits must be skipped in order to restore alignment relative to a previously
 * saved reference point. The alignment reference is stored in the BitRipper core’s `alignmentInfo` field,
 * and the result represents the offset (in bits) needed to realign the bitstream.
 *
 * @return Number of bits to skip from the current position to reach the saved alignment boundary.
 */
static int32_t alignCommon()
{
    /* Get the BitRipper state for the currently active HaOS core */
    pBitRipperState_t pBitRipperState = (pBitRipperState_t)HAOS::getActiveCoreBitRipper();

    /* Get a pointer to the current BitRipper state */
    pBitRipper_t pCurrState = &pBitRipperState->currState;

    /* Calculate how many bits must be skipped to restore alignment */
    int32_t retValue = pCurrState->bitsRemaining + MAX_BITS - pBitRipperState->alignmentInfo;

    return retValue;
}


/*
 * Aligns the BitRipper input stream to the nearest 8-bit boundary.
 *
 * This routine adjusts the BitRipper's internal state by skipping the minimal number of bits
 * necessary to reach the next byte-aligned position, based on the alignment reference previously
 * saved using `saveAlignment()`. It uses `alignCommon()` to compute the bit distance from the
 * saved alignment point and ensures alignment to an 8-bit boundary by applying a mask.
 *
 * Typically used before parsing byte-aligned data structures to ensure correct field extraction.
 */
void alignToByte()
{
    /* Calculate the bit distance between current position and the saved alignment reference */
    int32_t bitsToSkip = alignCommon();

    /* Mask the result to compute how many bits to skip to reach the next 8-bit boundary (0–7 bits) */
    bitsToSkip &= 8 - 1;

    /* Skip the computed number of bits in the input stream to achieve byte alignment */
    skipBits(bitsToSkip);
}


/*
 * Aligns the BitRipper input stream to a 16-bit boundary.
 *
 * This function adjusts the current input pointer so that the BitRipper state is aligned
 * to the next 16-bit (2-byte) boundary relative to the alignment position saved using
 * saveAlignment(). It computes the required number of bits to skip by comparing
 * the current bit position with the stored alignment reference and masks the result to
 * ensure alignment to a 16-bit boundary.
 *
 * This is typically used when word-aligned access is needed during parsing of input streams.
 */
void alignToWord()
{
    /* Compute bit distance to saved alignment reference */
    int32_t bitsToSkip = alignCommon();

    /* Limit skip to the range [0–15] to reach next 16-bit aligned position */
    bitsToSkip &= 16 - 1;

    /* Skip the required number of bits to achieve 16-bit alignment */
    skipBits(bitsToSkip);
}


/*
 * Aligns the BitRipper input stream to a 32-bit (DWord) boundary.
 *
 * This function adjusts the current read position so that the BitRipper stream becomes aligned
 * to the next 32-bit boundary with respect to the reference position saved via SaveAlignment().
 * It calculates the number of bits to skip using the common alignment offset and masks the result
 * to round up to the nearest 32-bit aligned position.
 *
 * Typically used when parsing structures or data types that require natural 4-byte alignment.
 */
void alignToDWord()
{
    /* Calculate relative offset to alignment reference */
    int32_t bitsToSkip = alignCommon();

    /* Mask to determine bits needed to reach next 32-bit boundary */
    bitsToSkip &= 32 - 1;

    /* Skip the required number of bits to align to 32-bit boundary */
    skipBits(bitsToSkip);
}


}



