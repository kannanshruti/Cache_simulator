# Cache_simulator
## Description:
- Code which simulates the working of a cache memory. The command line arguments for the cache include the cache size and its associativity. The code also accepts an input file, with each line containing a memory address and a boolean value representing 'Load' or 'Store' operation at that address. The output of the program is a count of the number of memory accesses, 'Load's, 'Store's and corresponding 'Hits' and 'Misses'.
- Each memory address has the following associated variables:
1. 'index': #index bits
2. 'tag': #tag bits
3. 'offset': #offset bits
4. 'addr_index': Index of the memory address
5. 'addr_tag': Tag of the memory address
6. 'addr_offset': Offset of the memory address
- Each cache cell has the following associated variables: 
1. 'index': Index of the cache block
2. 'tag': Tag of the cache block
3. 'valid': Valid bit for the cache block
4. 'nmru': The 'Not Most Recently Used' bit
- For every input address in the file, the aim is to find whether the information from that address exists in the cache, defined by whether the cache 'valid' bit is True at the any of the 'ways' (cache array columns) specified by the memory index, and if the 'tag' of both the memory address and the cache cell match. This is counted as a 'load hit' or a 'store hit'
- If the address does not exist, and the cache row specified by the memory 'index' is not 'full', the 'nmru' bit for that cell is set. This is counted as a 'load miss' or a 'store miss'.
- If the address does not exist, and the cache row for that memory 'index' is full, one of the cells must be 'evicted', and the new information from this memory address, stored in its place. One of the cells of this 'index' with the 'nmru' bit reset (i.e. it is not the most recently used cell) is evicted. This is counted as a 'load miss' or a 'store miss'.

## How to run:
- Compile:
> g++ cache_sim.cpp -std=c++11 -o cache_sim
- Run:
> ./cache_sim <INPUT_FILE> <CACHE_SIZE> <N_WAY_ASSOCIATIVITY>

## Sample input/ output
Input: 
0x00CDEF00 0
0xAACDEF00 0
0xBBCDEF00 0
0xCCCDEF00 0
0xBBCDEF00 0
0x00CDEF00 1
0xCCCDEF00 0
0xAACDEF00 0
0xABCDEF10 0
0xAACDEF00 0
0xADCDEF20 0
0xAECDEF00 0

Output:
Number of accesses:12
Number of loads:11
Number of stores:1
Number of hits:5
Number of load hits:4
Number of store hits:1
Number of miss:7
Number of load miss:7
Number of store miss:0
Number of evictions:3

## Co-author: 
Shreyas Joshi
