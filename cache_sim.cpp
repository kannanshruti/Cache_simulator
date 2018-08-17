/*

Name: Cache memory simulator

Description: Code which simulates the working of a cache memory. The command
line arguments for the cache include the cache size and its associativity.
The code also accepts an input file, with each line containing a memory 
address and a boolean value representing 'Load' or 'Store' operation at 
that address. The output of the program is a count of the number of memory
accesses, 'Load's, 'Store's and corresponding 'Hits' and 'Misses'.

Authors: Shruti Kannan, Shreyas Joshi
*/

#include <stdio.h>
#include <string>
#include <iostream> 
#include <string.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <math.h>

using namespace std;

// Grouping all the variables related to the input memory 
typedef struct memory_addr { 
	unsigned int tag; // No. of tag bits
	unsigned int index; // No. of index bits
	unsigned int offset; // No. of offset bits
	int addr_offset; // Integer representation of offset
	int addr_index; // Integer representation of index
	string addr_tag; // Tag is string (binary)
	int ld_st;
} struct1;

// Grouping all the variables related to the whole cache
typedef struct cache {
	bool valid; // Valid bit for each block
	string tag; // Tag of the cache block
	int addr_index; // Index of the cache block
	bool nmru; // Not Most Recently Used bit
} struct2;


class cache_sim {
	/*
		Description: Implements the methods required to simulate cache memory
	*/
public:
	vector<string> input_address;
	vector<int> input_ldst;	
	struct1 mem_access;
	int nLoad = 0, nStore = 0, nAccesses = 0;
	int hit = 0, store_hit = 0, load_hit = 0;
	int miss = 0,  store_miss = 0, load_miss = 0;
	int count_ways = 0, count_evict = 0, nEvict = 0;
	unsigned int cacheBlockSize;

	cache_sim(string traffic_file, int cacheSize, unsigned int nWayAsso);
	void get_file_line(string input_address, int input_ldst);
	void check_in_cache();
	void check_cache_full();
	void check_cache_eviction();
	bool not_found_in_cache();
	bool is_cache_eviction_necessary();
private:
	unsigned int offset_bits, index_bits, tag_bits;
	unsigned int nWayAssociativity, nCacheBlocks, nCacheSets;
	vector <vector <struct2>> cache;

	void get_file_contents(string traffic_file);
	void access_extract(string mem_addr, struct1 &mem_access);
	void create_cache(vector <vector <struct2>> &cache, unsigned intnCacheSets,
					  unsigned int nWayAssociativity);
	string hex_to_bin(char hex[]);
	int bin_to_dec(string bin);
	void set_nmru(vector <struct2> &cache_set, int curr_way, unsigned int nWayAssociativity);
};

cache_sim::cache_sim(string traffic_file, int cacheSize,
						  unsigned int nWayAsso) {
	cacheBlockSize = 128;
	offset_bits = 7;
	nWayAssociativity = nWayAsso;

	// Reading the input file
	get_file_contents(traffic_file);

	// Calculating cache parameters
	nCacheBlocks = cacheSize / cacheBlockSize;
	nCacheSets = nCacheBlocks / nWayAssociativity;
	index_bits = log2(nCacheSets);
	tag_bits = 32 - index_bits - offset_bits;

	// Initializing the cache
	cache.resize(nCacheSets, vector<struct2>(nWayAssociativity));
	create_cache(cache, nCacheSets, nWayAssociativity);

	// Set the number of bits
	mem_access.tag = tag_bits;
	mem_access.index = index_bits;
	mem_access.offset = offset_bits;
}


void cache_sim::get_file_contents(string traffic_file) {
	/*	Description: Reads the contents of "traffic_file". Stores the
		memory addresses in "input_address", and the load/store status
		in "input_ldst". The number of required accesses is stored 
		as "nAccesses"
	*/
	ifstream ifs(traffic_file);
	string line, line1;
	// Looping through each line of the input text file
	while (getline(ifs, line)) {
		stringstream ss(line);
		int tmp = 0;
		// Extracting address and load/store bit from each line
		while (getline(ss, line1, ' ')) {
			if (tmp % 2 == 0) {
				input_address.push_back(line1);
			}
			else if (tmp % 2 != 0) {
				input_ldst.push_back(stoi(line1));
				if (line1 == "0") ++nLoad;
				else ++nStore;
			}
			tmp += 1;
		}
	}
	nAccesses = input_address.size();
	return;
}

void cache_sim::create_cache(vector <vector <struct2>> &cache, unsigned int nCacheSets,
				  unsigned int nWayAssociativity) {
	/*	Description: Initializes an empty cache based on the 
		number of sets and the associativity
	*/
	for (unsigned int i = 0; i < nCacheSets; i++) { // add to init
		for (unsigned int j = 0; j < nWayAssociativity; j++) {
			cache[i][j].valid = false;
			cache[i][j].addr_index = i;
			cache[i][j].tag = " ";
			cache[i][j].nmru = false;
		}
	}
	return;
}

void cache_sim::get_file_line(string input_address,int input_ldst) {
	/*	Description: Stores the memory address and load/store status
		to the "mem_access" struct
	*/
	char nCurrentAccess[15];
	strcpy(nCurrentAccess, input_address.c_str());
	string mem_addr = hex_to_bin(nCurrentAccess); // Converting Hex address to a binary string
	mem_access.ld_st = input_ldst;
	access_extract(mem_addr, mem_access);
	return;
}

bool cache_sim::not_found_in_cache() {
	/*	Description: Returns 'True' if the memory address has been
		searched across all the cache ways for the particular index,
		and not found. 
	*/
	return count_ways == nWayAssociativity;
}

void cache_sim::check_cache_full() {
	/*	Description: If the address index is not found in any of the cache ways,
		checks if the cache is full , if not, the information from the new 
		address is to be inserted into the cache. 
	*/
	for (int i = 0; i < nWayAssociativity; i++) {
		if (cache[mem_access.addr_index][i].valid == false) { 
			cache[mem_access.addr_index][i].tag = mem_access.addr_tag;
			cache[mem_access.addr_index][i].valid = true;
			set_nmru(cache[mem_access.addr_index], i, nWayAssociativity);
			count_evict = 0;
			count_ways = 0;
			if (mem_access.ld_st == 0) ++load_miss;
			else ++store_miss;
			++miss;
			break;
		}
		else ++count_evict;
	}
	return;
}

void cache_sim::check_in_cache() {
	/*	Description: Checks if the memory address index is found in the cache.
		If yes, a corresponding 'hit' is incremented.
	*/
	for (unsigned int j = 0; j < nWayAssociativity; j++) {
		if ((cache[mem_access.addr_index][j].tag == mem_access.addr_tag) && (cache[mem_access.addr_index][j].valid == true)) { // Found in cache
			set_nmru(cache[mem_access.addr_index], j, nWayAssociativity);
			if (mem_access.ld_st == 0) ++load_hit;
			else ++store_hit;
			++hit;
			count_ways = 0;
			break;
		}
		else ++count_ways; // Not found in cache
	}
	return;
}

bool cache_sim::is_cache_eviction_necessary() {
	/*	Description: Checks if an eviction from cache is required.
		Returns 'True' if eviction is necessary
	*/
	return count_evict == nWayAssociativity;
}

void cache_sim::check_cache_eviction() {
	/*	Description: Determines which cache index is to be evicted.
		Loops over the cache ways of the index in question, and if the "nmru"
		bit is reset, evicts that particular value.
	*/
	for (int i = 0; i < nWayAssociativity; i++) {
		if (cache[mem_access.addr_index][i].nmru == false) {
			cache[mem_access.addr_index][i].tag = mem_access.addr_tag;
			cache[mem_access.addr_index][i].valid = true;
			set_nmru(cache[mem_access.addr_index], i, nWayAssociativity);
			++nEvict;
			if (mem_access.ld_st == 0) ++load_miss;
			else ++store_miss;
			++miss;
			count_ways = 0;
			count_evict = 0;
			break;
		}
	}
	return;
}

void cache_sim::set_nmru(vector <struct2> &cache_set, int curr_way, unsigned int nWayAssociativity) {
	/*	Description: Setting NMRU bit of the most recently used index to 1 and the others to 0
		Input:	cache_set: Cache set which is to be 
				curr_way:
				nWayAssociativity:
		Output: cache_set NMRU bits are updated
	*/
	for (int i = 0; i < nWayAssociativity; i++) {
		if (i == curr_way) {
			cache_set[i].nmru = true;
		}
		else {
			cache_set[i].nmru = false;
		}
	}
}

string cache_sim::hex_to_bin(char hex[]) {
	/*	Description: Convert hex character array to binary string
		Input: hex: Character array containing the hex string
		Output: bin: Converted binary string
	*/
	string bin = "";
	int start = 0;
	if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
		start = 2;
	}
	for (unsigned i = start; i < strlen(hex); ++i) {
		switch (toupper(hex[i])) {
		case '0': bin += "0000"; break;
		case '1': bin += "0001"; break;
		case '2': bin += "0010"; break;
		case '3': bin += "0011"; break;
		case '4': bin += "0100"; break;
		case '5': bin += "0101"; break;
		case '6': bin += "0110"; break;
		case '7': bin += "0111"; break;
		case '8': bin += "1000"; break;
		case '9': bin += "1001"; break;
		case 'A': bin += "1010"; break;
		case 'B': bin += "1011"; break;
		case 'C': bin += "1100"; break;
		case 'D': bin += "1101"; break;
		case 'E': bin += "1110"; break;
		case 'F': bin += "1111"; break;
		}
	}
	return bin;
}

void cache_sim::access_extract(string mem_addr, struct1 &mem_access) {
	/*	Description: Separates the index, offset and tag bits from the input memory address
		Input:	mem_addr: String binary address
				mem_access: struct containing separated index, offset and tag
	*/
	string tmp1 = mem_addr.substr(mem_addr.length() - mem_access.offset, mem_access.offset);
	mem_access.addr_offset = bin_to_dec(tmp1);

	string tmp2 = mem_addr.substr(mem_addr.length() - mem_access.offset- mem_access.index, mem_access.index);
	mem_access.addr_index = bin_to_dec(tmp2);

	mem_access.addr_tag = mem_addr.substr(0, mem_access.tag);

	return;
}

int cache_sim::bin_to_dec(string bin) {
	/*	Description: Converts binary string to integer 
		Input: "bin": Binary string to be converted to decimal
		Output: "dec": Converted integer
	*/
	int bin1 = stoi(bin);
	int rem, counter=0, dec=0;
	while (bin1 != 0) {
		rem = bin1 % 10;
		dec += pow(2, counter)*rem;
		bin1 = bin1 / 10;
		counter++;
	}
	return dec;
}


int main(int argc, char* argv[]) {
	string traffic_file = argv[1]; 
	int cacheSize = stod(argv[2]);
	unsigned int nWayAssociativity = stoi(argv[3]);
	
	cache_sim c1(traffic_file, cacheSize, nWayAssociativity);

	cout << "Input: \n";
	for (int i = 0; i < c1.input_ldst.size(); i++) {
		cout << c1.input_address[i] << " " << c1.input_ldst[i] << "\n";
	}

	// Loop over file contents
	for (int index = 0; index < c1.nAccesses; index++) {
		// Decoding the input memory address and
		// Separating index, offset, tag of the input address
		c1.get_file_line(c1.input_address[index], c1.input_ldst[index]);
		cout << " Tag " << c1.mem_access.addr_tag << "\n";
		cout << " Index " << c1.mem_access.addr_index << "\n";
		cout << " Offset " << c1.mem_access.addr_offset <<"\n";
		cout << " LDST " << c1.mem_access.ld_st <<"\n";

		// Check if present in cache
		c1.check_in_cache();

		// If not found in cache and cache not full, insert to cache
		if (c1.not_found_in_cache())	{	
			c1.check_cache_full();
		}
		// If not found in cache and cache full, evict one entry from cache
		if (c1.is_cache_eviction_necessary()) { 
			c1.check_cache_eviction();
		}
	}
	cout << "Number of accesses:" << c1.nAccesses << "\n";
	cout << "Number of loads:" << c1.nLoad << "\n";
	cout << "Number of stores:" << c1.nStore << "\n";
	cout << "Number of hits:" << c1.hit << "\n";
	cout << "Number of load hits:" << c1.load_hit << "\n";
	cout << "Number of store hits:" << c1.store_hit << "\n";
	cout << "Number of miss:" << c1.miss << "\n";
	cout << "Number of load miss:" << c1.load_miss << "\n";
	cout << "Number of store miss:" << c1.store_miss << "\n";
	cout << "Number of evictions:" << c1.nEvict << "\n";
	return 0;
}