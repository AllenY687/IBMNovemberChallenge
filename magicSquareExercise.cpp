// #include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <set>
#include <sstream>
#include <array>

using namespace std;


int start[4][4] = {{13, 15, 2, 0}, {8, 1, 14, 7}, {3, 10, 5, 12}, {6, 4, 9, 11}}; // temp magic square

// solution 1 is the "true" solution
// solution 2 is the "false" solution, switched 14 and 15 from true solution
int sol1[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 1, 12}, {13, 14, 15, 0}};
int sol2[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 1, 12}, {13, 15, 14, 0}};

// need interpreter, import txt file, convert to list of boardstates

// short magic_s[7040][4][4];

array<array<array<short, 4>, 4>, 7040> interpret_file(string filename)
{
    ifstream inputFile(filename);
    array<array<array<short, 4>, 4>, 7040> magic_s{};

    if (!inputFile.is_open())
    {
        cerr << "Failed to open the input file." << endl;
        return magic_s; // Return empty array as file opening failed
    }

    string line;
    int gridIndex = -1;
    int row = 0;

    while (getline(inputFile, line))
    {
        if (line[0] == '#')
        {
            gridIndex++;
            row = 0;
        }
        else
        {
            istringstream iss(line);
            for (int col = 0; col < 4; col++)
            {
                iss >> magic_s[gridIndex][row][col];
            }
            row++;
        }
    }

    return magic_s;
}

// decoding and encoding grids
long long hash_grid(short (grid)[4][4]) {
    long long hash = 0;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            hash += *grid[x][y] << (y * 4 + x);
        }
    }
    return hash;
}
// {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 0}} becomes 0b0001001000110100010101100111100010011010101111001101111011110000 = 1311768467463790320 which is 123456789abcde0 in hex

void unhash_grid(short (grid)[4][4], long long hash)
{
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            long long mask = 0b1111 << (y * 4 + x);
            (*grid)[x][y] = (hash & mask) >> (y * 4 + x);
        }
    }
}

// Function that outputs true or false depending on solvability of square
// source: https://www.geeksforgeeks.org/check-instance-15-puzzle-solvable/
bool check_solvable(long long hash) {
    bool solvable;
    
    short row;
    short inversions;

    for (short i = 0; i < 16; i++) {
        if (i != 15) {
            if (hash[i] > hash[i+1]) {
                inversions++;
            }
        }
        
        if (hash[i] == 0) {
            row = i/4;
        }
    }
    +
    return row%2 != inversions%2;
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

// meet in the middle algorithm
set<long long> visited;
queue<long long> que1;
queue<long long> que2;
void bfs(long long start, long long end) {
    que1.push(start);
while (!que1.empty() || !que2.empty()) {
        int sz = que1.size();
        for (int cnt = 1; cnt <= sz; cnt++) {
            long long cur = que1.front();que1.pop();
            short unhashedGrid[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
            unhash_grid(unhashedGrid, cur);
            vector<short> moves = get_moves(unhashedGrid);
            for (auto move : moves) {
                // generate a new grid that incorporates the move
                short newGrid[4][4]; // generate (todo)
                long long hashedNewGrid = hash_grid(newGrid);

                // check if moving into an already visited node

                // check if the node has already been visited by the other bfs algorithm running from the end. This case results in a "meet in the middle"
                
                // update distance

                // push into the queue
                que1.push(hashedNewGrid);
            }
        }

        int sz = que2.size();
        
    }
}

vector<int> path;

int main() {;
    array<array<array<short, 4>, 4>, 7040> magic_s = interpret_file("magicSquares.txt");
    return 0;
}