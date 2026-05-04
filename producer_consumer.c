#include <stdlib.h>
#include <string.h> 
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define MAX_MSG 256

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER; 


typedef struct {
	char buffer[BUFFER_SIZE];
	int count; 
	int in;
	int out;
	int done;

}circular_queue_t;

circular_queue_t queue;

char message[MAX_MSG];
int msg_length;


void read_message(){
	FILE *fp; 
	fp = fopen ("message.txt", "r");
	if(fp == NULL){
		printf("ERROR: can't open message.txt\n");
		exit(1);
	}
	fgets(message, MAX_MSG, fp); 
	msg_length = strlen(message);

	if(message[msg_length - 1] == '\n'){
		message[msg_length - 1] = '\0';
		msg_length--;

	}
	
	fclose(fp); 

	printf("Message to process: \"%s\"\n", message);
	printf("Message length: %d characters\n\n", msg_length);
}

void initialize_queue(){

	queue.in = 0; 
	queue.out = 0; 
	queue.count = 0; 
	queue.done = 0; 
}

void enqueue(char item){
	queue.buffer[queue.in] = item;
	queue.in = (queue.in + 1) % BUFFER_SIZE;
	queue.count++;

}
char dequeue(){
	char item = queue.buffer[queue.out];
	queue.out = (queue.out + 1) % BUFFER_SIZE; 
	queue.count--; 
	return item;
}

void *producer(void *arg){
	int i,j;
	printf("Producer thread started\n");

	for(i = 0; i < msg_length; i++) {
		pthread_mutex_lock(&mutex);


	while(queue.count == BUFFER_SIZE){
		printf("Producer: buffer full, waiting...\n");
		pthread_cond_wait(&not_full, &mutex);	
	}

	enqueue(message[i]);
	printf("Producer: added '%c' to buffer[", message[i]); 

	for(j = 0; j < BUFFER_SIZE; j++) { 

		if(j < queue.count){
			printf(" %c", queue.buffer[(queue.out + j) % BUFFER_SIZE]);
		}else{
			printf(" _");
		}
	}

	printf(" ] (count: %d/%d)\n", queue.count, BUFFER_SIZE);

	pthread_cond_signal(&not_empty);
	pthread_mutex_unlock(&mutex);
	usleep(100000);
    }
    pthread_mutex_lock(&mutex);
    queue.done = 1;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
    printf("Producer: Finished producing all characters\n");
    pthread_exit(NULL);

}

void *consumer(void *arg){
	int count_consumed = 0;
	char item;

	printf("Consumer thread started\n");
	while(1){
	pthread_mutex_lock(&mutex);
	while(queue.count == 0 && !queue.done){
		printf("Consumer: buffer empty, waiting...\n");
		pthread_cond_wait(&not_empty, &mutex);

	}
	
	if(queue.count == 0 && queue.done){
		pthread_mutex_unlock(&mutex);
		break;
	}

	item = dequeue();
	count_consumed++;
	printf("Consumer: removed '%c' from buffer [", item);
	
	for(int j = 0; j < BUFFER_SIZE; j++){
		if(j < queue.count){
			printf(" %c", queue.buffer[(queue.out + j) % BUFFER_SIZE]);
		}else{
			printf(" _");
		}
	}
	printf(" ] (count: %d/%d)\n", queue.count, BUFFER_SIZE);
	printf("Consumer: printed '%c'\n", item);

	pthread_cond_signal(&not_full);
	pthread_mutex_unlock(&mutex);

	usleep(150000);

    }
	printf("Consumer: Finished consuming all chars\n");
	printf("Consumer: Total chars consume: %d\n", count_consumed);
	pthread_exit(NULL);
	
}

int main(){
	pthread_t producer_thread, consumer_thread;
	
	printf("Producer-Consumer problem");
	printf("Buffer Size: %d chars\n", BUFFER_SIZE);

	read_message();

	if (msg_length == 0){
	printf("No message to process\n");
	return 0;
	}
	initialize_queue();

	pthread_create(&producer_thread, NULL, producer, NULL);
	pthread_create(&consumer_thread, NULL, consumer, NULL);

	pthread_join(producer_thread, NULL);
	pthread_join(consumer_thread, NULL);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&not_full);
	pthread_cond_destroy(&not_empty);

	printf("program completed succesfully!\n");

	return 0;

}
