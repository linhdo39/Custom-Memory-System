//-----------------------------------------------------------------------------
// Copyright 2022, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#include "Used.h"
#include "Free.h"
#include "Block.h"

// Add magic here
Free::Free(uint16_t BlockSize)
	: pFreeNext(nullptr),
	pFreePrev(nullptr),
	mBlockSize(BlockSize),
	mType(Block::Free),
	mAboveBlockFreeFlag(false)
{
}

// ---  End of File ---
