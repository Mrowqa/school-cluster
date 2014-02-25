#include <iostream>
#include <vector>
#include <mpi.h>
#include <ctime>
#include <cstdlib>
#include <string>
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
	static int curNumber = 0; // MPI Irecv can update this variable out of this function call
	int curSpeed = curNumber;
	curNumber = 0;
	int procCnt = MPI::COMM_WORLD.Get_size();
	for(int j=1; j<procCnt; j++) {
		while(MPI::COMM_WORLD.Irecv(&curNumber, 1, MPI::INT, j, SPEED_TAG).Get_status()) {
			attempts[j] += curNumber;
			curSpeed += curNumber;
		}
	}
	return curSpeed;
}

string formatTime(time_t time) {
	char buffer[32]="00:00:00";
	int seconds = time % 60;
	int minutes = time / 60 % 60;
	int hours = time / 3600;
	sprintf(buffer, "%02i:%02i:%02i", hours, minutes, seconds);
	return buffer;
}

void doMaster() {
	int procCnt = MPI::COMM_WORLD.Get_size();
	cout << "Available " << procCnt << " processors\n";
	
	int size;
	cout << "Type number of numbers: ";
	cin >> size;
	size = min(1024, max(0, size)); // size between 0 and 1024
	vector<int> nums(size, 0);
	vector<long long> attempts(procCnt, 0);
	vector<MPI::Request> nodesAnswers(procCnt);
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
	//MPI::Request req = MPI::COMM_WORLD.Irecv(&nums[0], size, MPI::INT, 1, DATA_TAG);
	for(int i=1; i<procCnt; i++)
		nodesAnswers[i] = MPI::COMM_WORLD.Irecv(&nums[0], size, MPI::INT, i, DATA_TAG);
	for(int i=1; ; i = i+1<procCnt ? i+1 : 1) {
		//MPI::Request req = MPI::COMM_WORLD.Irecv(&nums[0], size, MPI::INT, i, DATA_TAG);
		//if(req.Get_status()) {
		if(nodesAnswers[i].Get_status()) {
		MPI::COMM_WORLD.Recv(solvedBy, nameMaxLen, MPI::CHAR, i, COMM_TAG);
			totalTime = difftime(time(NULL), startTime);
			break;
		}
		if(i==1) {
			sleep(1); // sleep for 1 second
			int curSpeed = getSpeedInfo(attempts);
			cout << "\rAttempts per second: " << curSpeed << " attemps/sec\t\t"
				"Total time: " << formatTime(difftime(time(NULL), startTime)) << flush;
		}
	}
	
	cout << "\nSorted by " << solvedBy << "\n"
		<< "Sorted numbers:\n";
	for(int i=0; i<size; i++)
		cout << nums[i] << " ";
	cout << "\n";
	
	for(int i=1; i<procCnt; i++) {
		cout << "Shutting node " << i << " down...\n";
		MPI::COMM_WORLD.Isend(DONE_MSG, strlen(DONE_MSG)+1, MPI::CHAR, i, COMM_TAG); // FIXME perform error checking!
	}
	
	sleep(1); // wait for info
	getSpeedInfo(attempts);
	int totalAttemptsCnt = 0;
	for(int i=0; i<size; i++)
		totalAttemptsCnt += attempts[i]; // TODO print attempts by given node
	
	cout << "Total attempts: " << totalAttemptsCnt << endl;
	cout << "Average speed: " << (static_cast<double>(totalAttemptsCnt)/totalTime) << " attemps/sec\n";
	cout << "Total time: " << formatTime(difftime(time(NULL), startTime)) << "\n\n";
	
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
	
	const int msgMaxLen = 32;
	char msg[msgMaxLen];
	MPI::Request req = MPI::COMM_WORLD.Irecv(msg, msgMaxLen, MPI::CHAR, masterNode, COMM_TAG);
	
	while(true) {
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
		/*req =*/ MPI::COMM_WORLD.Isend(&nums[0], size, MPI::INT, masterNode, DATA_TAG);
		//if(req.Get_status() == false) {
		//	cerr << "[" << procName << "] cannot send sorted numbers!\n" << flush;
		//	continue;
		//}
		/*req =*/ MPI::COMM_WORLD.Isend(procName, procNameLen, MPI::CHAR, masterNode, COMM_TAG);
		//if(req.Get_status() == false) {
		//	cerr << "[" << procName << "] cannot send processor name!\n" << flush;
		//	continue;
		//}
		cout << "Node " << MPI::COMM_WORLD.Get_rank() << " finished!\n" << flush;
		break;
	}
	cout << "Node " << MPI::COMM_WORLD.Get_rank() << " exiting...\n" << flush;
	MPI::COMM_WORLD.Isend(&attempts, 1, MPI::INT, masterNode, SPEED_TAG); // FIXME perform error checking!
}
