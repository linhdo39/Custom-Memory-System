//-----------------------------------------------------------------------------
// Copyright 2022, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#ifndef MEM_H
#define MEM_H

#include "Heap.h"

class Mem
{
public:
	enum class Guard
	{
		Type_A,
		Type_5
	};

public:

	Mem(Guard type);

	Mem() = delete;
	Mem(const Mem&) = delete;
	Mem& operator = (const Mem&) = delete;
	~Mem();

	Heap* GetHeap();
	void Print(int count);

	// implement these functions
	void free(void* const data);
	void* malloc(const uint32_t size);
	void initialize();
private:
	Free* findFreeBlock(const uint16_t size);
	void RemoveFreeBlock(const Free* pFree) const;
	void AdjustRemoveFreeBlock(const Free* pFree) const;
	void AdjustFreePointer(Free* pFree) const;
	void RemoveUsedBlock(const Used* pUsed) const;
	void AdjustRemoveUsedBlock(const Used* pUsed) const;
	void AddUsedBlock(Used* pUsed) const;
	void AdjustAddUsedBlock(const Used* pUsed) const;
	Used* FindLastUsedBlock();
private:
	// Useful in malloc and free
	Heap* poHeap;
	void* pTopAddr;
	void* pBottomAddr;

	// Below: 
	//    Not used in malloc, free,or inialize
	Guard type;
	void* poRawMem;
};

#endif 

// ---  End of File ---

