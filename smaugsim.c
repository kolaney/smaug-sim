#include <errno.h> 
#include <wait.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/resource.h> 


/* Define semaphores to be placed in a single semaphore set */
/* Numbers indicate index in semaphore set for named semaphore */

#define SEM_COWSINGROUP 0
#define SEM_PCOWSINGROUP 1
#define SEM_COWSWAITING 2
#define SEM_PCOWSEATEN 3
#define SEM_COWSEATEN 4
#define SEM_COWSDEAD 5
#define SEM_PTERMINATE 6
#define SEM_DRAGONEATING 7
#define SEM_DRAGONFIGHTING 8
#define SEM_DRAGONSLEEPING 9
#define SEM_PMEALWAITINGFLAG 10

//added here
#define SEM_SHEEPINGROUP 11	
#define SEM_PSHEEPINGROUP 12
#define SEM_SHEEPWAITING 13

#define SEM_PSHEEPEATEN 14
#define SEM_SHEEPEATEN 15
#define SEM_SHEEPDEAD 16

#define SEM_THIEVESWAITING 17
#define SEM_PTHIEVESWAITING 18
#define SEM_THIEVESPLAYED 19
#define SEM_THIEFPLAYING 20

#define SEM_HUNTERSWAITING 21
#define SEM_PHUNTERSWAITING 22
#define SEM_HUNTERSFOUGHT 23
#define SEM_HUNTERFIGHTING 24

#define SEM_GAMEDONE 25

#define SEM_DRAGONPLAYING 26

/* System constants used to control simulation termination */
#define MAX_COWS_EATEN 35
#define MAX_COWS_CREATED 500
#define MAX_TREASURE_IN_HOARD 1000
#define MIN_TREASURE_IN_HOARD 0

//added here
#define MAX_SHEEP_EATEN 35
#define MAX_SHEEP_CREATED 500
#define MAX_THIEVES_DEFEATED 48
#define MAX_THIEVES_CREATED 500
#define MAX_HUNTERS_DEFEATED 48
#define MAX_HUNTERS_CREATED 500

/* System constants to specify size of groups of cows*/
#define COWS_IN_GROUP 2
#define SHEEP_IN_GROUP 3

/* CREATING YOUR SEMAPHORES */
int semID; 

union semun
{
	int val;
	struct semid_ds *buf;
	ushort *array;
} seminfo;

struct timeval startTime;


/*  Pointers and ids for shared memory segments */
int *terminateFlagp = NULL;
int *cowCounterp = NULL;
int *cowsEatenCounterp = NULL;

int *mealWaitingFlagp = NULL;
int terminateFlag = 0;

int cowCounter = 0;
int cowsEatenCounter = 0;

int mealWaitingFlag = 0;

//added here
int *sheepCounterp = NULL;
int *sheepEatenCounterp = NULL;
int sheepCounter = 0;
int sheepEatenCounter = 0;

int *thievesWaitingCounterp=NULL;
int thievesWaitingCounter=0;

int thievesPlayedCounterp=NULL;
int thievesPlayedCounter=0;

int *huntersWaitingCounterp=NULL;
int huntersWaitingCounter=0;

int *huntersFoughtCounterp=NULL;
int huntersFoughtCounter=0;

/* Group IDs for managing/removing processes */
int smaugProcessID = -1;
int cowProcessGID = -1;
int parentProcessGID = -1;

//added here
int sheepProcessGID = -1;
int thiefProcessGID = -1;
int hunterProcessGID = -1;

/* Define the semaphore operations for each semaphore */
/* Arguments of each definition are: */
/* Name of semaphore on which the operation is done */
/* Increment (amount added to the semaphore when operation executes*/
/* Flag values (block when semaphore <0, enable undo ...)*/

/*Number in group semaphores*/
struct sembuf WaitCowsInGroup={SEM_COWSINGROUP, -1, 0};
struct sembuf SignalCowsInGroup={SEM_COWSINGROUP, 1, 0};

//added here
struct sembuf WaitSheepInGroup={SEM_SHEEPINGROUP, -1, 0};
struct sembuf SignalSheepInGroup={SEM_SHEEPINGROUP, 1, 0};


struct sembuf WaitThievesWaiting={SEM_THIEVESWAITING, -1, 0};
struct sembuf SignalThievesWaiting={SEM_THIEVESWAITING, 1, 0};


struct sembuf WaitHuntersWaiting={SEM_HUNTERSWAITING, -1, 0};
struct sembuf SignalHuntersWaiting={SEM_HUNTERSWAITING, 1, 0};

/*Number in group mutexes*/
struct sembuf WaitProtectCowsInGroup={SEM_PCOWSINGROUP, -1, 0};
struct sembuf WaitProtectMealWaitingFlag={SEM_PMEALWAITINGFLAG, -1, 0};

struct sembuf SignalProtectCowsInGroup={SEM_PCOWSINGROUP, 1, 0};
struct sembuf SignalProtectMealWaitingFlag={SEM_PMEALWAITINGFLAG, 1, 0};

//added here
struct sembuf WaitProtectSheepInGroup={SEM_PSHEEPINGROUP, -1, 0};
struct sembuf SignalProtectSheepInGroup={SEM_PSHEEPINGROUP, 1, 0};



/*Number waiting sempahores*/
struct sembuf WaitCowsWaiting={SEM_COWSWAITING, -1, 0};
struct sembuf SignalCowsWaiting={SEM_COWSWAITING, 1, 0};

//added here
struct sembuf WaitSheepWaiting={SEM_SHEEPWAITING, -1, 0};
struct sembuf SignalSheepWaiting={SEM_SHEEPWAITING, 1, 0};

struct sembuf WaitProtectThievesWaiting={SEM_PTHIEVESWAITING, -1, 0};
struct sembuf SignalProtectThievesWaiting={SEM_PTHIEVESWAITING, 1, 0};

struct sembuf WaitProtectHuntersWaiting={SEM_PHUNTERSWAITING, -1, 0};
struct sembuf SignalProtectHuntersWaiting={SEM_PHUNTERSWAITING, 1, 0};


/*Number eaten or fought semaphores*/
struct sembuf WaitCowsEaten={SEM_COWSEATEN, -1, 0};
struct sembuf SignalCowsEaten={SEM_COWSEATEN, 1, 0};

//added here 
struct sembuf WaitSheepEaten={SEM_SHEEPEATEN, -1, 0};
struct sembuf SignalSheepEaten={SEM_SHEEPEATEN, 1, 0};

struct sembuf WaitThiefPlaying={SEM_THIEFPLAYING, -1, 0};
struct sembuf SignalThiefPlaying={SEM_THIEFPLAYING, 1, 0};

struct sembuf WaitHunterFighting={SEM_HUNTERFIGHTING, -1, 0};
struct sembuf SignalHunterFighting={SEM_HUNTERFIGHTING, 1, 0};



/*Number eaten or fought mutexes*/
struct sembuf WaitProtectCowsEaten={SEM_PCOWSEATEN, -1, 0};
struct sembuf SignalProtectCowsEaten={SEM_PCOWSEATEN, 1, 0};

//added here
struct sembuf WaitProtectSheepEaten={SEM_PSHEEPEATEN, -1, 0};
struct sembuf SignalProtectSheepEaten={SEM_PSHEEPEATEN, 1, 0};


/*Number Dead semaphores*/
struct sembuf WaitCowsDead={SEM_COWSDEAD, -1, 0};
struct sembuf SignalCowsDead={SEM_COWSDEAD, 1, 0};

//added here
struct sembuf WaitSheepDead={SEM_COWSDEAD, -1, 0};
struct sembuf SignalSheepDead={SEM_COWSDEAD, 1, 0};

struct sembuf WaitThievesPlayed={SEM_THIEVESPLAYED, -1, 0};
struct sembuf SignalThievesPlayed={SEM_THIEVESPLAYED, 1, 0};

struct sembuf WaitHuntersFought={SEM_HUNTERSFOUGHT, -1, 0};
struct sembuf SignalHuntersFought={SEM_HUNTERSFOUGHT, 1, 0};


/*Dragon Semaphores*/
struct sembuf WaitDragonEating={SEM_DRAGONEATING, -1, 0};
struct sembuf SignalDragonEating={SEM_DRAGONEATING, 1, 0};

struct sembuf WaitDragonFighting={SEM_DRAGONFIGHTING, -1, 0};
struct sembuf SignalDragonFighting={SEM_DRAGONFIGHTING, 1, 0};

struct sembuf SignalDragonSleeping={SEM_DRAGONSLEEPING, 1, 0};
struct sembuf WaitDragonSleeping={SEM_DRAGONSLEEPING, -1, 0};

//added here
struct sembuf WaitDragonPlaying={SEM_DRAGONPLAYING, -1, 0};
struct sembuf SignalDragonPlaying={SEM_DRAGONPLAYING, 1, 0};

struct sembuf WaitGameDone={SEM_GAMEDONE, -1, 0};
struct sembuf SignalGameDone={SEM_GAMEDONE, 1, 0};

/*Termination Mutex*/
struct sembuf WaitProtectTerminate={SEM_PTERMINATE, -1, 0};
struct sembuf SignalProtectTerminate={SEM_PTERMINATE, 1, 0};


double timeChange( struct timeval starttime );
void initialize();
void smaug(float winProb);
void cow(int startTimeN);
void sheep(int startTimeN); //added here
void thief(int startTimeN); //added here
void hunter(int startTimeN);
void terminateSimulation();
void releaseSemandMem();
void semopChecked(int semaphoreID, struct sembuf *operation, unsigned something); 
void semctlChecked(int semaphoreID, int semNum, int flag, union semun seminfo); 

int main( )
{
	int k;
	int temp;

	/* variables to hold process ID numbers */
	int parentPID = 0;
	int cowPID = 0;
	int smaugPID = 0;
	int thiefPID = 0; //added here
	int sheepPID = 0; //added here
	int hunterPID = 0; //added here

	/* local counters, keep track of total number */
	/* of processes of each type created */
	int cowsCreated = 0;

	int sheepCreated = 0; //added here
	int thiefCreated = 0; //added here
	int hunterCreated = 0; //added here

	/* Variables to control the time between the arrivals */
	/* of successive beasts*/ 
	double minwait = 0;
	int newSeed = 0;
	int sleepingTime = 0;
	int maxCowIntervalUsec = 0;
	int maxSheepIntervalUsec = 0; //added here
	int maxThiefIntervalUsec = 0; //added here
	int maxHunterIntervalUsec = 0; //added here

	int nextInterval = 0.0;
	int status;
	int w = 0;
	double maxCowInterval = 0.0;
	double totalCowInterval = 0.0;

	double maxSheepInterval = 0.0; //added here
	double totalSheepInterval = 0.0; //added here

	double maxThiefInterval = 0.0; //added here
	double totalThiefInterval = 0.0; //added here
	double maxHunterInterval = 0.0; //added here
	double totalHunterInterval = 0.0; //added here

	float winProb = 0.0;

	double elapsedTime;
	double hold;

	parentPID = getpid();
	setpgid(parentPID, parentPID);
	parentProcessGID = getpgid(0);
	printf("CRCRCRCRCRCRCRCRCRCRCRCR  main process group  %d %d\n", parentPID, parentProcessGID);

	/* initialize semaphores and allocate shared memory */
	initialize();

	/* inialize each variable in shared memory */
	*cowCounterp = 0;
	*cowsEatenCounterp = 0;
	*mealWaitingFlagp = 0;

	//added here
	*sheepCounterp = 0;
	*sheepEatenCounterp = 0;
	*thievesWaitingCounterp = 0;
	*huntersWaitingCounterp = 0;


	printf("Please enter a random seed to start the simulation\n");
	scanf("%d",&newSeed);
	srand(newSeed);
	printf("Please enter the maximum interval length for cow (ms)");
	scanf("%lf", &maxCowInterval);
	maxCowIntervalUsec = (int)maxCowInterval * 1000;
	printf("max Cow interval time %f \n", maxCowInterval);

	//added here
	printf("Please enter the maximum interval length for sheep (ms)");
	scanf("%lf", &maxSheepInterval);
	maxSheepIntervalUsec = (int)maxSheepInterval * 1000;
	printf("max Sheep interval time %f \n", maxSheepInterval);

	//added here
	printf("Please enter the maximum interval length for thief (ms)");
	scanf("%lf", &maxThiefInterval);
	maxThiefIntervalUsec = (int)maxThiefInterval * 1000;
	printf("max Thief interval time %f \n", maxThiefInterval);

	//added here
	printf("Please enter the maximum interval length for hunter (ms)");
	scanf("%lf", &maxHunterInterval);
	maxHunterIntervalUsec = (int)maxHunterInterval * 1000;
	printf("max Hunter interval time %f \n", maxHunterInterval);

	//added here
	printf("Please enter the probability that a thief or hunter will win (float)");
	scanf("%f", &winProb);
	printf("winProb is %f \n", winProb);

	gettimeofday(&startTime,NULL);

	if ( ( smaugPID = fork())==0) {
		printf("CRCRCRCRCRCRCRCRCRCRCRCR  Smaug is born\n");
		smaug(winProb); 
		printf("CRCRCRCRCRCRCRCRCRCRCRCR  Smaug dies\n");
		exit(0);
	}
	else {
		if(smaugProcessID == -1) smaugProcessID = smaugPID;
		setpgid(smaugPID, smaugProcessID);
		printf("CRCRCRCRCRCRCRCRCRCRCRCR  Smaug PID %8d PGID %8d\n", smaugPID, smaugProcessID);
	}

	printf("CRCRCRCRCRCRCRCRCRCRCRCR  Smaug PID  create cow %8d \n", smaugPID);
      usleep(10);
       while( 1 ) {

		semopChecked(semID, &WaitProtectTerminate, 1);
       		if( *terminateFlagp != 0)  {
			semopChecked(semID, &SignalProtectTerminate, 1);
			break;
		}
		semopChecked(semID, &SignalProtectTerminate, 1);

		/* Create a cow process if needed */
		/* The condition used to determine if a process is needed is */
		/* if the last cow created will be enchanted */
		elapsedTime = timeChange(startTime);

		if( totalCowInterval - elapsedTime < totalCowInterval) {

			nextInterval =(int)((double)rand() / RAND_MAX *  maxCowIntervalUsec);
			totalCowInterval += nextInterval/1000.0;
			sleepingTime = (int)( (double)rand()/ RAND_MAX *  maxCowIntervalUsec);

			if ( (cowPID = fork())==0) {
				/* Child becomes a beast */
				elapsedTime = timeChange(startTime);
				cow( sleepingTime );
				/* Child (beast) quits after being consumed */
				exit(0);
			}
			else if( cowPID > 0) {
				cowsCreated++;
				if(cowProcessGID == -1){ 
					cowProcessGID = cowPID;
					printf("CRCRCRCRCR %8d  CRCRCRCRCR  cow PGID %8d \n", cowPID, cowProcessGID);
				}
				setpgid(cowPID, cowProcessGID);
				printf("CRCRCRCRCRCRCRCRCRCRCRCR   NEW COW CREATED %8d \n", cowsCreated);
			}
			else {
				printf("CRCRCRCRCRCRCRCRCRCRCRCRcow process not created \n");
				continue;
			}
			if( cowsCreated == MAX_COWS_CREATED ) {
				printf("CRCRCRCRCRCRCRCRCRCRCRCR Exceeded maximum number of cows\n");
				*terminateFlagp = 1;
				break;
			}
		}
		/* Create a sheep process if needed */
		if( totalSheepInterval - elapsedTime < totalSheepInterval) {

			nextInterval =(int)((double)rand() / RAND_MAX *  maxSheepIntervalUsec);
			totalSheepInterval += nextInterval/1000.0;
			sleepingTime = (int)( (double)rand()/ RAND_MAX *  maxSheepIntervalUsec);

			if ( (sheepPID = fork())==0) {
				/* Child becomes a beast */
				elapsedTime = timeChange(startTime);
				sheep( sleepingTime );
				/* Child (beast) quits after being consumed */
				exit(0);
			}
			else if( sheepPID > 0) {
				sheepCreated++;
				if(sheepProcessGID == -1){ 
					sheepProcessGID = sheepPID;
					printf("CRCRCRCRCR %8d  CRCRCRCRCR  sheep PGID %8d \n", sheepPID, sheepProcessGID);
				}
				setpgid(sheepPID, sheepProcessGID);
				printf("CRCRCRCRCRCRCRCRCRCRCRCR   NEW sheep CREATED %8d \n", sheepCreated);
			}
			else {
				printf("CRCRCRCRCRCRCRCRCRCRCRCR sheep process not created \n");
				continue;
			}
			if( sheepCreated == MAX_SHEEP_CREATED ) {
				printf("CRCRCRCRCRCRCRCRCRCRCRCR Exceeded maximum number of sheep\n");
				*terminateFlagp = 1;
				break;
			}
		}

		
		// Create a thief process if needed 
		if( totalThiefInterval - elapsedTime < totalThiefInterval) {

			nextInterval =(int)((double)rand() / RAND_MAX *  maxThiefIntervalUsec);
			totalThiefInterval += nextInterval/1000.0;
			sleepingTime = (int)( (double)rand()/ RAND_MAX *  maxThiefIntervalUsec);

			if ( (thiefPID = fork())==0) {
				// Child becomes a thief 
				elapsedTime = timeChange(startTime);
				thief( sleepingTime );
				// Child (beast) quits after being consumed 
				exit(0);
			}
			else if( thiefPID > 0) {
				thiefCreated++;
				if(thiefProcessGID == -1){ 
					thiefProcessGID = thiefPID;
					printf("CRCRCRCRCR %8d  CRCRCRCRCR  thief PGID %8d \n", thiefPID, thiefProcessGID);
				}
				setpgid(thiefPID, thiefProcessGID);
				printf("CRCRCRCRCRCRCRCRCRCRCRCR   NEW thief CREATED %8d \n", thiefCreated);
			}
			else {
				printf("CRCRCRCRCRCRCRCRCRCRCRCR thief process not created \n");
				continue;
			}
			if( thiefCreated == MAX_THIEVES_CREATED ) {
				printf("CRCRCRCRCRCRCRCRCRCRCRCR Exceeded maximum number of thief\n");
				*terminateFlagp = 1;
				break;
			}
		}
		
		
		// Create a hunter process if needed 
		if( totalHunterInterval - elapsedTime < totalHunterInterval) {

			nextInterval =(int)((double)rand() / RAND_MAX *  maxHunterIntervalUsec);
			totalHunterInterval += nextInterval/1000.0;
			sleepingTime = (int)( (double)rand()/ RAND_MAX *  maxHunterIntervalUsec);

			if ( (hunterPID = fork())==0) {
				// Child becomes a hunter 
				elapsedTime = timeChange(startTime);
				hunter( sleepingTime );
				// Child (hunter) quits after being consumed 
				exit(0);
			}
			else if( hunterPID > 0) {
				hunterCreated++;
				if(hunterProcessGID == -1){ 
					hunterProcessGID = hunterPID;
					printf("CRCRCRCRCR %8d  CRCRCRCRCR  hunter PGID %8d \n", hunterPID, hunterProcessGID);
				}
				setpgid(hunterPID, hunterProcessGID);
				printf("CRCRCRCRCRCRCRCRCRCRCRCR   NEW hunter CREATED %8d \n", hunterCreated);
			}
			else {
				printf("CRCRCRCRCRCRCRCRCRCRCRCR hunter process not created \n");
				continue;
			}
			if( hunterCreated == MAX_HUNTERS_CREATED ) {
				printf("CRCRCRCRCRCRCRCRCRCRCRCR Exceeded maximum number of hunters\n");
				*terminateFlagp = 1;
				break;
			}
		}
		


		/* wait for processes that have exited so we do not accumulate zombie or cows*/
		while( (w = waitpid( -1, &status, WNOHANG)) > 1){
		       	if (WIFEXITED(status)){
			       	if (WEXITSTATUS(status) > 0) {
					printf("exited, status=%8d\n", WEXITSTATUS(status));
					terminateSimulation();
					printf("GOODBYE from main process %8d\n", getpid());
					exit(0);
				}
				else {
					printf("                           REAPED process %8d\n", w);
				}
			}
		}

		/* terminateFlagp is set to 1 (from initial value of 0) when any */
		/* termination condition is satisfied (max sheep eaten ... ) */
		if( *terminateFlagp == 1 )break;

		/*sleep for 80% of the cow interval */
		/*then try making more processes */
	        usleep( (totalCowInterval*800) );

		usleep( (totalSheepInterval*800) );
	}
	if( *terminateFlagp == 1 ) {
		terminateSimulation();
	}
	printf("GOODBYE from main process %8d\n", getpid());
	exit(0);
}


void smaug(float winProb)
{
	int k;
	int temp;
	int localpid;
	double elapsedTime;

	/* local counters used only for smaug routine */
	int cowsEatenTotal = 0;
	int sheepEatenTotal = 0;
	int ThievesDefeatedTotal = 0;
	int HuntersDefeatedTotal = 0;
	float dice = 0;
	int jewels = 500;


	/* Initialize random number generator*/
	/* Random numbers are used to determine the time between successive beasts */
	smaugProcessID = getpid();
	printf("SMAUGSMAUGSMAUGSMAUGSMAU   PID is %d \n", smaugProcessID );
	localpid = smaugProcessID;
	printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has gone to sleep\n" );
	semopChecked(semID, &WaitDragonSleeping, 1);
	printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has woken up \n" );

	while (TRUE) {		
		semopChecked(semID, &WaitProtectMealWaitingFlag, 1);
		if( *mealWaitingFlagp >= 1  ) {
			*mealWaitingFlagp = *mealWaitingFlagp - 1;
			printf("SMAUGSMAUGSMAUGSMAUGSMAU   signal meal flag %d\n", *mealWaitingFlagp);
			semopChecked(semID, &SignalProtectMealWaitingFlag, 1);
			printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug is eating a meal\n");
			for( k = 0; k < COWS_IN_GROUP; k++ ) {
				semopChecked(semID, &SignalCowsWaiting, 1);
				printf("SMAUGSMAUGSMAUGSMAUGSMAU   A cow is ready to eat\n");
			}
			for( k = 0; k < SHEEP_IN_GROUP; k++ ) {
				semopChecked(semID, &SignalSheepWaiting, 1);
				printf("SMAUGSMAUGSMAUGSMAUGSMAU   A sheep is ready to eat\n");
			}

			/*Smaug waits to eat*/
			semopChecked(semID, &WaitDragonEating, 1);
			for( k = 0; k < COWS_IN_GROUP; k++ ) {
				semopChecked(semID, &SignalCowsDead, 1);
				cowsEatenTotal++;
				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug finished eating a cow\n");
			}

			for( k = 0; k < SHEEP_IN_GROUP; k++ ) {
				semopChecked(semID, &SignalSheepDead, 1);
				sheepEatenTotal++;
				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug finished eating a sheep\n");
			}
			printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has finished a meal\n");

	
			if(cowsEatenTotal >= MAX_COWS_EATEN ) {
				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has eaten the allowed number of cows\n");
				*terminateFlagp= 1;
				break; 
			}

			if(sheepEatenTotal >= MAX_SHEEP_EATEN ) {
				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has eaten the allowed number of sheep\n");
				*terminateFlagp= 1;
				break; 
			}

				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has finished all meals\n");
				semopChecked(semID, &SignalProtectMealWaitingFlag, 1);
				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug sleeps again\n");
				semopChecked(semID, &WaitDragonSleeping, 1);
				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug is awake again\n");

		}
		else{
			//BRING IN THIEVES
			semopChecked(semID, &SignalProtectMealWaitingFlag, 1);
			semopChecked(semID, &WaitProtectThievesWaiting, 1);

			printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug checks for visitors\n");


			printf("SMAUGSMAUGSMAUGSMAUGSMAU   There are %d thieves waiting\n", *thievesWaitingCounterp);
			printf("SMAUGSMAUGSMAUGSMAUGSMAU   There are %d hunters waiting\n", *huntersWaitingCounterp);
			
			if(*thievesWaitingCounterp >=1){
				
				
			
				*thievesWaitingCounterp = *thievesWaitingCounterp - 1;

				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug invites Thief into cave\n");

				semopChecked(semID, &SignalThievesWaiting, 1);
				semopChecked(semID, &SignalProtectThievesWaiting, 1);

				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug is waiting for Thief to be ready\n");

				semopChecked(semID, &WaitDragonPlaying, 1);

				//Check Win or Lose

				dice = (float)rand() / RAND_MAX;
				//if dice>winProb, Smaug Wins
				if (dice>winProb){
					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has won against Thief!\n");
					ThievesDefeatedTotal = ThievesDefeatedTotal + 1;

					if(ThievesDefeatedTotal >= MAX_THIEVES_DEFEATED) {
						printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has defeated the allowed number of thieves\n");
						*terminateFlagp= 1;
						break; 
					}	
					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has won 20 jewels\n");
					jewels +=20;
					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug now has %i jewels\n", jewels);
					if(jewels >= MAX_TREASURE_IN_HOARD) {
						printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has too much treasure!\n");
						*terminateFlagp= 1;
						break; 
					}	
				}

				else{
					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has lost against Thief!\n");
					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has lost 8 jewels\n");
					jewels -=8;
					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug now has %i jewels\n", jewels);
					if(jewels <= MIN_TREASURE_IN_HOARD) {
						printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has no more treasure left!\n");
						*terminateFlagp= 1;
						break; 
					}	

				}

				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug is done playing with Thief\n");
				printf("SMAUGSMAUGSMAUGSMAUGSMAU   Thief is released\n");

				semopChecked(semID, &SignalThievesPlayed, 1);


				semopChecked(semID, &WaitProtectThievesWaiting, 1);

				
			}

				semopChecked(semID, &SignalProtectThievesWaiting, 1);

				//BRING IN HUNTERS

				

				semopChecked(semID, &WaitProtectHuntersWaiting, 1);

				if(*huntersWaitingCounterp >=1){

					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug invites Hunter into cave\n");
					printf("SMAUGSMAUGSMAUGSMAUGSMAU   %d hunters left\n", *huntersWaitingCounterp);
				
					*huntersWaitingCounterp = *huntersWaitingCounterp - 1;

					semopChecked(semID, &SignalHuntersWaiting, 1);

					
					semopChecked(semID, &SignalProtectHuntersWaiting, 1);

					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug is waiting for Hunter to be ready\n");

					semopChecked(semID, &WaitDragonFighting, 1);

					//Check Win or Lose

					dice = (float)rand() / RAND_MAX;

					//if dice>winProb, Smaug Wins
					if (dice>winProb){
						printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has won against Hunter!\n");
						HuntersDefeatedTotal = HuntersDefeatedTotal + 1;
						if(HuntersDefeatedTotal >= MAX_HUNTERS_DEFEATED) {
							printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has defeated the allowed number of hunters\n");
							*terminateFlagp= 1;
							break; 
						}	
						printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has won 5 jewels\n");
						jewels +=5;
						printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug now has %i jewels\n", jewels);
						if(jewels >= MAX_TREASURE_IN_HOARD) {
							printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has too much treasure!\n");
							*terminateFlagp= 1;
							break; 
						}	
					}

					else{
						printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has lost against Hunter!\n");
						printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug has lost 10 jewels\n");
						jewels -=10;
						printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug now has %i jewels\n", jewels);
						if(jewels <= MIN_TREASURE_IN_HOARD) {
							printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug no more treasure left!\n");
							*terminateFlagp= 1;
							break; 
						}	

					}


					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug is done playing with Hunter\n");
					printf("SMAUGSMAUGSMAUGSMAUGSMAU   Hunter is released\n");

					semopChecked(semID, &SignalHuntersFought, 1);

					semopChecked(semID, &WaitProtectHuntersWaiting, 1);
				}

			
			semopChecked(semID, &SignalProtectHuntersWaiting, 1);


			printf("SMAUGSMAUGSMAUGSMAUGSMAU   All visitors have been dealt with.\n");	
			printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug sleeps again\n");
			semopChecked(semID, &WaitDragonSleeping, 1);	
			printf("SMAUGSMAUGSMAUGSMAUGSMAU   Smaug wakes again\n");

		}






	}
}


void initialize()
{
	/* Init semaphores */
	semID=semget(IPC_PRIVATE, 40, 0666 | IPC_CREAT);


	/* Init to zero, no elements are produced yet */
	seminfo.val=0;
	semctlChecked(semID, SEM_COWSINGROUP, SETVAL, seminfo);
	semctlChecked(semID, SEM_COWSWAITING, SETVAL, seminfo);
	semctlChecked(semID, SEM_COWSEATEN, SETVAL, seminfo);
	semctlChecked(semID, SEM_COWSDEAD, SETVAL, seminfo);

	semctlChecked(semID, SEM_SHEEPINGROUP, SETVAL, seminfo);
	semctlChecked(semID, SEM_SHEEPWAITING, SETVAL, seminfo);
	semctlChecked(semID, SEM_SHEEPEATEN, SETVAL, seminfo);
	semctlChecked(semID, SEM_SHEEPDEAD, SETVAL, seminfo);

	semctlChecked(semID, SEM_THIEVESWAITING, SETVAL, seminfo);
	semctlChecked(semID, SEM_THIEVESPLAYED, SETVAL, seminfo);
	semctlChecked(semID, SEM_THIEFPLAYING, SETVAL, seminfo);

	semctlChecked(semID, SEM_HUNTERSWAITING, SETVAL, seminfo);
	semctlChecked(semID, SEM_HUNTERSFOUGHT, SETVAL, seminfo);
	semctlChecked(semID, SEM_HUNTERFIGHTING, SETVAL, seminfo);

	semctlChecked(semID, SEM_GAMEDONE, SETVAL, seminfo);

	semctlChecked(semID, SEM_DRAGONPLAYING, SETVAL, seminfo);
	semctlChecked(semID, SEM_DRAGONFIGHTING, SETVAL, seminfo);
	semctlChecked(semID, SEM_DRAGONSLEEPING, SETVAL, seminfo);
	semctlChecked(semID, SEM_DRAGONEATING, SETVAL, seminfo);
	printf("!!INIT!!INIT!!INIT!!  semaphores initiialized\n");
	
	/* Init Mutex to one */
	seminfo.val=1;
	semctlChecked(semID, SEM_PCOWSINGROUP, SETVAL, seminfo);

	semctlChecked(semID, SEM_PSHEEPINGROUP, SETVAL, seminfo);

	semctlChecked(semID, SEM_PTHIEVESWAITING, SETVAL, seminfo);
	semctlChecked(semID, SEM_PHUNTERSWAITING, SETVAL, seminfo);

	semctlChecked(semID, SEM_PMEALWAITINGFLAG, SETVAL, seminfo);

	semctlChecked(semID, SEM_PCOWSEATEN, SETVAL, seminfo);
	semctlChecked(semID, SEM_PSHEEPEATEN, SETVAL, seminfo);
	semctlChecked(semID, SEM_PTERMINATE, SETVAL, seminfo);
	printf("!!INIT!!INIT!!INIT!!  mutexes initiialized\n");


	/* Now we create and attach  the segments of shared memory*/
        if ((terminateFlag = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0) {
                printf("!!INIT!!INIT!!INIT!!  shm not created for terminateFlag\n");
                 exit(1);
        }
        else {
                printf("!!INIT!!INIT!!INIT!!  shm created for terminateFlag\n");
        }
	if ((cowCounter = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0) {
		printf("!!INIT!!INIT!!INIT!!  shm not created for cowCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm created for cowCounter\n");
	}

	if ((sheepCounter = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0) {
		printf("!!INIT!!INIT!!INIT!!  shm not created for sheepCounter\n");
		exit(1);
	}

	else {
		printf("!!INIT!!INIT!!INIT!!  shm created for sheepCounter\n");
	}
	if ((mealWaitingFlag = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0) {
		printf("!!INIT!!INIT!!INIT!!  shm not created for mealWaitingFlag\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm created for mealWaitingFlag\n");
	}
	if ((cowsEatenCounter = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0) {
		printf("!!INIT!!INIT!!INIT!!  shm not created for cowsEatenCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm created for cowsEatenCounter\n");
	}
	if ((sheepEatenCounter = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0) {
		printf("!!INIT!!INIT!!INIT!!  shm not created for sheepEatenCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm created for sheepEatenCounter\n");
	}
	if ((thievesWaitingCounter = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0) {
		printf("!!INIT!!INIT!!INIT!!  shm not created for thievesWaitingCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm created for thievesWaitingCounter\n");
	}

	if ((huntersWaitingCounter = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) < 0) {
		printf("!!INIT!!INIT!!INIT!!  shm not created for huntersWaitingCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm created for huntersWaitingCounter\n");
	}


	/* Now we attach the segment to our data space.  */
        if ((terminateFlagp = shmat(terminateFlag, NULL, 0)) == (int *) -1) {
                printf("!!INIT!!INIT!!INIT!!  shm not attached for terminateFlag\n");
                exit(1);
        }
        else {
                 printf("!!INIT!!INIT!!INIT!!  shm attached for terminateFlag\n");
        }

	if ((cowCounterp = shmat(cowCounter, NULL, 0)) == (int *) -1) {
		printf("!!INIT!!INIT!!INIT!!  shm not attached for cowCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm attached for cowCounter\n");
	}

	if ((sheepCounterp = shmat(sheepCounter, NULL, 0)) == (int *) -1) {
		printf("!!INIT!!INIT!!INIT!!  shm not attached for sheepCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm attached for sheepCounter\n");
	}
	if ((mealWaitingFlagp = shmat(mealWaitingFlag, NULL, 0)) == (int *) -1) {
		printf("!!INIT!!INIT!!INIT!!  shm not attached for mealWaitingFlag\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm attached for mealWaitingFlag\n");
	}
	if ((cowsEatenCounterp = shmat(cowsEatenCounter, NULL, 0)) == (int *) -1) {
		printf("!!INIT!!INIT!!INIT!!  shm not attached for cowsEatenCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm attached for cowsEatenCounter\n");
	}
	if ((sheepEatenCounterp = shmat(sheepEatenCounter, NULL, 0)) == (int *) -1) {
		printf("!!INIT!!INIT!!INIT!!  shm not attached for sheepEatenCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm attached for sheepEatenCounter\n");
	}

	if ((thievesWaitingCounterp= shmat(thievesWaitingCounter, NULL, 0)) == (int *) -1) {
		printf("!!INIT!!INIT!!INIT!!  shm not attached for thievesWaitingCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm attached for thievesWaitingCounter\n");
	}

	if ((huntersWaitingCounterp= shmat(huntersWaitingCounter, NULL, 0)) == (int *) -1) {
		printf("!!INIT!!INIT!!INIT!!  shm not attached for huntersWaitingCounter\n");
		exit(1);
	}
	else {
		printf("!!INIT!!INIT!!INIT!!  shm attached for huntersWaitingCounter\n");
	}



	printf("!!INIT!!INIT!!INIT!!   initialize end\n");
}



void cow(int startTimeN)
{
	int localpid;
	int retval;
	int k;
	localpid = getpid();

	/* graze */
	printf("CCCCCCC %8d CCCCCCC   A cow is born\n", localpid);
	if( startTimeN > 0) {
		if( usleep( startTimeN) == -1){
			/* exit when usleep interrupted by kill signal */
			if(errno==EINTR)exit(4);
		}	
	}
	printf("CCCCCCC %8d CCCCCCC   cow grazes for %f ms\n", localpid, startTimeN/1000.0);


	/* does this beast complete a group of BEASTS_IN_GROUP ? */
	/* if so wake up the dragon */
	semopChecked(semID, &WaitProtectCowsInGroup, 1);
	semopChecked(semID, &SignalCowsInGroup, 1);
	*cowCounterp = *cowCounterp + 1;
	printf("CCCCCCC %8d CCCCCCC   %d  cows have been enchanted \n", localpid, *cowCounterp );
	if( ( *cowCounterp  >= COWS_IN_GROUP && *sheepCounterp  >= SHEEP_IN_GROUP )) {
		*cowCounterp = *cowCounterp - COWS_IN_GROUP;
		for (k=0; k<COWS_IN_GROUP; k++){
			semopChecked(semID, &WaitCowsInGroup, 1);
		}
		printf("CCCCCCC %8d CCCCCCC   The last cow is waiting\n", localpid);
		semopChecked(semID, &SignalProtectCowsInGroup, 1);
		semopChecked(semID, &WaitProtectMealWaitingFlag, 1);
		*mealWaitingFlagp = *mealWaitingFlagp + 1;
		printf("CCCCCCC %8d CCCCCCC   signal meal flag %d\n", localpid, *mealWaitingFlagp);
		semopChecked(semID, &SignalProtectMealWaitingFlag, 1);
		semopChecked(semID, &SignalDragonSleeping, 1);
		printf("CCCCCCC %8d CCCCCCC   last cow  wakes the dragon \n", localpid);
	}
	else
	{
		semopChecked(semID, &SignalProtectCowsInGroup, 1);
	}

	semopChecked(semID, &WaitCowsWaiting, 1);

	/* have all the beasts in group been eaten? */
	/* if so wake up the dragon */
	semopChecked(semID, &WaitProtectCowsEaten, 1);
	semopChecked(semID, &SignalCowsEaten, 1);
	*cowsEatenCounterp = *cowsEatenCounterp + 1;
	if( ( *cowsEatenCounterp >= COWS_IN_GROUP )) {
		*cowsEatenCounterp = *cowsEatenCounterp - COWS_IN_GROUP;
		for (k=0; k<COWS_IN_GROUP; k++){
       		        semopChecked(semID, &WaitCowsEaten, 1);
		}
		printf("CCCCCCC %8d CCCCCCC   The last cow has been eaten\n", localpid);
		semopChecked(semID, &SignalProtectCowsEaten, 1);
		semopChecked(semID, &SignalDragonEating, 1);
	}
	else
	{
		semopChecked(semID, &SignalProtectCowsEaten, 1);
		printf("CCCCCCC %8d CCCCCCC   A cow is waiting to be eaten\n", localpid);
	}
	semopChecked(semID, &WaitCowsDead, 1);

	printf("CCCCCCC %8d CCCCCCC   cow  dies\n", localpid);
}

void sheep(int startTimeN)
{
	int localpid;
	int retval;
	int k;
	localpid = getpid();

	/* graze */
	printf("SHSHSHS %8d SHSHSHS   A sheep is born\n", localpid);
	if( startTimeN > 0) {
		if( usleep( startTimeN) == -1){
			/* exit when usleep interrupted by kill signal */
			if(errno==EINTR)exit(4);
		}	
	}
	printf("SHSHSHS %8d SHSHSHS   sheep grazes for %f ms\n", localpid, startTimeN/1000.0);


	/* does this beast complete a group of BEASTS_IN_GROUP ? */
	/* if so wake up the dragon */
	semopChecked(semID, &WaitProtectSheepInGroup, 1);
	semopChecked(semID, &SignalSheepInGroup, 1);
	*sheepCounterp = *sheepCounterp + 1;
	printf("SHSHSHS %8d SHSHSHS   %d  sheep have been enchanted \n", localpid, *sheepCounterp );
	if( ( *sheepCounterp  >= SHEEP_IN_GROUP && *cowCounterp  >= COWS_IN_GROUP)) {
		*sheepCounterp = *sheepCounterp - SHEEP_IN_GROUP;
		for (k=0; k<SHEEP_IN_GROUP; k++){
			semopChecked(semID, &WaitSheepInGroup, 1);
		}
		printf("SHSHSHS %8d SHSHSHS   The last sheep is waiting\n", localpid);
		semopChecked(semID, &SignalProtectSheepInGroup, 1);
		semopChecked(semID, &WaitProtectMealWaitingFlag, 1);
		*mealWaitingFlagp = *mealWaitingFlagp + 1;
		printf("SHSHSHS %8d SHSHSHS   signal meal flag %d\n", localpid, *mealWaitingFlagp);
		semopChecked(semID, &SignalProtectMealWaitingFlag, 1);
		semopChecked(semID, &SignalDragonSleeping, 1);
		printf("SHSHSHS %8d SHSHSHS   last sheep  wakes the dragon \n", localpid);
	}
	else
	{
		semopChecked(semID, &SignalProtectSheepInGroup, 1);
	}

	semopChecked(semID, &WaitSheepWaiting, 1);

	/* have all the beasts in group been eaten? */
	/* if so wake up the dragon */
	semopChecked(semID, &WaitProtectSheepEaten, 1);
	semopChecked(semID, &SignalSheepEaten, 1);
	*sheepEatenCounterp = *sheepEatenCounterp + 1;
	if( ( *sheepEatenCounterp >= SHEEP_IN_GROUP)) {
		*sheepEatenCounterp = *sheepEatenCounterp - SHEEP_IN_GROUP;
		for (k=0; k<SHEEP_IN_GROUP; k++){
       		        semopChecked(semID, &WaitSheepEaten, 1);
		}
		printf("SHSHSHS %8d SHSHSHS   The last sheep has been eaten\n", localpid);
		semopChecked(semID, &SignalProtectSheepEaten, 1);
		semopChecked(semID, &SignalDragonEating, 1);
	}
	else
	{
		semopChecked(semID, &SignalProtectSheepEaten, 1);
		printf("SHSHSHS %8d SHSHSHS   A sheep is waiting to be eaten\n", localpid);
	}
	semopChecked(semID, &WaitSheepDead, 1);

	printf("SHSHSHS %8d SHSHSHS   sheep  dies\n", localpid);
}


void thief(int startTimeN)
{
	int localpid;
	int retval;
	int k;
	localpid = getpid();

	/* travelling to valley */
	printf("TTTTTTT %8d TTTTTTT   A thief is created\n", localpid);
	if( startTimeN > 0) {
		if( usleep( startTimeN) == -1){
			/* exit when usleep interrupted by kill signal */
			if(errno==EINTR)exit(4);
		}	
	}
	printf("TTTTTTT %8d TTTTTTT   thief travels for %f ms\n", localpid, startTimeN/1000.0);

	semopChecked(semID, &WaitProtectThievesWaiting, 1);

	printf("TTTTTTT %8d TTTTTTT   A thief is waiting to play with Smaug\n", localpid);

	*thievesWaitingCounterp = *thievesWaitingCounterp + 1;

	semopChecked(semID, &SignalProtectThievesWaiting, 1);

	printf("TTTTTTT %8d TTTTTTT   thief wakes up Smaug\n", localpid);
	semopChecked(semID, &SignalDragonSleeping, 1);			//problems

	semopChecked(semID, &WaitThievesWaiting, 1);
	printf("TTTTTTT %8d TTTTTTT   Smaug is ready to play with thief\n", localpid);

	
	semopChecked(semID, &SignalDragonPlaying, 1);
	printf("TTTTTTT %8d TTTTTTT   thief is playing with Smaug\n", localpid);
	
	semopChecked(semID, &WaitThievesPlayed, 1);
	printf("TTTTTTT %8d TTTTTTT   thief is done playing\n", localpid);
	printf("TTTTTTT %8d TTTTTTT   thief leaves\n", localpid);

}


void hunter(int startTimeN)
{
	int localpid;
	int retval;
	int k;
	localpid = getpid();

	/* travelling to valley */
	printf("HHHHHHH %8d HHHHHHH   A hunter is created\n", localpid);
	if( startTimeN > 0) {
		if( usleep( startTimeN) == -1){
			/* exit when usleep interrupted by kill signal */
			if(errno==EINTR)exit(4);
		}	
	}
	printf("HHHHHHH %8d HHHHHHH   hunter travels for %f ms\n", localpid, startTimeN/1000.0);

	semopChecked(semID, &WaitProtectHuntersWaiting, 1);

	printf("HHHHHHH %8d HHHHHHH   A hunter is waiting to fight with Smaug\n", localpid);

	*huntersWaitingCounterp = *huntersWaitingCounterp + 1;

	semopChecked(semID, &SignalProtectHuntersWaiting, 1);

	printf("HHHHHHH %8d HHHHHHH   hunter wakes up Smaug\n", localpid);

	semopChecked(semID, &SignalDragonSleeping, 1);

	semopChecked(semID, &WaitHuntersWaiting, 1);
	printf("HHHHHHH %8d HHHHHHH   Smaug is ready to fight with hunter\n", localpid);


	semopChecked(semID, &SignalDragonFighting, 1);
	printf("HHHHHHH %8d HHHHHHH   hunter is fighting with Smaug\n", localpid);

	semopChecked(semID, &WaitHuntersFought, 1);
	printf("HHHHHHH %8d HHHHHHH   hunter is done fighting\n", localpid);
	printf("HHHHHHH %8d HHHHHHH   hunter leaves\n", localpid);
}


void terminateSimulation() {
	pid_t localpgid;
	pid_t localpid;
	int w = 0;
	int status;

	localpid = getpid();
	printf("RELEASESEMAPHORES   Terminating Simulation from process %8d\n", localpgid);
	if(cowProcessGID != (int)localpgid ){
		if(killpg(cowProcessGID, SIGKILL) == -1 && errno == EPERM) {
			printf("XXTERMINATETERMINATE   COWS NOT KILLED\n");
		}
		printf("XXTERMINATETERMINATE   killed cows \n");
	}
	if(sheepProcessGID != (int)localpgid ){
		if(killpg(sheepProcessGID, SIGKILL) == -1 && errno == EPERM) {
			printf("XXTERMINATETERMINATE   SHEEP NOT KILLED\n");
		}
		printf("XXTERMINATETERMINATE   killed sheep \n");
	}

//added here

	if(thiefProcessGID != (int)localpgid ){
		if(killpg(thiefProcessGID, SIGKILL) == -1 && errno == EPERM) {
			printf("XXTERMINATETERMINATE   THIEF NOT KILLED\n");
		}
		printf("XXTERMINATETERMINATE   killed thief \n");
	}

	if(hunterProcessGID != (int)localpgid ){
		if(killpg(hunterProcessGID, SIGKILL) == -1 && errno == EPERM) {
			printf("XXTERMINATETERMINATE   HUNTER NOT KILLED\n");
		}
		printf("XXTERMINATETERMINATE   killed hunter \n");
	}

	if(smaugProcessID != (int)localpgid ) {
		kill(smaugProcessID, SIGKILL);
		printf("XXTERMINATETERMINATE   killed smaug\n");
	}

	while( (w = waitpid( -1, &status, WNOHANG)) > 1){
			printf("                           REAPED process in terminate %d\n", w);
	}

	releaseSemandMem();
	printf("GOODBYE from terminate\n");
}

void releaseSemandMem() 
{
	pid_t localpid;
	int w = 0;
	int status;

	localpid = getpid();

	//should check return values for clean termination
	semctl(semID, 0, IPC_RMID, seminfo);


	// wait for the semaphores 
	usleep(2000);
	while( (w = waitpid( -1, &status, WNOHANG)) > 1){
			printf("                           REAPED process in terminate %d\n", w);
	}
	printf("\n");
        if(shmdt(terminateFlagp)==-1) {
                printf("RELEASERELEASERELEAS   terminateFlag share memory detach failed\n");
        }
        else{
                printf("RELEASERELEASERELEAS   terminateFlag share memory detached\n");
        }
        if( shmctl(terminateFlag, IPC_RMID, NULL ))
        {
                printf("RELEASERELEASERELEAS   share memory delete failed %d\n",*terminateFlagp );
        }
        else{
                printf("RELEASERELEASERELEAS   share memory deleted\n");
        }
	if( shmdt(cowCounterp)==-1)
	{
		printf("RELEASERELEASERELEAS   cowCounterp memory detach failed\n");
	}
	else{
		printf("RELEASERELEASERELEAS   cowCounterp memory detached\n");
	}

	if( shmdt(sheepCounterp)==-1)
	{
		printf("RELEASERELEASERELEAS   sheepCounterp memory detach failed\n");
	}
	else{
		printf("RELEASERELEASERELEAS   sheepCounterp memory detached\n");
	}
	if( shmctl(cowCounter, IPC_RMID, NULL ))
	{
		printf("RELEASERELEASERELEAS   cowCounter memory delete failed \n");
	}
	else{
		printf("RELEASERELEASERELEAS   cowCounter memory deleted\n");
	}
	if( shmctl(sheepCounter, IPC_RMID, NULL ))
	{
		printf("RELEASERELEASERELEAS   sheepCounter memory delete failed \n");
	}
	else{
		printf("RELEASERELEASERELEAS   sheep Counter memory deleted\n");
	}
	if( shmdt(mealWaitingFlagp)==-1)
	{
		printf("RELEASERELEASERELEAS   mealWaitingFlagp memory detach failed\n");
	}
	else{
		printf("RELEASERELEASERELEAS   mealWaitingFlagp memory detached\n");
	}
	if( shmctl(mealWaitingFlag, IPC_RMID, NULL ))
	{
		printf("RELEASERELEASERELEAS   mealWaitingFlag share memory delete failed \n");
	}
	else{
		printf("RELEASERELEASERELEAS   mealWaitingFlag share memory deleted\n");
	}
	if( shmdt(cowsEatenCounterp)==-1)
	{
		printf("RELEASERELEASERELEAS   cowsEatenCounterp memory detach failed\n");
	}
	else{
		printf("RELEASERELEASERELEAS   cowsEatenCounterp memory detached\n");
	}
	if( shmctl(cowsEatenCounter, IPC_RMID, NULL ))
	{
		printf("RELEASERELEASERELEAS   cowsEatenCounter memory delete failed \n");
	}
	else{
		printf("RELEASERELEASERELEAS   cowsEatenCounter memory deleted\n");
	}

	if( shmdt(sheepEatenCounterp)==-1)
	{
		printf("RELEASERELEASERELEAS   sheepEatenCounterp memory detach failed\n");
	}
	else{
		printf("RELEASERELEASERELEAS   sheepEatenCounterp memory detached\n");
	}
	if( shmctl(sheepEatenCounter, IPC_RMID, NULL ))
	{
		printf("RELEASERELEASERELEAS   sheepEatenCounter memory delete failed \n");
	}
	else{
		printf("RELEASERELEASERELEAS   sheepEatenCounter memory deleted\n");
	}

	if( shmdt(thievesWaitingCounterp)==-1)
	{
		printf("RELEASERELEASERELEAS   thievesWaitingCounterp memory detach failed\n");
	}
	else{
		printf("RELEASERELEASERELEAS   thievesWaitingCounterp memory detached\n");
	}
	if( shmctl(thievesWaitingCounter, IPC_RMID, NULL ))
	{
		printf("RELEASERELEASERELEAS   thievesWaitingCounter memory delete failed \n");
	}
	else{
		printf("RELEASERELEASERELEAS   thievesWaitingCounter memory deleted\n");
	}

	if( shmdt(huntersWaitingCounterp)==-1)
	{
		printf("RELEASERELEASERELEAS   huntersWaitingCounterp memory detach failed\n");
	}
	else{
		printf("RELEASERELEASERELEAS   huntersWaitingCounterp memory detached\n");
	}
	if( shmctl(huntersWaitingCounter, IPC_RMID, NULL ))
	{
		printf("RELEASERELEASERELEAS   huntersWaitingCounter memory delete failed \n");
	}
	else{
		printf("RELEASERELEASERELEAS   huntersWaitingCounter memory deleted\n");
	}



}

void semctlChecked(int semaphoreID, int semNum, int flag, union semun seminfo) { 
	/* wrapper that checks if the semaphore control request has terminated */
	/* successfully. If it has not the entire simulation is terminated */

	if (semctl(semaphoreID, semNum, flag,  seminfo) == -1 ) {
		if(errno != EIDRM) {
			printf("semaphore control failed: simulation terminating\n");
			printf("errno %8d \n",errno );
			*terminateFlagp = 1;
			releaseSemandMem();
			exit(2);
		}
		else {
			exit(3);
		}
	}
}

void semopChecked(int semaphoreID, struct sembuf *operation, unsigned something) 
{

	/* wrapper that checks if the semaphore operation request has terminated */
	/* successfully. If it has not the entire simulation is terminated */
	if (semop(semaphoreID, operation, something) == -1 ) {
		if(errno != EIDRM) {
			printf("semaphore operation failed: simulation terminating\n");
			*terminateFlagp = 1;
			releaseSemandMem();
			exit(2);
		}
		else {
			exit(3);
		}
	}
}


double timeChange( const struct timeval startTime )
{
	struct timeval nowTime;
	double elapsedTime;

	gettimeofday(&nowTime,NULL);
	elapsedTime = (nowTime.tv_sec - startTime.tv_sec)*1000.0;
	elapsedTime +=  (nowTime.tv_usec - startTime.tv_usec)/1000.0;
	return elapsedTime;

}

