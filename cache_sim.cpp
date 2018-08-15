/*
Description: Cache simulator
*/
#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <iostream> 
#include <string> 
#include <fstream>
#include <vector>
#include <sstream>
#include <math.h>

using namespace std;

// grouping all the variables related to the input memory 
typedef struct cacheLine { 
	unsigned int tag; // No. of tag bits
	unsigned int index; // No. of index bits
	unsigned int offset; // No. of offset bits
	int addr_offset; // Integer representation of offset
	int addr_index; // Integer representation of index
	string addr_tag; // Tag is string (binary)
} struct1;

// Grouping all the variables related to the whole cache
typedef struct cache {
	bool valid; // Valid bit for each block
	string tag; // Tag of the cache block
	int addr_index; // Index of the cache block
	bool nmru; // Not Most Recently Used bit
} struct2;

string hex_to_bin(char hex[]); 
void access_extract(string mem_addr, struct1 &mem_access);
int bin_to_dec(string bin);
void set_nmru(vector <struct2> &cache_set, int curr_way, unsigned int nWayAssociativity);

int main(int argc, char* argv[]) {
	unsigned int cacheBlockSize, nCacheBlocks, nCacheSets, nWayAssociativity;
	char nCurrentAccess[15];
	unsigned int offset_bits, index_bits, tag_bits;
	struct1 mem_access;
	int hit = 0, store_hit = 0, load_hit = 0, miss = 0,  store_miss = 0, load_miss = 0;
	int count_ways = 0, count_evict = 0, evict = 0;
	int nLoad = 0, nStore = 0, nAccesses = 0;
	int ld_st = 0;	
	cacheBlockSize = 128;
	offset_bits = 7;

	// File access and looping through the file contents
	string traffic_file = argv[1]; 
	int cacheSize = stod(argv[2]);
	nWayAssociativity = stoi(argv[3]);
	ifstream ifs(traffic_file);
	string line, line1;
	vector<string> input_address;
	vector<int> input_ldst;	
	while (getline(ifs, line)) { // Looping through each line of the input text file
		stringstream ss(line);
		int tmp = 0;
		while (getline(ss, line1, ' ')) { // Extracting address and load/store bit from each line
			if (tmp % 2 == 0) {
				input_address.push_back(line1);
			}
			else if (tmp % 2 != 0) {
				input_ldst.push_back(stoi(line1));
				if (line1 == "0")
					++nLoad;
				else
					++nStore;
			}
			tmp += 1;
		}
	}
	nAccesses = input_address.size();

	cout << "Input: \n";
	for (int i = 0; i < input_ldst.size(); i++) {
		cout << input_address[i] << " " << input_ldst[i] << "\n";
	}

	// Calculating cache parameters
	nCacheBlocks = cacheSize / cacheBlockSize;
	nCacheSets = nCacheBlocks / nWayAssociativity;
	index_bits = log2(nCacheSets);
	tag_bits = 32 - index_bits - offset_bits; // Given: 32 bit memory address

	// Creating a cache 2D Vector (default settings)
	vector <vector <struct2>> cache(nCacheSets, vector<struct2>(nWayAssociativity)); 
	for (unsigned int i = 0; i < nCacheSets; i++) {
		for (unsigned int j = 0; j < nWayAssociativity; j++) {
			cache[i][j].valid = false;
			cache[i][j].addr_index = i;
			cache[i][j].tag = " ";
			cache[i][j].nmru = false;
		}
	}
	
	// Loop over file contents
	for (int index = 0; index < nAccesses; index++) {

		// Decoding the input memory address
		strcpy_s(nCurrentAccess, input_address[index].c_str());
		string mem_addr = hex_to_bin(nCurrentAccess); // Converting Hex address to a binary string
		ld_st = input_ldst[index];
		cout << "Memory address: " << mem_addr << " Load/Store: " << ld_st << "\n";

		mem_access.tag = tag_bits;
		mem_access.index = index_bits;
		mem_access.offset = offset_bits;
		access_extract(mem_addr, mem_access); // Separating index, offset, tag of the input address
		cout << " Tag " << mem_access.addr_tag << "\n";
		cout << " Index " << mem_access.addr_index << "\n";
		cout << " Offset " << mem_access.addr_offset <<"\n";

		// Tag and valid bit match to find the hit conditions
		for (unsigned int j = 0; j < nWayAssociativity; j++) {
			if ((cache[mem_access.addr_index][j].tag == mem_access.addr_tag) && (cache[mem_access.addr_index][j].valid == true)) { // Found in cache
				set_nmru(cache[mem_access.addr_index], j, nWayAssociativity);
				if (ld_st == 0) {
					++load_hit;
				}
				else {
					++store_hit;
				}
				++hit;
				count_ways = 0;
				break;
			}
			else { // Not found in cache
				++count_ways;			
			}
		}

		if (count_ways == nWayAssociativity) {
			for (int i = 0; i < nWayAssociativity; i++) {
				if (cache[mem_access.addr_index][i].valid == false) { // If not found in cache + that index is not full
					cache[mem_access.addr_index][i].tag = mem_access.addr_tag;
					cache[mem_access.addr_index][i].valid = true;
					set_nmru(cache[mem_access.addr_index], i, nWayAssociativity);
					count_evict = 0;
					count_ways = 0;
					if (ld_st == 0) {
						++load_miss;
					}
					else {
						++store_miss;
					}
					++miss;
					break;
				}
				else { 
					++count_evict;
				}
			}
		}
		
		if (count_evict == nWayAssociativity) { // Not found in the cache and cache is full
			for (int i = 0; i < nWayAssociativity; i++) {
				if (cache[mem_access.addr_index][i].nmru == false) { // Eviction based on NMRU 
					cache[mem_access.addr_index][i].tag = mem_access.addr_tag;
					cache[mem_access.addr_index][i].valid = true;
					set_nmru(cache[mem_access.addr_index], i, nWayAssociativity);
					++evict;
					if (ld_st == 0) {
						++load_miss;
					}
					else {
						++store_miss;
					}
					++miss;
					count_ways = 0;
					count_evict = 0;
					break;
				}
			}
		}
	}
	cout << "Number of accesses:" << nAccesses << "\n";
	cout << "Number of loads:" << nLoad << "\n";
	cout << "Number of stores:" << nStore << "\n";
	cout << "Number of hits:" << hit << "\n";
	cout << "Number of load hits:" << load_hit << "\n";
	cout << "Number of store hits:" << store_hit << "\n";
	cout << "Number of miss:" << miss << "\n";
	cout << "Number of load miss:" << load_miss << "\n";
	cout << "Number of store miss:" << store_miss << "\n";
	cout << "Number of evictions:" << evict << "\n";
	return 0;
}

void set_nmru(vector <struct2> &cache_set, int curr_way, unsigned int nWayAssociativity) {
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

int bin_to_dec(string bin) {
	/*	Description: Converts binary string to integer 
		Input: bin: Binary string to be converted to decimal
		Output: dec: Converted integer
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

void access_extract(string mem_addr, struct1 &mem_access) {
	/*	Description: Separates the index, offset and tag bits from the input memory address
		Input:	mem_addr: String binary address
				mem_access: struct containing separated index, offset and tag
	*/
	string tmp1 = mem_addr.substr(mem_addr.length() - mem_access.offset, mem_access.offset);
	mem_access.addr_offset = bin_to_dec(tmp1);

	string tmp2 = mem_addr.substr(mem_addr.length() - mem_access.offset- mem_access.index, mem_access.index);
	mem_access.addr_index = bin_to_dec(tmp2);

	mem_access.addr_tag = mem_addr.substr(0, mem_access.tag);
}


string hex_to_bin(char hex[]) {
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
