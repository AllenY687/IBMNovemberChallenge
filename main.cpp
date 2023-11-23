#include <iostream>
#include <unordered_map>
#include <queue>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace std;


// ------------------- GRID UTILS -------------------- //


// print a long long in readable hex
void print_hex(unsigned long long value) {
	char m[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	char h[16];
	for (int i = 0; i < 16; i++) {
		h[i] = m[value & (unsigned long long) 0xF];
		value = value >> 4;
	}
	cout << "0x";
	for (int i = 15; i >= 0; i--) {
		cout << h[i];
	}
}
// encode grid into hashed values
unsigned long long hash_grid(short grid[4][4]) {
	// params: grid to be hashed
	// return: hashed value (64 bit long)
	unsigned long long hash = 0;
	for (char y = 0; y < 4; y++) {
		for (char x = 0; x < 4; x++) {
			int offset = ((3-y) * 4 + (3-x)) * 4;
			hash += (unsigned long long) grid[y][x] << offset;
		}
	}
	return hash;
}
// decode hashed values back into grid
void unhash_grid(short grid[4][4], unsigned long long hash) {
	// params: return grid, hashed grid
	// unhashes grid and stores into return grid
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			int offset = ((3-y) * 4 + (3-x)) * 4;
			unsigned long long mask = (unsigned long long) 0xF << offset;
			grid[y][x] = (hash & mask) >> offset;
		}
	}
}
// get number from the hash by index
short get_hash_n(char n, unsigned long long grid) {
	// params: index, hash
	// return: value at index
	unsigned long long mask = (unsigned long long) 0xF << (15-n)*4;
	return (grid & mask) >> (15-n)*4;
} 
// set number from the hash by index
unsigned long long set_hash_n(char n, short value, unsigned long long grid) {
	// params: index, value, hash
	// return: replaces value of hash at index n with value
	unsigned long long zmask = (unsigned long long) 0xFFFFFFFFFFFFFFFF - (((unsigned long long) (0xF)) << ((15-n)*4));
	grid &= zmask;
	unsigned long long amask = (unsigned long long) value << (15-n)*4;
	grid |= amask;
	return grid;
}
// swap values in a hash by index
unsigned long long swap_hash_ns(char n1, char n2, unsigned long long grid) {
	// params: index 1, index 2, hash
	// return: hash with swapped values
	unsigned long long copy = grid;
	grid = set_hash_n(n2, get_hash_n(n1, grid), grid);
	return set_hash_n(n1, get_hash_n(n2, copy), grid);
}



// --------------------- GRID SERIALIZATION UTILS ---------------------- //



// append a serialized grid into a file binary
void serialize(ofstream* file, unsigned long long grid) {
	// params: point to output file stream to be serialized, grid to be serialized
	
	if (!file->is_open()) {
		cerr << "Failed to open output file";
		return;
	}

	for (int i = 0; i < 8; i++) {
		unsigned long long mask = (unsigned long long) 0xFF << (7-i)*8;
		char byte = (mask & grid) >> (7-i)*8;
	
		file->write(&byte, sizeof(byte));
	}
}
// get size of a file in bytes
long sizeof_deserialize(string filename) {
	// params: file path
	// return: size of the file
	filesystem::path p{filename};
	return filesystem::file_size(p);
}
// deserialize a serialized map
void deserialize_map(ifstream* file, unordered_map<unsigned long long, char>* map, long size) {
	// params: pointer to input file stream where map is, pointer to return map, size in bytes of file
	// return: file contents deserialized into map
	if (!file->is_open()) {
		cerr << "Failed to open input file";
		return;
	}

	int count = 0;
	for (long i = 0; i < size / 8; i++) {
		unsigned long long long_val;
		char depth;
		file->read((char*)&long_val, 8);
		// print_hex(long_val);
		(*map)[(unsigned long long) long_val] = 0xFF;
		if (i * 800 / size > count) {
			count++;
			cout << "\r";
			fflush(stdout);
			cout << count << "%";
			fflush(stdout);
			if (count == 99) cout << endl << "Deserialized" << endl;
		}
	}
}
// deserialize a serialized list
void deserialize_list(ifstream* file, unsigned long long* solns, int size) {
	// params: pointer to input file stream where list is, return solution array, size in bytes of file
	// return: file contents deserialized into an array
	
	if (!file->is_open()) {
		cerr << "Failed to open input file";
		return;
	}

	for (int i = 0; i < size / 8; i++) {
		unsigned long long long_val;
		file->read((char*)&long_val, 8);
		solns[i] = long_val;
	}
}



// ------------------------- SEARCH FUNCTIONS ------------------------- //



// get possible moves for a given grid
void get_moves(unsigned long long results[4], unsigned long long hash) {
	// params: return array of 4 possible moves, grid
	// return: array of values, 0x0 of invalid move, corresponding move if valid move
	
	// find 0 index
	char nz;
	for (int i = 0; i < 16; i++) {
		if (get_hash_n(i, hash) == 0) {
			nz = i;
		}
	}
	// get valid moves
	char x = nz & 0x3, y = nz >> 2;
	results[0] = (x != 0) ? swap_hash_ns(nz, nz-1, hash) : 0x0;
	results[1] = (x != 3) ? swap_hash_ns(nz, nz+1, hash) : 0x0;
	results[2] = (y != 0) ? swap_hash_ns(nz, nz-4, hash) : 0x0;
	results[3] = (y != 3) ? swap_hash_ns(nz, nz+4, hash) : 0x0;
}
// breadth first search meet in the middle implementation with start and end
bool bfs(unsigned long long start, unsigned long long end) {
	// params: start and end grid
	// returns: if a path exists

	// fowards and backwards queue
	queue<unsigned long long> fqc, bqc;
	// forwards and backwards maps
	unordered_map<unsigned long long, char> fmp, bmp;

	// depth markers
	int current_depth = 0;
	
	// push the beginning and end nodes to queues
	fqc.push(start);
	fmp[start] = 1;
	bqc.push(end);
	bmp[end] = 1;

	while (current_depth < 25) {
		// get current forward and backwards grids
		unsigned long long current_grid_f = fqc.front(); fqc.pop();
		unsigned long long current_grid_b = bqc.front(); bqc.pop();
		
		// get next moves
		unsigned long long forward_next_moves[4];
		unsigned long long backward_next_moves[4];
		get_moves(forward_next_moves, current_grid_f);
		get_moves(backward_next_moves, current_grid_b);

		for (int i = 0; i < 4; i++) {
			// for each next moves
			unsigned long long f_move = forward_next_moves[i];
			unsigned long long b_move = backward_next_moves[i];
			
			if (!(f_move == 0x0 || fmp[f_move] > 0)) {
				// solution found
				if (bmp[f_move]) return true;
				// solution not found
				else {
					fqc.push(f_move);
					fmp[f_move] = fmp[current_grid_f] + 1;
					// update depth
					if (fmp[f_move] > current_depth) {
					       current_depth++;
					       print_hex(f_move);
					       cout << endl;
					}
				}
			}
			if (!(b_move == 0x0 || bmp[b_move] > 0)) {
				// solution found
				if (fmp[b_move]) return true;
				// solution not found
				else {
					bqc.push(b_move);
					bmp[b_move] = bmp[current_grid_b] + 1;
				}
			}
		}
	}
	return false;
}
// breadth first search meet in the middle with precalculated forward propogation map
long long bfs_precalc(unordered_map<unsigned long long, char> fmp, unsigned long long end) {
	// params: forward propogation map, ending grid
	// returns: 0x0 if no solution found, else middle meet point
	
	cout << "Searching ";
	print_hex(end);
	cout << endl;

	queue<unsigned long long> q;
	unordered_map<unsigned long long, char> bmp;

	int current_depth = 0;
	q.push(end);
	bmp[end] = 1;

	if (fmp[end]) return true;

	while (current_depth < 25) {
		unsigned long long grid = q.front(); q.pop();

		unsigned long long moves[4];
		get_moves(moves, grid);

		for (int i = 0; i < 4; i++) {
			unsigned long long move = moves[i];

			if (!(move == 0x0 || bmp[move] > 0)) {
				if (fmp[move]) {
					cout << endl;
					return move;
				}
				else {
					q.push(move);
					bmp[move] = bmp[grid] + 1;
					if (bmp[move] > current_depth) {
						current_depth++;
						cout << "\r";
						fflush(stdout);
						cout << "Depth " << current_depth << ": ";
						print_hex(move);
						fflush(stdout);
					}
				}
			}
		}
	}
	cout << endl;
	return 0x0;
}
// precalculation for forward propogation map
void precalc_fmp(unsigned long long start, int depth, unordered_map<unsigned long long, char> fmp, string filename) {
	// params: starting grid, calculation depth, propogation map, serialization file
	
	queue<unsigned long long> q;
	int current_depth = 0;
	
	q.push(start);
	fmp[start] = 1;

	cout << "Searching ";
	print_hex(start);
	cout << endl;

	ofstream f_fmp(filename, ios::app | ios::binary);

	while (current_depth < depth) {

		unsigned long long grid = q.front(); q.pop();

		unsigned long long moves[4];
		get_moves(moves, grid);
		
		for (int i = 0; i < 4; i++) {
			unsigned long long move = moves[i];

			if (!(move == 0x0 || fmp[move] > 0)) {
				q.push(move);
				fmp[move] = fmp[grid] + 1;
				serialize(&f_fmp, move);
				if (fmp[move] > current_depth) {
					current_depth++;
					cout << "Depth " << current_depth << ": ";
					print_hex(move);
					cout << endl;

					f_fmp.close();
					f_fmp.open(filename, ios::app | ios::binary);
				}
			}
		}
	}
	f_fmp.close();
}



// ------------------------ LOGIC ENTRY ------------------------ //



int main() {

	// unordered_map<unsigned long long, char> fmp1;
	unordered_map<unsigned long long, char> fmp2;

	// --- PRECALCULATE FORWARD PROPOGATION MAPS --- //

	// cout << "Precalculating forward map 1" << endl;
	// precalc_fmp(0x123456789ABCDEF0, 25, fmp1, "fmpl.map");
	// cout << "Precalculating forward map 2" << endl;
	// precalc_fmp(0x123456789ABCDFE0, 25, fmp2, "fmpr.map");

	// --- PRECALCULATE SOL1 AND SOL2 --- //

	// unsigned long long results[7040];
	// parse_file("magicSquares.txt", results);

	// ofstream soln1("soln1.list", ios::app | ios::binary);
	// ofstream soln2("soln2.list", ios::app | ios::binary);

	// for (int i = 0; i < 7040; i++) {
	// 	if (check_solvable(results[i])) {
	// 		serialize(&soln1, results[i]);
	// 	}
	// 	else {
	// 		serialize(&soln2, results[i]);
	// 	}
	// }

	// --- DESERIALIZE SOL1 AND SOL2 --- //
	
	// ifstream f_sol1("soln1.list");
	// long sol1size = sizeof_deserialize("soln1.list");
	// unsigned long long sol1s[sol1size];
	// deserialize_list(&f_sol1, sol1s, sol1size);

	ifstream f_sol2("soln2.list");
	long sol2size = sizeof_deserialize("soln2.list");
	unsigned long long sol2s[sol2size];
	deserialize_list(&f_sol2, sol2s, sol2size);

	// --- DESERIALIZE FORWARD PROP MAPS --- //

	// cout << "Deserializing forward map 1" << endl;

	// ifstream f_fmp1("fmpl.map");
	// long size1 = sizeof_deserialize("fmpl.map");
	// deserialize_map(&f_fmp1, &fmp1, size1);
	// f_fmp1.close();

	cout << "Deserializing forward map 2" << endl;

	ifstream f_fmp2("fmpr.map");
	long size2 = sizeof_deserialize("fmpr.map");
	deserialize_map(&f_fmp2, &fmp2, size2);
	f_fmp2.close();

	// --- SEARCH FOR SOLUTIONS --- //

	cout << "Searching solution space" << endl;
	// for (int i = 0; i < sol1size / 8; i++) {
	// 	long long answer = bfs_precalc(fmp1, sol1s[i]);
	// 	cout << (answer ? "Solved: " : "Unsolved: ");
	// 	print_hex(answer);
	// 	cout << endl;
	// }
	for (int i = 0; i < sol2size / 8; i++) {
		long long answer = bfs_precalc(fmp2, sol2s[i]);
		cout << (answer ? "Solved: " : "Unsolved: ");
		print_hex(answer);
		cout << endl;
		if (answer) return 0;
	}

	return 0;
}






// ----------------- LEGACY CODE ----------------- //


// LEGACY: parse file containing magic squares
void parse_file(string filename, unsigned long long results[7040]) {
	// params: filename, return array
	// return: array of hashed grids
	ifstream file(filename);

	if (!file.is_open()) {
		cerr << "Failed to open input file." << endl;
		return;
	}

	string line;
	int gridIndex = -1; // boardstate number #N
	int row = 0; // magic square row
	
	short temp[4][4];

	while (getline(file, line)) { // check that there are still lines to read
		if (line[0] == '#') { // check for new squares
			results[gridIndex] = hash_grid(temp);
			gridIndex++;
			row = 0;
		}
		else {
			istringstream iss(line); // iterate through values to generate grid row
			for (int col = 0; col < 4; col++) {
				iss >> temp[row][col];
			}
			row++; // advance row 
		}
	}

}

// LEGACY
// Function that outputs true or false depending on solvability of square
// True => use sol1 as solved state
// False => use sol2 as solved state
// source: https://www.geeksforgeeks.org/check-instance-15-puzzle-solvable/
bool check_solvable(unsigned long long hash) {    
	// define variables that will affect whether the square is solvable
	short row = 3;
	short inversions;
	for (short i = 0; i < 15; i++) {
		if (get_hash_n(i, hash) == 0) { // search for zero
			row = i/4; // find row of zero
		} else {
			for (short j = 0; j < 15-i; j++) { // iterate through all pairs of tiles
				if (get_hash_n(i+j, hash) != 0 && get_hash_n(i, hash) > get_hash_n(i+j, hash)) {
					inversions++; // inversion detection
				}
			}
		}
	}
	return row%2 != inversions%2; // congruence means unsolvability 
}


