//-----------------------------------------------------------------------------
// Copyright 2022, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#include "Free.h"
#include "Used.h"

// Add code here
Used::Used(uint16_t BlockSize)
	: pUsedNext(nullptr),
	pUsedPrev(nullptr),
	mBlockSize(BlockSize),
	mType(Block::Used),
	mAboveBlockFreeFlag(false)
{
}


// ---  End of File ---

