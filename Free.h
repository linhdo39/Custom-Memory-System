//-----------------------------------------------------------------------------
// Copyright 2022, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#ifndef FREE_H
#define FREE_H

#include "Block.h"

class Used;

class Free
{
public:
    // Big four
    Free() = delete;
    Free(const Free&) = delete;
    Free& operator =(const Free&) = delete;
    ~Free() = default;
    Free(uint16_t BlockSize);

public:
    // ---------------------------------------------------------
    // Do not reorder, change or add data fields
    //     --> any changes to the data... 0 given to the assignment
    // You can add methods if you wish
    // ---------------------------------------------------------

    Free     *pFreeNext;        // next free block
    Free     *pFreePrev;        // prev free block    
    uint16_t   mBlockSize;       // size of block (16 bit)
    Block      mType;            // block type 
    bool       mAboveBlockFreeFlag;  // AboveBlock flag:
                                     // if(AboveBlock is type free) -> true 
                                     // if(AboveBlock is type used) -> false
};

#endif 

// ---  End of File ---
