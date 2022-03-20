//-----------------------------------------------------------------------------
// Copyright 2022, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------- 

#include "Mem.h"
#include "Heap.h"
#include "Block.h"

// To help with coalescing... not required
struct SecretPtr
{
	Free* pFree;
};

Free* Mem::findFreeBlock(const uint16_t size)
{
	unsigned int count = 0;
	Free* pFreeTemp = this->poHeap->pNextFit;
	Free* pFreeFound = nullptr;
	while (pFreeTemp != nullptr) {
		if (pFreeTemp->mBlockSize >= size) {
			pFreeFound = pFreeTemp;
			break;
		}
		pFreeTemp = pFreeTemp->pFreeNext;
		if (count < 1 && pFreeTemp == nullptr) {
			pFreeTemp = this->poHeap->pFreeHead;
			count++;
		}
	}
	return pFreeFound;
}

void Mem::RemoveFreeBlock(const Free* pFree) const
{
	assert(pFree);
	Free* freeNext = pFree->pFreeNext;
	Free* freePrev = pFree->pFreePrev;
	if (freeNext == nullptr && freePrev == nullptr) {
		this->poHeap->pFreeHead = nullptr;
	}
	else if (freePrev != nullptr) {
		pFree->pFreePrev->pFreeNext = freeNext;
		if (freeNext != nullptr) {
			pFree->pFreeNext->pFreePrev = freePrev;
		}
	}
}

void Mem::AdjustRemoveFreeBlock(const Free* pFree) const
{
	assert(pFree);
	this->poHeap->currFreeMem -= pFree->mBlockSize;
	this->poHeap->currNumFreeBlocks -= 1;
}

void Mem::RemoveUsedBlock(const Used* pUsed) const
{
	assert(pUsed);
	Used* usedNext = pUsed->pUsedNext;
	Used* usedPrev = pUsed->pUsedPrev;
	if (pUsed->pUsedNext == nullptr && pUsed->pUsedPrev == nullptr && this->poHeap->currNumFreeBlocks <=1) {
		this->poHeap->pUsedHead = nullptr;
		return;
	}
	if (usedNext != nullptr) {
		usedNext->pUsedPrev = usedPrev;
		if(this->poHeap->pUsedHead == nullptr || this->poHeap->pUsedHead == pUsed)
			this->poHeap->pUsedHead = usedNext;
	}
	if (usedPrev != nullptr) {
		usedPrev->pUsedNext = usedNext;
		return;
	}
}

void Mem::AdjustRemoveUsedBlock(const Used* pUsed) const
{
	assert(pUsed);
	this->poHeap->currUsedMem -= pUsed->mBlockSize;
	this->poHeap->currNumUsedBlocks -= 1;
}

void Mem::AddUsedBlock(Used* pUsed) const
{
	assert(pUsed);
	//add to the front of the list
	pUsed->pUsedNext = nullptr;
	pUsed->pUsedPrev = nullptr;
	this->poHeap->pUsedHead = pUsed;
}

void Mem::AdjustAddUsedBlock(const Used* pUsed) const
{
	assert(pUsed);
	this->poHeap->currNumUsedBlocks += 1;
	this->poHeap->currUsedMem += pUsed->mBlockSize;
	if(this->poHeap->currNumUsedBlocks > this->poHeap->peakNumUsed)
		this->poHeap->peakNumUsed = this->poHeap->currNumUsedBlocks;
	if(this->poHeap->currUsedMem > this->poHeap->peakUsedMemory)
		this->poHeap->peakUsedMemory = this->poHeap->currUsedMem;
}

Used* Mem::FindLastUsedBlock()
{
	Used* temp = this->poHeap->pUsedHead;
	if (temp == nullptr) {
		return nullptr;
	}
	while (temp->pUsedPrev != nullptr) {
		temp = temp->pUsedPrev;
	}
	return temp;
}

void Mem::AdjustFreePointer(Free* pFree) const {
	if (this->poHeap->pFreeHead == pFree) {
		return;
	}
	if (this->poHeap->pFreeHead != nullptr) {
		if (pFree < this->poHeap->pFreeHead) {
			this->poHeap->pFreeHead->pFreePrev = pFree;
			pFree->pFreeNext = this->poHeap->pFreeHead;
			this->poHeap->pFreeHead = pFree;
		}
		else {
			Free* temp = this->poHeap->pFreeHead->pFreeNext;
			while (pFree > temp) {
				if (temp == nullptr) {
					pFree->pFreeNext = nullptr;
					pFree->pFreePrev = nullptr;
					return;
				}
				temp = temp->pFreeNext;
			}
			pFree->pFreePrev = temp->pFreePrev;
			pFree->pFreeNext = temp;
			if (temp->pFreePrev != nullptr) {
				temp->pFreePrev->pFreeNext = pFree;
				temp->pFreePrev = pFree;
			}
		}

	}
	else {
		this->poHeap->pFreeHead = pFree;
		this->poHeap->pFreeHead->pFreePrev = nullptr;
		this->poHeap->pNextFit = pFree;
	} 
}

// ----------------------------------------------------
//  Initialized the Memory block:
//  Update the Heap
//  Create a free block
//  Heap structure links to free hdr
//-----------------------------------------------------
void Mem::initialize()
{
	Heap* heapTemp = this->GetHeap();
	Free* pFreeStart = (Free*)(heapTemp + 1);
	Free* pFreeEnd = pFreeStart + 1;
	uint32_t blockSize = ((uint32_t)this->pBottomAddr - (uint32_t)pFreeEnd);
	Free* pFree = new(pFreeStart) Free((uint16_t)blockSize);		
	uint32_t* ptr = (uint32_t*)((uint32_t)this->pBottomAddr - 4);
	heapTemp->pFreeHead = pFree;
	heapTemp->pNextFit = pFree;
	heapTemp->currFreeMem = (uint16_t)blockSize;
	heapTemp->currNumFreeBlocks = 1;
	*ptr = (uint32_t&)this->poHeap->pFreeHead;
}


// ----------------------------------------------------
//  Do your Magic here:
//  Find a free block that fits
//  Change it to used (may require subdivision)
//  Correct the heap Links (used,free) headers 
//  Update stats
//  Return pointer to block
//-----------------------------------------------------
void* Mem::malloc(const uint32_t _size)
{
	Free* pFree = findFreeBlock((uint16_t)_size);
	Used* temp = this->FindLastUsedBlock();
	void* pUsedBlock = nullptr;
	bool head = false;
	if (pFree != nullptr) {
		if (pFree->mBlockSize == (uint16_t)_size) {
			this->poHeap->pNextFit = pFree->pFreeNext;
			this->RemoveFreeBlock(pFree);
			this->AdjustRemoveFreeBlock(pFree);
			Used* pUsed = new(pFree) Used((uint16_t)_size);
			this->AddUsedBlock(pUsed);
			this->AdjustAddUsedBlock(pUsed);
			uint32_t pUsedBlockEnd = (uint32_t)pUsed + (uint32_t)pUsed->mBlockSize;
			Used* pCheckStart = (Used*)(pUsedBlockEnd);
			Used* pCheckEnd = pCheckStart + 1;
			if (pCheckEnd->mAboveBlockFreeFlag == true) {
				pCheckEnd->mAboveBlockFreeFlag = false;
			}
			pUsedBlock = (void*)(pUsed + 1);
			if (temp != nullptr) {
				temp->pUsedPrev = pUsed;
				pUsed->pUsedNext = temp;
			}
		}
		else {
			if (pFree == this->poHeap->pFreeHead) {
				head = true;
			}
			Used* pUsed = nullptr;
			uint32_t FreeSize = (uint32_t)pFree->mBlockSize - _size - sizeof(Used);
			Free* prevFree = pFree->pFreePrev;
			Free* nextFree = pFree->pFreeNext;
			
			pUsed = new(pFree) Used((uint16_t)_size);
			this->AddUsedBlock(pUsed);
			if (temp != nullptr) {
				temp->pUsedPrev = pUsed;
				pUsed->pUsedNext = temp;
			}
			this->AdjustAddUsedBlock(pUsed);
			pUsedBlock = (void*)(pUsed + 1);
			uint32_t pUsedBlockEnd = (uint32_t)pUsed + sizeof(Used) + pUsed->mBlockSize;
			this->poHeap->currFreeMem -=(pUsed->mBlockSize+sizeof(Used));
			Free* pFreeStart = (Free*)(pUsedBlockEnd);
			Free* pFreeEnd = pFreeStart + 1;
			pFree = new(pFreeStart) Free((uint16_t)FreeSize);
			this->poHeap->pNextFit = pFree;
			if (this->poHeap->currNumFreeBlocks == 1) {
				this->poHeap->pFreeHead = pFree;
			}
			else {
				pFree->pFreeNext = nextFree;
				pFree->pFreePrev = prevFree;
				if (prevFree != nullptr)
					pFree->pFreePrev->pFreeNext = pFree;
				if(head)
					this->poHeap->pFreeHead = pFree;
				if (nextFree == nullptr) {
					this->poHeap->pNextFit = pFree;
				}
				else {
					pFree->pFreeNext->pFreePrev = pFree;
				}				
			}
			SecretPtr* ptr = (SecretPtr*)((uint32_t)pFreeEnd + (uint32_t)pFree->mBlockSize - 4);
			ptr->pFree = pFree;
		}
	}
	else {
		return nullptr;
	}
	
	return pUsedBlock;
}

// ----------------------------------------------------
//  Do your Magic here:
//  Return the free block to the system
//  May require Coalescing
//  Correct the heap Links (used,free) headers 
//  Update stats
//-----------------------------------------------------
void Mem::free(void* const data)
{
	Used* pUsed = (Used*)data;
	pUsed = pUsed - 1;
	bool aboveFree = pUsed->mAboveBlockFreeFlag;
	Used* prevUsed = pUsed->pUsedPrev;
	bool lastUsed = false;
	if (pUsed->pUsedNext == nullptr) {
		lastUsed = true;
	}
	this->RemoveUsedBlock(pUsed);
	this->AdjustRemoveUsedBlock(pUsed);
	Free* pFree = new(pUsed) Free(pUsed->mBlockSize);
	SecretPtr s;
	s.pFree = pFree;
	uint32_t pFreeBlockEnd = (uint32_t)pFree + (uint32_t)pFree->mBlockSize;
	Free* pCheckStart = (Free*)(pFreeBlockEnd);
	Free* pCheckEnd = pCheckStart + 1;
	SecretPtr* secretptr = (SecretPtr*)((uint32_t)pFree - 4);

	this->poHeap->currNumFreeBlocks++;
	SecretPtr* ptr = (SecretPtr*)((uint32_t)pCheckEnd  - 4);

	//check previous block
	if (aboveFree == true) {
		if (pCheckEnd->mType == Block::Free) {
			this->poHeap->currFreeMem += (pFree->mBlockSize + sizeof(Free));
			ptr = (SecretPtr*)((uint32_t)pCheckEnd + pCheckEnd->mBlockSize + sizeof(Free) - 4);
			pFree->mBlockSize += (pCheckEnd->mBlockSize + sizeof(Free));
			secretptr->pFree->mBlockSize += (pFree->mBlockSize + sizeof(Free));
			this->poHeap->currFreeMem += sizeof(Free);
			this->poHeap->currNumFreeBlocks-=2;
			secretptr->pFree->pFreeNext = pCheckEnd->pFreeNext;
		}
		else {
			secretptr->pFree->mBlockSize += (pFree->mBlockSize + sizeof(Free));
			if (pFree < this->poHeap->pFreeHead || this->poHeap->pFreeHead == nullptr) {
				this->poHeap->currFreeMem += pFree->mBlockSize;
			}
			this->poHeap->currFreeMem += (pFree->mBlockSize + sizeof(Free));
			this->poHeap->currNumFreeBlocks--;
			if (prevUsed != nullptr)
				prevUsed->mAboveBlockFreeFlag = true;
			ptr->pFree = secretptr->pFree;
			return;
		}
	}
	//check next block
	else if (pCheckEnd->mType == Block::Free) {
		pFree->pFreeNext = pCheckEnd->pFreeNext;
		pFree->pFreePrev = pCheckEnd->pFreePrev;
		if (pFree->pFreeNext != nullptr) {
			pFree->pFreeNext->pFreePrev = pFree;
		}
		if (pFree < this->poHeap->pFreeHead) {
			this->poHeap->pFreeHead = pFree;
		}
		if (pFree->pFreePrev != nullptr) {
			pFree->pFreePrev->pFreeNext = pFree;
		}
		else
			this->poHeap->pNextFit = pFree;

		this->poHeap->currFreeMem += pFree->mBlockSize;
		this->poHeap->currFreeMem += sizeof(Free);
		if(pFree->pFreeNext> this->poHeap->pNextFit)
			this->poHeap->pNextFit = pFree->pFreeNext;
		ptr = (SecretPtr*)((uint32_t)pCheckEnd + pCheckEnd->mBlockSize + sizeof(Free)- 4);
		pFree->mBlockSize += (pCheckEnd->mBlockSize + sizeof(Free));
		this->poHeap->currNumFreeBlocks--;
	}
	else {
		if (prevUsed != nullptr)
			prevUsed->mAboveBlockFreeFlag = true;
		this->poHeap->currFreeMem += pFree->mBlockSize;
		this->AdjustFreePointer(pFree);
		
	}
	
	//reset pointer
	if (this->poHeap->currNumFreeBlocks == 1 ) {
		pFree->pFreeNext = nullptr;
		pFree->pFreePrev = nullptr;
		this->poHeap->pNextFit = this->poHeap->pFreeHead;
		if (this->poHeap->currNumUsedBlocks == 0) {
			this->poHeap->pUsedHead = nullptr;
		}
		if((uint32_t)this->poHeap->pUsedHead < (uint32_t)this->poHeap->pFreeHead)
			ptr = (SecretPtr*)((uint32_t)this->pBottomAddr - 4);
		ptr->pFree= this->poHeap->pFreeHead;
	}
	else
		ptr->pFree = s.pFree;
}


// ---  End of File ---
