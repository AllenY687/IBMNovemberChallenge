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
long long hash_grid(int (*grid)[4][4]) {
    long long hash = 0;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            hash += *grid[x][y] << (y * 4 + x);
        }
    }
    return hash;
}

void unhash_grid(int (*grid)[4][4], long long hash)
{
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            long long mask = 0b1111 << (y * 4 + x);
            (*grid)[x][y] = (hash & mask) >> (y * 4 + x);
        }
    }
}

// Function that processes grid and returns a list of possible moves
vector<int> get_moves(int (*grid)[4][4])
{
    vector<int> moves;
    int x, y;
    // find the empty space
    for (int i = 0; i < 16; i++) {
        if (grid[i / 4][i % 4] == 0) {
            x = i / 4;
            y = i % 4;
            break;
        }
    }
    // check if the empty space can move up, down, left, or right
    if (x > 0) {
        moves.push_back(0);
    }
    if (x < 3) {
        moves.push_back(1);
    }
    if (y > 0) {
        moves.push_back(2);
    }
    if (y < 3) {
        moves.push_back(3);
    }
    return moves;
}

// meet in the middle algorithm
set<long long> visited;
queue<long long> que;
void bfs(long long st) {
    que.push(st);
    while (!que.empty()) {
        long long cur = que.front();que.pop();
        
    }
}


vector<int> path;

int main() {
    array<array<array<short, 4>, 4>, 7040> magic_s = interpret_file("magicSquares.txt");
    // output magic_s value
    for (int i = 0; i < 7040; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 3; k++) {
                cout << magic_s[i][j][k] << " ";
            }
            cout << magic_s[i][j][3] << endl;
        }
        cout << endl;
    }


    return 0;
}