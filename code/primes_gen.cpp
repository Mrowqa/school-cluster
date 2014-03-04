#include <iostream>
#include <vector>
#include <mpi.h>
#include <string>
#include <list>
#include <fstream>
using namespace std;

void doMaster();
void doSlave();

int main(int argc, char* argv[]) {
	MPI::Init(argc, argv);
	int rank = MPI::COMM_WORLD.Get_rank();
	
	if (rank == 0)
		doMaster();
	else
		doSlave();
	
	MPI::Finalize();
	return 0;
}

int loadCache(list<vector<int>>& primes, int blockSize, string cacheLocation) {
	fstream readStr;
	int loadedBlocks = 0;
	ostringstream filename;
	
	primes.clear();
	while(true) {
		filename.str("");
		filename << cacheLocation << "/primes-" << (loadedBlocks*blockSize+1) <<
			"-" << (loadedBlocks*(blockSize+1)) << ".txt";
		readStr.open(filename.str(), ios::read);
		if(readStr.open()) {
			primes.push_back(vector<int>());
			int primeNumber;
			while(readStr >> primeNumber)
				primes.back().push_back(primeNumber);
				
			readStr.close();
		}
		else
			break;
		readStr.clear();
	}
	
	return loadedBlocks * blockSize;
}

bool saveCacheSegment(const vector<int>& primes, int from, int blockSize, string cacheLocation) {
	
	return true;
}

void doMaster() {
	int procCnt = MPI::COMM_WORLD.Get_size();
	cout << "Available " << procCnt << " processors.\n";
	
	int limit, blockSize;
	string cacheLocation;
	cout << "Type searching upper bound: ";
	cin >> limit;
	cout << "Type searching block size: ";
	cin >> blockSize;
	if(blockSize <= 0) {
		cerr << "Error! Block size must be positive integer.\n";
		return;
	}
	cout << "Type cache location (empty line for no cache): ";
	cin.clear(); // Clear '\n' at end of the stdin stream
	getline(cin, cacheLocation);
	
	list<vector<int>> primes;
	if(!cacheLocation.empty()) {
		int cachedTo = loadCache(&primes, blockSize, cacheLocation);
		cout << "Loaded cached primes to " << cachedTo << "!\n";
	}
	else
		cout << "Computing without cache!\n";
	

	
	cout << "Exiting master function.\n";
}

void doSlave() {
	int masterNode = 0;
	
	cout << "Node " << MPI::COMM_WORLD.Get_rank() << " exiting...\n" << flush;
}
