#include <iostream>
#include <unordered_map>
#include <queue>
#include <fstream>
#include <sstream>

using namespace std;

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
	cout << endl;
}
// decoding and encoding grids
// encoding format: sol1 -> 0x123456789ABCDEF0, sol2 -> 0x123456789ABCDFE0
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

void interpret_file(string filename, unsigned long long results[7040]) {
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

void get_moves(unsigned long long results[4], unsigned long long hash) {
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

bool bfs(unsigned long long start, unsigned long long end) {
	// fowards and backwards queue
	queue<unsigned long long> fqc, bqc;
	// forwards and backwards maps
	unordered_map<unsigned long long, char> fmp, bmp;

	// depth markers
	int current_depth = 0;
	unsigned long long current_depth_marker = start;
	
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

int main() {

	unsigned long long results[7040];
	interpret_file("magicSquares.txt", results);
	print_hex(results[0]);

	//if (bfs(0x123456789ABCDEF0, 0x01EFBD24C6937A58)) {
	//	cout << "Solved" << endl;
	//}
	//else {
	//	cout << "Unsolved" << endl;
	//}
	// print_hex(set_hash_n(8, 0, 0x123456789ABC0DEF));

	return 0;
}


