#define WIN32_LEAN_AND_MEAN 

#pragma warning(disable : 4996)

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <WinSock2.h>
#include <time.h>

#if !defined(_Wp64)
#define DWORD_PTR DWORD
#define LONG_PTR LONG
#define INT_PTR INT
#endif

SECURITY_ATTRIBUTES stdOutSA = /* SA for inheritable handle. */
{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };


//////////////// Donut Bin info ////////////////
// create a bin that holds 4 types of donuts: 
// emptySem, filledSem, lockBin (mutex), int donutsIn, int donutsSold, int maxDonuts
typedef struct _DonutBins {
	HANDLE emptySem;
	HANDLE filledSem;
	HANDLE lockBinMutex;
	int donutsIn;
	int donutsSold;
	int maxDonuts;
} DonutBins;

//////////////// Baker info ////////////////
// baker struct
typedef struct _Bakers {
	HANDLE hBaker; 
	int donutTypes[4];
	int donutsToBake;
	int donutsBaked;
} Bakers;

//////////////// Line Manager info ////////////////
typedef struct _LineManagers {
	HANDLE hLineManager; 
	HANDLE donutAvailEvent;
	HANDLE workDoneEvent;
	BOOL alive;
} LineManagers;

//////////////// Worker info ////////////////
typedef struct _Workers {
	HANDLE workerGoEvent;
	HANDLE hWorker;
	int donutsSoldArr[4];
	int totalDonutsSold;
} Workers;

//////////////// Donut Line info ////////////////
typedef struct _DonutLine {
	HANDLE lockQueueMutex; 
	int workerQueue[6];
	int front;
	int back;
	int count;
} DonutLine;

#define numBakers 2
#define numDonuts 4
#define numWorkers 6

Bakers bakers[numBakers];
HANDLE bakerAliveMutex;	
HANDLE lineManagerAliveMutex;

DonutBins donutBins[numDonuts];			// array of structs containing donut info

Workers workerArr[numWorkers];
DonutLine donutLineArr[numDonuts];
LineManagers lineMngrArr[numDonuts];

int bakerAliveCount = numBakers;
int lineManagerAliveCount = numDonuts;

// funcitons
void baker(int);
void lineManagerFunc(int);
void worker(int);

int _tmain() {
	HANDLE workerHandles[numWorkers];
	int binSize = -1;				// holds the num of donuts in a bin by user
	int maxDonutsBaker1 = -1;		// holds the max num of donuts the baker can bake
	int maxDonutsBaker2 = -1;

	// get input from user
	printf("\nWelcome to Axl & Bailey's Donut Shop!");
	printf("\n\n\tFirst we need to gather some information!");

	printf("\n\n\t\tHow many donuts should fit in a bin: ");
	if (scanf("%d", &binSize) != 1) {
		printf("Invalid input: please enter an integer");
		return 1;
	}

	printf("\n\t\tHow many donuts should Baker 1 make: ");
	if (scanf("%d", &maxDonutsBaker1) != 1) {
		printf("Invalid input: please enter an integer");
		return 1;
	}

	printf("\n\t\tHow many donuts should Baker 2 make: ");
	if (scanf("%d", &maxDonutsBaker2) != 1) {
		printf("Invalid input: please enter an integer");
		return 1;
	}

	////////////////// starting bakers, line managers //////////////////
		// begin baker threads and init values

	bakers[0].donutsToBake = maxDonutsBaker1;
	bakers[1].donutsToBake = maxDonutsBaker2;

	for (int i = 0; i < numBakers; i++) {
		bakers[i].hBaker = (HANDLE)_beginthreadex(NULL, 0, baker, i, CREATE_SUSPENDED, NULL);
		bakers[i].donutsBaked = 0;
	}

		// begin line manager threads and init values
	for (int i = 0; i < numDonuts; i++) {
		lineMngrArr[i].hLineManager = (HANDLE)_beginthreadex(NULL, 0, lineManagerFunc, i, CREATE_SUSPENDED, NULL);
	}

		// beign worker threads and init vlaues
	for (int i = 0; i < numWorkers; i++) {
		workerArr[i].hWorker = (HANDLE)_beginthreadex(NULL, 0, worker, i, CREATE_SUSPENDED, NULL);
		workerHandles[i] = workerArr[i].hWorker; 
	}

	////////////////// Initializing semaphores, mutexs, events, initialize donutline values //////////////////
		// emptySem, filledSem, and lockbin init
	bakerAliveMutex = CreateMutex(NULL, FALSE, NULL);
	lineManagerAliveMutex = CreateMutex(NULL, FALSE, NULL);
	
	for (int i = 0; i < numDonuts; i++) {
		donutBins[i].emptySem = CreateSemaphore(NULL, 0, binSize, NULL);
		donutBins[i].filledSem = CreateSemaphore(NULL, binSize, binSize, NULL);
		donutBins[i].lockBinMutex = CreateMutex(NULL, FALSE, NULL);
		donutBins[i].maxDonuts = binSize;
		donutBins[i].donutsIn = 0;
		donutBins[i].donutsSold = 0;

		lineMngrArr[i].donutAvailEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		lineMngrArr[i].workDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		lineMngrArr[i].alive = TRUE; 

		donutLineArr[i].lockQueueMutex = CreateMutex(NULL, FALSE, NULL); 
		donutLineArr[i].front = 0;
		donutLineArr[i].back = 0;
		donutLineArr[i].count = 0;
	}

	// workerGo event init
	for (int i = 0; i < numWorkers; i++) {
		workerArr[i].workerGoEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		for (int j = 0; j < numDonuts; j++) {
			workerArr[i].donutsSoldArr[j] = 0;
		}
		workerArr[i].totalDonutsSold = 0;
	}

	/////////// resume threads ///////////
	for (int i = 0; i < numBakers; i++) {
		ResumeThread(bakers[i].hBaker);
	}

	for (int i = 0; i < numDonuts; i++) {
		ResumeThread(lineMngrArr[i].hLineManager); 
	}

	for (int i = 0; i < numWorkers; i++) {
		ResumeThread(workerArr[i].hWorker);
	}

	/// waiting ///
	for (int i = 0; i < numBakers; i++) {
		WaitForSingleObject(bakers[i].hBaker, INFINITE); 
	}

	for (int i = 0; i < numDonuts; i++) {
		WaitForSingleObject(lineMngrArr[i].hLineManager, INFINITE);  
	}

	for (int i = 0; i < numWorkers; i++) {
		WaitForSingleObject(workerArr[i].hWorker, INFINITE); 
	}

	////////////////// results //////////////////
	for (int i = 0; i < numBakers; i++) {
		printf("\n\n\tBaker %d:", i+1);
		printf("\n\t\tMade %d donuts", bakers[i].donutsBaked);
		for (int j = 0; j < 4; j++) {
			printf("\n\t\t\t%d type #%d", bakers[i].donutTypes[j], j+1);
		}
	}

	for (int i = 0; i < numDonuts; i++) {
		printf("\n\n\tDonut Bin #%d", i+1);
		printf("\n\t\t%d donuts", donutBins[i].donutsSold);
	}

	for (int i = 0; i < numWorkers; i++) {
		printf("\n\n\tWorker #%d", i+1);
		printf("\n\t\tSold %d donuts", workerArr[i].totalDonutsSold);
		for (int j = 0; j < numDonuts; j++) {
			printf("\n\t\t%d type #%d", workerArr[i].donutsSoldArr[j], j+1);
		}
	}

	printf("\n\n");
	system("pause");
	return 0;
}

void baker(int who) {
	int donutType = -1;

	// while all donuts not baked
	while(bakers[who].donutsBaked < bakers[who].donutsToBake) {
		// get random donut 0-3
		donutType = rand() % 4;

		// check if open spot for donut
		if (WaitForSingleObject(donutBins[donutType].emptySem, 10)) {
			// bake donut
			Sleep(5); 

			// lock specific donut bin
			WaitForSingleObject(donutBins[donutType].lockBinMutex, INFINITE);

			// update num of donuts in specific bin
			donutBins[donutType].donutsIn++;

			// unlock bin
			ReleaseMutex(donutBins[donutType].lockBinMutex);

			//release sem
			ReleaseSemaphore(donutBins[donutType].filledSem, 1, NULL); 

			// update bakers donuts made
			bakers[who].donutsBaked++;					// increment donuts made
			bakers[who].donutTypes[donutType]++;		// updating the donut bin for baker

			// signal line managers donut available
			SetEvent(lineMngrArr[donutType].donutAvailEvent);
		}
	}

	// lock baker alive
	WaitForSingleObject(bakerAliveMutex, INFINITE);
	bakerAliveCount--;
	if (bakerAliveCount == 0) {
		// signal all line managers donut available
		for (int i = 0; i < numDonuts; i++) { 
			SetEvent(lineMngrArr[i].donutAvailEvent);
		}
	}
	ReleaseMutex(bakerAliveMutex);

	/*printf("\n\t\tBaker #%d went home.", who);*/
}

void lineManagerFunc(int who) {
	int curWorker = -1;

	// stay in while bakers alive or there is a donut in the line managers bin
	while (bakerAliveCount > 0 || donutBins[who].donutsIn > 0) {
		// wait for donut avail event
		WaitForSingleObject(lineMngrArr[who].donutAvailEvent, INFINITE);	

		if (donutLineArr[who].count > 0 && donutBins[who].donutsIn > 0) {
			
			// dequeue the worker
			curWorker = donutLineArr[who].workerQueue[donutLineArr[who].front];

			// change front
			donutLineArr[who].front = (donutLineArr[who].front + 1) % 6;

			//lock the donut line arr queue and decrement count
			WaitForSingleObject(donutLineArr[who].lockQueueMutex, INFINITE);
			donutLineArr[who].count--;
			ReleaseMutex(donutLineArr[who].lockQueueMutex);

			// signal worker to go
			SetEvent(workerArr[curWorker].workerGoEvent);
			WaitForSingleObject(lineMngrArr[who].workDoneEvent, INFINITE);

			// if the managers bin still has donuts do stuff
			if (donutBins[who].donutsIn > 0) {
				// signal event donut available
				SetEvent(lineMngrArr[who].donutAvailEvent);
			}
		}
		else if (donutBins[who].donutsIn > 0){
			// signal event donut available
			SetEvent(lineMngrArr[who].donutAvailEvent); 
		}
	}

	// lock line manager alive
	WaitForSingleObject(lineManagerAliveMutex, INFINITE);
	lineManagerAliveCount--;	//decrement manager alive count
	lineMngrArr[who].alive = FALSE;		//set line manager alive to false
	ReleaseMutex(lineManagerAliveMutex);

	// while line is not empty deque workers
	while (donutLineArr[who].count > 0) {
		curWorker = donutLineArr[who].workerQueue[donutLineArr[who].front]; 
		donutLineArr[who].front = (donutLineArr[who].front + 1) % 6;  

		WaitForSingleObject(donutLineArr[who].lockQueueMutex, INFINITE); 
		donutLineArr[who].count--; 
		ReleaseMutex(donutLineArr[who].lockQueueMutex); 

		// signal worker to go
		SetEvent(workerArr[curWorker].workerGoEvent); 
	}

	/*printf("\n\t\tLine Manager #%d went home", who);*/
}

void worker(int who) {
	int donutType = -1;
	
	// while there are line managers
	while (lineManagerAliveCount > 0) {
		// take order
		donutType = rand() % 4; 

		// this causes and infinite loop/deadlock without it we only run into num donuts so later added another check
		//// add lock line manager alive here
		//WaitForSingleObject(lineManagerAliveMutex, INFINITE);

		// if the line manager for the donut is alive do stuff
		if (lineMngrArr[donutType].alive) {
			// lock the donut types worker que and add the worker
			WaitForSingleObject(donutLineArr[donutType].lockQueueMutex, INFINITE);
			donutLineArr[donutType].workerQueue[donutLineArr[donutType].back] = who;	// putting the who into the worker queue

			////unlock line manager alive here because we wanted to do it after we add to the queue
			//ReleaseMutex(lineManagerAliveMutex);	// this caused alot of issues
			donutLineArr[donutType].back = (donutLineArr[donutType].back + 1) % 6;			// making sure to change the front
			donutLineArr[donutType].count++;		// changing the count of how many workers
			
			ReleaseMutex(donutLineArr[donutType].lockQueueMutex);

			// wait on worker go event
			WaitForSingleObject(workerArr[who].workerGoEvent, INFINITE);

			// check if there is a donut available
			if (WaitForSingleObject(donutBins[donutType].filledSem, 10)) { 
				// ADDED this check because we kept getting into this wwith multiple workers and increment sold donuts by num of workers
				// this fixed it
				if (donutBins[donutType].donutsIn > 0) {
					// lock the bin
					WaitForSingleObject(donutBins[donutType].lockBinMutex, INFINITE);

					// update data in the donut bin
					donutBins[donutType].donutsIn--;
					donutBins[donutType].donutsSold++;
					ReleaseMutex(donutBins[donutType].lockBinMutex);

					// update worker data
					workerArr[who].donutsSoldArr[donutType]++;
					workerArr[who].totalDonutsSold++;

					// release empty semaphore
					ReleaseSemaphore(donutBins[donutType].emptySem, 1, NULL);

					//signal line manager work is done
					SetEvent(lineMngrArr[donutType].workDoneEvent);
				}
				else {
					// If there are no donuts left in the bin, signal the line manager that work is done
					SetEvent(lineMngrArr[donutType].workDoneEvent);
				}
			}
			else
			{
				SetEvent(lineMngrArr[donutType].workDoneEvent);	 
			}
		}
	}
	/*printf("\n\t\tWorker #%d went home", who);*/
}