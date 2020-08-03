#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>

// GitHub name: ericcorbu
// GitHub repo: https://github.com/ericcorbu/cp386-assignment4


void logStart(char* tID);//function to log that a new thread is started
void logFinish(char* tID);//function to log that a thread has finished its time

void startClock();//function to start program clock
long getCurrentTime();//function to check current time since clock was started
time_t programClock;//the global timer/clock for the program

// semaphores
sem_t even, odd;

// Number of resources
int numResources;
int customerNumber;
char inputFileName[] = "sample4_in.txt";

char input[100] = "";
// counters to check if there are odd or even threads left
int evenCount = 0, oddCount = 0;
int** maximum = NULL;
int** allocation = NULL;
int** need = NULL;
int* available = NULL;



typedef struct thread //represents a single thread, you can add more members if required
{
	char tid[4];//id of the thread as read from file
	unsigned int startTime;
	int state;
	pthread_t handle;
	int retVal;
	int even;
} Thread;

//you can add more functions here if required

int threadsLeft(Thread* threads, int threadCount);
int threadToStart(Thread* threads, int threadCount);
void* threadRun(void* t);//the thread function, the code executed by each thread
int readFile(char* fileName);//function to read the file content and build array of threads

int RQ(int* input);
void RL(int* input);
void asterisk();

int main(int argc, char *argv[])
{
	if(argc<2)
	{
		printf("Input file name missing...exiting with error code -1\n");
		return -1;
	}

    //you can add some suitable code anywhere in main() if required

	//Thread* threads = NULL;
	
	numResources = argc -1;
	customerNumber = readFile(inputFileName);
	printf("Number of customers: %d\n",customerNumber);

	available = (int*) malloc(sizeof(int)*(argc-1));
	for (int i=0; i<argc-1; i++){
		available[i] = atoi(argv[i+1]);
	}
	printf("Currently available resources: ");
	for (int i =0; i<argc-1; i++) {
		printf("%d ",available[i]);

	}
	printf("\nMaximum resources from file:\n");

	for (int i = 0; i<customerNumber; i++){

		for (int j=0; j<numResources; j++){
			printf("%d, ",maximum[i][j]);
		}
		printf("\n");
	}

	char* inToken = NULL;
	while (1){
		printf("Enter command: ");
		fgets(input, 100, stdin);
		inToken = strtok(input, " ");
		int* arguments = (int *)malloc(numResources +1  * sizeof(int));
		if (strcmp(inToken,"RQ") == 0) {
			int i =0;
			while (inToken != NULL){
				arguments[i] = atoi(inToken);
				i++;
				inToken = strtok(NULL, " ");
			}
			int result = RQ(arguments);
			if (result == 1 ){
				printf("Request is satisfied.\n");
			}
			else {
				printf("Not enough free resources.\n");
			}
		}
		else if (strcmp(inToken,"RL") == 0){
			printf("RL Command");
			int i =0;
			while (inToken != NULL){
				arguments[i] = atoi(inToken);
				i++;
				inToken = strtok(NULL, " ");
			}
			RL(arguments);
		}
		else if (strcmp(inToken, "*\n") == 0){
			printf("Printing state of arrays.\n");
			asterisk();
		}
		else {
			printf("Invalid command");
		}
		
	}
	


	return 0;
}

int readFile(char* fileName)//do not modify this method
{
	FILE *in = fopen(fileName, "r");
	if(!in)
	{
		printf("Child A: Error in opening input file...exiting with error code -1\n");
		return -1;
	}

	struct stat st;
	fstat(fileno(in), &st);
	char* fileContent = (char*)malloc(((int)st.st_size+1)* sizeof(char));
	fileContent[0]='\0';	
	while(!feof(in))
	{
		char line[100];
		if(fgets(line,100,in)!=NULL)
		{
			strncat(fileContent,line,strlen(line));
		}
	}
	fclose(in);

	char* command = NULL;
	int customerCount = 0;
	char* fileCopy = (char*)malloc((strlen(fileContent)+1)*sizeof(char));
	strcpy(fileCopy,fileContent);
	command = strtok(fileCopy,"\r\n");
	while(command!=NULL)
	{
		customerCount++;
		command = strtok(NULL,"\r\n");
	}
	maximum = (int**) malloc(sizeof(int*)*customerCount);
	need = (int**) malloc(sizeof(int*)*customerCount);
	allocation = (int**) malloc(sizeof(int*)*customerCount);


	char* lines[customerCount];
	command = NULL;
	int i=0;
	command = strtok(fileContent,"\r\n");
	while(command!=NULL)
	{
		lines[i] = malloc(sizeof(command)*sizeof(char));
		strcpy(lines[i],command);
		i++;
		command = strtok(NULL,"\r\n");
	}

	for(int k=0; k<customerCount; k++)
	{
		char* token = NULL;
		int j = 0;
		//printf(lines[k]);
		token =  strtok(lines[k],",");
		
		//printf("Line: %s\n", lines[k]);
		maximum[k] = (int *)malloc(numResources * sizeof(int));
		need[k] = (int *)malloc(numResources * sizeof(int));
		allocation[k] = (int *)malloc(numResources * sizeof(int));
		while(token!=NULL)
		{
//if you have extended the Thread struct then here
//you can do initialization of those additional members
//or any other action on the Thread members
			//printf("Token: %d\n", atoi(token));
			maximum[k][j]=atoi(token);
			need[k][j] = atoi(token);
			allocation[k][j]=0;
			//printf("%d", maximum[k][j]);
			j++;
			token = strtok(NULL,",");
		}
	}
	return customerCount;
}

int RQ(int* input){
	int customerNum = input[0];
	int safe = 0;
	int valid = 0;
	int sufficient = 0;
	for (int i=1; i< numResources+1; i++){
		printf("Request: %d \n", input[i]);
		printf("Need: %d\n", need[customerNum][i-1]);
		if (input[i] > need[customerNum][i-1]){
			valid = -1;
			printf("Requested more resources then needed, request invalid.\n");
		}
		if (input[i] > available[i-1]){
			sufficient = -1;
			printf("Insufficient available resources.\n");
		}
		available[i-1] = available[i-1] - input[i];
		allocation[customerNum][i-1] = allocation[customerNum][i-1] + input[i];
		need[customerNum][i-1] = need[customerNum][i-1] - input[i];

	}

	if (valid == -1 || sufficient == -1){
		RL(input);
	}
	else {
		printf("Safe.\n");
	}

	return safe;

}
void RL(int* input) {
	int customerNum = input[0];
	for (int i=1; i<numResources+1; i++){
		available[i-1] = available[i-1] + input[i];
		allocation[customerNum][i-1] = allocation[customerNum][i-1] - input[i];
		need[customerNum][i-1] = need[customerNum][i-1] + input[i];
	}
	
}

int* safety() {
	int* work = (int*) malloc(sizeof(int)*(numResources));
	int* finish = (int*) malloc(sizeof(int)*(customerNumber));
	int* order = (int*) malloc(sizeof(int)*customerNumber);
	for (int i=0; i<customerNumber; i++){
		order[i] = -1;
	}
	for (int i=0; i<numResources; i++){
		work[i] = available[i];
	}
	for (int i=0; i<customerNumber; i++){
		finish[i] = 0;
	}
	int i = 0;
	int completedCount = 0;
	while (i<customerNumber){
		if (finish[i] == 0){
			int possible = 1;
			for (int j=0; j<numResources; j++){
				if (need[i][j] > work[j]){
					possible = -1;
				}
			}
			if (possible == 1 ){
				for (int j =0; j<numResources; j++){
					work[j] = work[j] + allocation[i][j];
				}
				finish[i] = 1;
				order[completedCount] = i;
				completedCount++;
				i=0;
			}
			else {
				i++;
			}
		
		}
		else {
			i++;
		}
		if (i==customerNumber){
			if (completedCount != customerNumber){
				return -1;
			}
			else {
				return order;
			}
		}

}
}

void asterisk() {
	printf("\nAvailable: ");
	for (int i=0; i<numResources; i++){
		printf("%d ", available[i]);
	}
	printf("\n\nMaximum:");
	for (int i=0; i<customerNumber; i++){
		printf("\nCustomer/Thread %d: ", i);
		for (int j=0; j<numResources; j++){
			printf("%d ", maximum[i][j]);
		}
	}
	printf("\n\nAllocated:");
	for (int i=0; i<customerNumber; i++){
		printf("\nCustomer/Thread %d: ", i);
		for (int j=0; j<numResources; j++){
			printf("%d ", allocation[i][j]);
		}
	}
	printf("\n\nNeeded:");
	for (int i=0; i<customerNumber; i++){
		printf("\nCustomer/Thread %d: ", i);
		for (int j=0; j<numResources; j++){
			printf("%d ", need[i][j]);
		}
	}
	printf("\n");
}

void logStart(char* tID)
{
	printf("[%ld] New Thread with ID %s is started.\n", getCurrentTime(), tID);
}

void logFinish(char* tID)
{
	printf("[%ld] Thread with ID %s is finished.\n", getCurrentTime(), tID);
}

int threadsLeft(Thread* threads, int threadCount)
{
	int remainingThreads = 0;
	for(int k=0; k<threadCount; k++)
	{
		if(threads[k].state>-1)
			remainingThreads++;
	}
	return remainingThreads;
}

int threadToStart(Thread* threads, int threadCount)
{
	for(int k=0; k<threadCount; k++)
	{
		if(threads[k].state==0 && threads[k].startTime==getCurrentTime())
			return k;
	}
	return -1;
}

void* threadRun(void* t)//implement this function in a suitable way
{
	logStart(((Thread*)t)->tid);
	
//your synchronization logic will appear here
	// if even, wait for even semaphore
	
	//critical section starts here
	printf("[%ld] Thread %s is in its critical section\n",getCurrentTime(), ((Thread*)t)->tid);

	//critical section ends here

//your synchronization logic will appear here

	// signal opposite semaphore

	// for even threads



	logFinish(((Thread*)t)->tid);
	((Thread*)t)->state = -1;
	pthread_exit(0);
}

void startClock()
{
	programClock = time(NULL);
}

long getCurrentTime()//invoke this method whenever you want check how much time units passed
//since you invoked startClock()
{
	time_t now;
	now = time(NULL);
	return now-programClock;
}

