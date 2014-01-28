#include <iostream>
#include <vector>
#include <mpi.h>
#include <ctime>
#include <cstdlib>
using namespace std;

void doMaster();
void doSlave();

int main(int argc, char* argv[]) {
	srand(time(NULL));
	MPI::Init(argc, argv);
	int rank = MPI::COMM_WORLD.Get_rank();
	
	if (rank == 0)
		doMaster();
	else
		doSlave();
	
	MPI::Finalize();
	return 0;
}

#define SIZE_TAG	0
#define DATA_TAG	1
#define COMM_TAG	2
#define SPEED_TAG	3

#define DONE_MSG "Done"


int getSpeedInfo(vector<long long>& attempts) {
	int curSpeed = 0;
	int procCnt = MPI::COMM_WORLD.Get_size();
	for(int j=1; j<procCnt; j++) {
		int curNumber = 0;
		while(MPI::COMM_WORLD.Irecv(&curNumber, 1, MPI::INT, j, SPEED_TAG).Get_status()) {
			attempts[j] += curNumber;
			curSpeed += curNumber;
		}
	}
	return curSpeed;
}

void doMaster() {
	int procCnt = MPI::COMM_WORLD.Get_size();
	cout << "Available " << procCnt << " processors\n";
	
	int size;
	cout << "Type number of numbers: ";
	cin >> size;
	size = min(1024, max(0, size)); // size between 0 and 1024
	vector<int> nums(size, 0);
	vector<long long> attempts(size, 0);
	cout << "Type " << size << " numbers: ";
	for(int i=0; i<size; i++)
		cin >> nums[i];
	
	time_t startTime = time(NULL);
	time_t totalTime = 0;
	for(int i=1; i<procCnt; i++) {
		MPI::COMM_WORLD.Send(&size, 1, MPI::INT, i, SIZE_TAG);
		MPI::COMM_WORLD.Send(&nums[0], size, MPI::INT, i, DATA_TAG);
	}
	
	const int nameMaxLen = 32;
	char solvedBy[nameMaxLen] = { 0 };
	for(int i=1; ; i = i+1<procCnt ? i+1 : 1) {
		MPI::Request req = MPI::COMM_WORLD.Irecv(&nums[0], size, MPI::INT, i, DATA_TAG);
		if(req.Get_status()) {
			MPI::COMM_WORLD.Recv(solvedBy, nameMaxLen, MPI::CHAR, i, COMM_TAG);
			totalTime = difftime(time(NULL), startTime);
			break;
		}
		if(i==1) {
			sleep(1); // sleep for 1 second
			int curSpeed = getSpeedInfo(attempts);
			cout << "\rAttempts per second: " << curSpeed;
		}
	}
	
	cout << "\nSorted by " << solvedBy << "\n"
		<< "Sorted numbers:\n";
	for(int i=0; i<size; i++)
		cout << nums[i] << " ";
	cout << "\n";
	
	for(int i=1; i<procCnt; i++) {
		cout << "Shutting node " << i << " down...\n";
		MPI::COMM_WORLD.Send(DONE_MSG, strlen(DONE_MSG)+1, MPI::CHAR, i, COMM_TAG);
	}
	
	sleep(1); // wait for info
	getSpeedInfo(attempts);
	int totalAttemptsCnt = 0;
	for(int i=0; i<size; i++)
		totalAttemptsCnt += attempts[i]; // TODO print attempts by given node
	
	cout << "Total attempts: " << totalAttemptsCnt << endl;
	cout << "Average speed: " << (static_cast<double>(totalAttemptsCnt)/totalTime) << " attemps/sec\n";
	
	cout << "Exiting master function.\n";
}

void shuffle(vector<int>& v) {
	int size = v.size();
	for(int i=0; i<size; i++)
		swap(v[i], v[rand()%size]);
}

void doSlave() {
	int masterNode = 0;
	int size;
	vector<int> nums;
	MPI::COMM_WORLD.Recv(&size, 1, MPI::INT, masterNode, SIZE_TAG);
	nums.resize(size);
	MPI::COMM_WORLD.Recv(&nums[0], size, MPI::INT, masterNode, DATA_TAG);
	
	time_t lastUpdate = time(NULL);
	int attempts = 0;
	
	while(true) {
		const int msgMaxLen = 32;
		char msg[msgMaxLen];
		MPI::Request req = MPI::COMM_WORLD.Irecv(msg, msgMaxLen, MPI::CHAR, masterNode, COMM_TAG);
		if(req.Get_status() && strcmp(msg, DONE_MSG) == 0)
			break;
		
		attempts++;
		if(lastUpdate != time(NULL)) {
			lastUpdate = time(NULL);
			MPI::COMM_WORLD.Isend(&attempts, 1, MPI::INT, masterNode, SPEED_TAG); // FIXME perform error checking!
			attempts = 0;
		}
		
		shuffle(nums);
		bool goodOrder = true;
		for(int i=0; i<size-1; i++)
			if(nums[i] > nums[i+1]) {
				goodOrder = false;
				break;
			}
		if(goodOrder == false)
			continue;
		
		char procName[msgMaxLen] = { 0 };
		int procNameLen = 0;
		MPI::Get_processor_name(procName, procNameLen);
		MPI::COMM_WORLD.Send(&nums[0], size, MPI::INT, masterNode, DATA_TAG);
		MPI::COMM_WORLD.Send(procName, procNameLen, MPI::CHAR, masterNode, COMM_TAG);
		break;
	}
	//cout << "Node " << MPI::COMM_WORLD.Get_rank() << " exiting...";
	MPI::COMM_WORLD.Send(&attempts, 1, MPI::INT, masterNode, SPEED_TAG);
}
