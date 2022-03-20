//-----------------------------------------------------------------------------
// Copyright 2022, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#ifndef USED_H
#define USED_H

#include "Block.h"

class Free;

class Used
{
public:
    // Big four
    Used() = delete;
    Used(const Used&) = delete;
    Used& operator =(const Used&) = delete;
    ~Used() = default;
    Used(uint16_t BlockSize);
public:
   // ---------------------------------------------------------
   // Do not reorder, change or add data fields
   //     --> any changes to the data... 0 given to the assignment
   // You can add methods if you wish
   // ---------------------------------------------------------
    Used       *pUsedNext;       // next used block
    Used       *pUsedPrev;       // prev used block
    uint16_t   mBlockSize;       // size of block (16 bit)
    Block      mType;            // block type 

    bool       mAboveBlockFreeFlag;  // AboveBlock flag:
                                 // if(AboveBlock is type free) -> true 
                                 // if(AboveBlock is type used) -> false

};

#endif 

// ---  End of File ---

