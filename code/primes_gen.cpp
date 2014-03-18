#include <iostream>
#include <vector>
#include <mpi.h>
#include <string>
#include <deque>
#include <fstream>
#include <map>
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

#define PRIMES_SEGMENT_SIZE_TAG		1
#define PRIMES_SEGMENT_DATA_TAG		2
#define QUERY_TAG					3

// ---------------------------------------- MASTER ---------------------------
int loadCache(deque<vector<long long>>& primes, long long blockSize, string cacheLocation) {
	ifstream readStr;
	int loadedBlocks = 0;
	ostringstream filename;
	
	primes.clear();
	while(true) {
		filename.str("");
		filename << cacheLocation << "primes-" << (loadedBlocks*blockSize+1) <<
			"-" << (loadedBlocks*(blockSize+1)) << ".txt";
		readStr.open(filename.str(), ios::read);
		if(readStr.open()) {
			primes.push_back(vector<int>());
			long long primeNumber;
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

bool saveCacheSegment(const vector<long long>& primes, long long from, long long blockSize, string cacheLocation) {
	ostringstream filename;
	filename << cacheLocation << "primes-" << from << "-" << to << ".txt";
	ofstream writeStr(filename.str(), io::trunc);
	if(writeStr.open()) {
		for(int i=0; i< primes.size(); i++)
			writeStr << primes[i] << " ";
		writeStr.close();
	}
	else
		return false;
	
	return true;
}

void doMaster() {
	int procCnt = MPI::COMM_WORLD.Get_size();
	cout << "Available " << procCnt << " processors.\n";
	
	long long limit, blockSize;
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
	cin.sync(); // Clear '\n' at end of the stdin stream
	getline(cin, cacheLocation);
	if(cacheLocation[cacheLocation.size()-1] != '/')
		cacheLocation += '/';
	
	deque<vector<long long>> primes;
	if(!cacheLocation.empty()) {
		int cachedTo = loadCache(primes, blockSize, cacheLocation);
		cout << "Loaded cached primes to " << cachedTo << "!\n";
		
		int loadedBlocks = primes.size();
		for(int j=0; j<loadedBlocks; j++) {
			for(int i=1; i<procCnt; i++) {
				int segmentSize = primes[j].size();
				MPI::COMM_WORLD.Isend(&segmentSize, 1, MPI::INT, i, PRIMES_SEGMENT_SIZE_TAG);
				MPI::COMM_WORLD.Isend(&primes[j], &loadedBlocks, MPI::LONG_LONG, i, PRIMES_SEGMENT_DATA_TAG);
			}
		}
	}
	else
		cout << "Computing without cache!\n";
	
	map<int,vector<long long>*> awaitingBlocks; // something else than pointer...
	int nextBlockId = primes.size();
	for(int i=primes.size(); i*blockSize <= limit; i++) {
		
	}
	
	cout << "Exiting master function.\n";
}

// ---------------------------------------- SLAVE ----------------------------
void doSlave() {
	int masterNode = 0;
	
	cout << "Node " << MPI::COMM_WORLD.Get_rank() << " exiting...\n" << flush;
}
