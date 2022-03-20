# Custom-Memory-System
Created a memory system framework for the heap layout resulting in a faster memory system in a controlled stress test.
This project was completed using original template structure (1) from a class project (Optimize C++). The logic and implementation were done and completed by me. 

<b> Implementation
- Overloaded New/Delete operators for heap assignment and alignment specification.
- Added allocators (implemented with next fit algorithm) and de-allocators for the memory system.
- Implemented secret pointers, memory flags, and headers to identify each allocated memory block.
  
<b> Outcome
- Two times faster memory system compare to the built-in memory systems
- Passed the stressed test with over 100 random memory allocation and de-allocation operations.
  
(1) Copyright 2022, Ed Keenan, all rights reserved.
