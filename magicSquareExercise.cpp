// #include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <set>
#include <sstream>
#include <array>
#include <map>
#include <algorithm>
#include <iterator>

using namespace std;

// defining default boardstates

//short start[4][4] = {{13, 15, 2, 0}, {8, 1, 14, 7}, {3, 10, 5, 12}, {6, 4, 9, 11}}; // temp magic square
short start[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {0, 13, 14, 15}};
// solution 1 is the "true" solution
// solution 2 is the "false" solution, switched 14 and 15 from true solution
short sol1[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 0}};
short sol2[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 15, 14, 0}};

// interpreter, import txt file, convert to list of boardstates

array<array<array<short, 4>, 4>, 7040> interpret_file(string filename) {
    // params: filename
    // return: array of magic squares
    ifstream inputFile(filename);
    array<array<array<short, 4>, 4>, 7040> magic_s{}; // generating empty array of boardstates

    if (!inputFile.is_open()) {
        cerr << "Failed to open the input file." << endl;
        return magic_s; // Return empty array as file opening failed
    }

    string line;
    int gridIndex = -1; // boardstate number #N
    int row = 0; // magic square row

    while (getline(inputFile, line)) { // check that there are still lines to read
        if (line[0] == '#') { // check for new squares
            gridIndex++;
            row = 0;
        }
        else {
            istringstream iss(line); // iterate through values to generate grid row
            for (int col = 0; col < 4; col++) {
                iss >> magic_s[gridIndex][row][col];
            }
            row++; // advance row 
        }
    }

    return magic_s; // return final array
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
    unsigned long long zmask = (unsigned long long) 0xFFFFFFFFFFFFFFFF - (0xF << (15-n)*4);
    grid &= zmask;
    unsigned long long amask = (unsigned long long) value << (15-n)*4;
    grid += amask;
    return grid;
}
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

void print_grid(short arr[4][4]) {
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            cout << arr[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

unsigned long long swap_hash_ns(char n1, char n2, unsigned long long grid) {
    unsigned long long hash = set_hash_n(n2, get_hash_n(n1, grid), grid);
    return set_hash_n(n1, get_hash_n(n2, grid), hash);
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

// Function that processes grid and returns a list of possible moves
vector<short> get_moves(short grid[4][4])
{
    vector<short> moves;
    int x, y;
    // find the empty space
    for (int i = 0; i < 16; i++)
    {
        if (grid[i / 4][i % 4] == 0)
        {
            x = i / 4;
            y = i % 4;
            break;
        }
    }
    // check if the empty space can move up, down, left, or right
    if (x > 0)
    {
        moves.push_back(0);
    }
    if (x < 3)
    {
        moves.push_back(1);
    }
    if (y > 0)
    {
        moves.push_back(2);
    }
    if (y < 3)
    {
        moves.push_back(3);
    }
    return moves;
}

void generateNewGrid(short newGrid[4][4], short move) {
    // params: newGrid, move
    // return: newGrid
    // generates a new grid based on the move

    int x, y;
    bool flag = false;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (newGrid[i][j] == 0) {
                x = i;
                y = j;
                flag = true;
                break;
            }
        }
    }
    if (!flag) throw 101;
    switch (move) {
        case 0: // move up
            newGrid[x][y] = newGrid[x-1][y];
            newGrid[x-1][y] = 0;
            break;
        case 1: // move down
            newGrid[x][y] = newGrid[x+1][y];
            newGrid[x+1][y] = 0;
            break;
        case 2: // move left
            newGrid[x][y] = newGrid[x][y-1];
            newGrid[x][y-1] = 0;
            break;
        case 3: // move right
            newGrid[x][y] = newGrid[x][y + 1];
            newGrid[x][y+1] = 0;
            break;
    }
}

// meet in the middle algorithm

// Distance Hashing
// Map to store the distance of each hashed grid from the start or end
map<unsigned long long, int> distanceMap;
// Vector to store the distance of each of the nodes visited by the BFS algorithm
vector<int> dist;
// Variable to keep track of the number of node
unsigned long long distMapSize = 2;

// Initial variables
queue<unsigned long long> que1;
queue<unsigned long long> que2;
int depth = 0;
const int MAX_DEPTH = 25;

// Function to perform the bidirectional BFS
bool bfs(unsigned long long start, unsigned long long end) {
    // params: start, end
    // return: bool
    que1.push(start);
    
    while (!que1.empty() || !que2.empty()) {
        int sz = que1.size();
        // Process all nodes at the current level
        depth++;
        if (depth > MAX_DEPTH) return false;
        for (int cnt = 1; cnt <= sz; cnt++) {
            // Get the next node from the queue
            unsigned long long cur = que1.front();que1.pop();
            // Get the grid from the hash
            short unhashedGrid[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
            unhash_grid(unhashedGrid, cur);
            // Get the possible moves from the current grid
            vector<short> moves = get_moves(unhashedGrid);
            
            // Process all the possible moves
            for (auto move : moves) {
                // Generate a new grid that incorporates the move
                //copy unhashedGrid
                short newGrid[4][4];
                unhash_grid(newGrid, cur);
                generateNewGrid(newGrid, move);
                
                unsigned long long hashedNewGrid = hash_grid(newGrid);

                // Check if moving into an already visited node
                if (distanceMap.find(hashedNewGrid) != distanceMap.end() && dist[distanceMap[hashedNewGrid]] < 0) continue;

                // check if the node has already been visited by the other bfs algorithm running from the end. This case results in a "meet in the middle"
                if (distanceMap.find(hashedNewGrid) != distanceMap.end() && dist[distanceMap[hashedNewGrid]] < 0) {
                    // The node has been visited by the other BFS algorithm
                    // Calculate the total distance from the start to the end
                    int totalDistance = dist[distanceMap[cur]] -  dist[distanceMap[hashedNewGrid]] - 1;
                    dist[distanceMap[hashedNewGrid]] = totalDistance;
                    // solution has been found. The hash of the grid is being returned
                    return true;                
                }
                
                // update distance
                distanceMap[hashedNewGrid] = distMapSize;
                distMapSize++;
                dist.push_back(dist[distanceMap[cur]]+1);
                // push into the queue
                que1.push(hashedNewGrid);
            }
        }

        que2.push(end);

        sz = que2.size();
         for (int cnt = 1; cnt <= sz; cnt++) {
            // Get the next node from the queue
            unsigned long long cur = que2.front();que2.pop();
            // Get the grid from the hash
            short unhashedGrid[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
            unhash_grid(unhashedGrid, cur);
            // Get the possible moves from the current grid
            vector<short> moves = get_moves(unhashedGrid);
            
            // Process all the possible moves
            for (auto move : moves) {
                // Generate a new grid that incorporates the move
                //copy unhashedGrid
                short newGrid[4][4];
                unhash_grid(newGrid, cur);
                generateNewGrid(newGrid, move);
                
                unsigned long long hashedNewGrid = hash_grid(newGrid);

                // Check if moving into an already visited node
                if (distanceMap.find(hashedNewGrid) != distanceMap.end() && dist[distanceMap[hashedNewGrid]] < 0) continue;

                // check if the node has already been visited by the other bfs algorithm running from the end. This case results in a "meet in the middle"
                if (distanceMap.find(hashedNewGrid) != distanceMap.end() && dist[distanceMap[hashedNewGrid]] > 0) {
                    // The node has been visited by the other BFS algorithm
                    // Calculate the total distance from the start to the end
                    int totalDistance = -dist[distanceMap[cur]] +  dist[distanceMap[hashedNewGrid]] - 1;
                    dist[distanceMap[hashedNewGrid]] = totalDistance;
                    // solution has been found. The hash of the grid is being returned
                    return true;                
                }
                
                // update distance
                distanceMap[hashedNewGrid] = distMapSize;
                distMapSize++;
                dist.push_back(dist[distanceMap[cur]]-1);
                // push into the queue
                que2.push(hashedNewGrid);
            }
        }
    }
    return false;
}

int main() {;
    // preprocessing
    array<array<array<short, 4>, 4>, 7040> magic_s = interpret_file("magicSquares.txt");
    
    
    // for each magic square
    for (array<array<short, 4>, 4> magicSqTmp : magic_s) {
        // conversion to short[4][4]
        short magicSquare[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
        for (int i=0; i<4; i++) {
            for (int j=0; j<4; j++) {
                magicSquare[i][j] = magicSqTmp[i][j];
            }
        }
        // hash array
        unsigned long long hash = hash_grid(magicSquare);
        // find solvability
        bool solvableSol1 = check_solvable(hash);
        
        // bfs with sol1 if solvable, sol2 if not
        if (solvableSol1) {
            distanceMap[hash_grid(magicSquare)] = 0;
            dist.push_back(1);
            distanceMap[hash_grid(sol1)] = 1;
            dist.push_back(-1);
            cout << (bfs(hash_grid(magicSquare), hash_grid(sol1))? "Solved" : "Not a Solution");
            print_grid(magicSquare);
            distanceMap.clear();
            dist.clear();
        } else {
            distanceMap[hash_grid(magicSquare)] = 0;
            dist.push_back(1);
            distanceMap[hash_grid(sol2)] = 1;
            dist.push_back(-1);
            cout << (bfs(hash_grid(magicSquare), hash_grid(sol2))? "Solved" : "Not a Solution");
            print_grid(magicSquare);
            distanceMap.clear();
            dist.clear();
        }
    }
    return 0;
}
