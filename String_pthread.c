
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define NUM_THREADS	4
#define MAX 1024

void *sub_string(void *);
int readf(FILE *fp);
int total=0;
int n1,n2;
char *s1,*s2;
FILE *fp;
pthread_mutex_t total_lock;

int main(int argc, char *argv[])
{
	int i,rc;
	pthread_t threads[NUM_THREADS];

	pthread_mutex_init(&total_lock,NULL);
	readf(fp);
	for(i=0;i<NUM_THREADS;i++){
		rc=pthread_create(&threads[i],NULL,sub_string,(void *)(intptr_t ) i);
		if (rc){
			printf("ERROR: return error from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for(i=0; i<NUM_THREADS; i++){
		rc = pthread_join(threads[i], NULL);
		if (rc){
			printf("ERROR: return error from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}
	printf("the occurences of s2 in s1 is %d\n",total);


	pthread_mutex_destroy(&total_lock);
	if(s1) free(s1); 
	if(s2) free(s2); 
	if(fp) fclose(fp); 
	pthread_exit(0); 
}



int readf(FILE *fp)
{
	if((fp=fopen("string.txt", "r"))==NULL){
		printf("ERROR: can't open string.txt!\n");
		return 0;
	}
	s1=(char *)malloc(sizeof(char)*MAX);
	if(s1==NULL){
		printf("ERROR: Out of memory!\n");
		return -1;
	}
	s2=(char *)malloc(sizeof(char)*MAX);
	if(s2==NULL){
		printf("ERROR: Out of memory\n");
		return -1;
	}
	/*read s1 s2 from the file*/
	s1=fgets(s1, MAX, fp);
	s2=fgets(s2, MAX, fp);
	char *newline = strchr(s1, '\n');
	if(newline) *newline = '\0';
	newline = strchr(s2, '\n'); 
	if(newline) *newline = '\0';

	n1=strlen(s1);  /*length of s1*/
	n2=strlen(s2); /*length of s2*/

	if(s1==NULL || s2==NULL ||n1<n2)  /*when error exit*/
		return -1;
}

void *sub_string(void *threadid) 	/*each process searches in the string with the step of nprocs until it reach or beyond*/ 
	/*the (n1-n2)th char which is the last possible beginning of the substring*/
{
	int tid = (int)(intptr_t)threadid; 
	int start_idx, end_idx; 
	int i, j; 
	int segment_size;
	int local_total = 0; 


	//calculate segment size
	segment_size = n1/NUM_THREADS; 

	//calculate start index 
	start_idx = tid * segment_size;

	// Last thread gets the remaining portion including the last possible substring start
	
	if(tid == NUM_THREADS - 1){

		end_idx = n1 - n2;
	}else{
		end_idx = start_idx + segment_size - 1;  
	}

	if(end_idx > n1 - n2){
		end_idx = n1 - n2;
	}
	
	if(start_idx < 0 || start_idx > n1 - n2){
		printf("Thread %d: start_idx %d is out of range (max : %d), skipping", tid, start_idx, n1 - n2);
		pthread_exit(NULL);
	}

	if(end_idx < start_idx){
		printf("Thread %d: no valid range to search (start = %d, end=%d)\n", tid, start_idx, end_idx); 
		pthread_exit(NULL);
	}

	printf("Thread %d: searching indices %d to %d (total positions: %d)\n", tid, start_idx, end_idx, end_idx - start_idx + 1);

	// search for substrings within the assigned ranged
	for(i = start_idx; i <= end_idx; i++){
		int match = 1;
		for(j = 0; j < n2; j++){
			if(s1[i + j] != s2[j]){
				match = 0;
				break;
			}
		}
		if(match){
			local_total++; 
			printf("Thread %d: found match at postion %d\n", tid, i); 
		}
	}
	
	if(local_total >0){
		pthread_mutex_lock(&total_lock);
		total += local_total;
		pthread_mutex_unlock(&total_lock);

	}
	printf("Thread %d found %d matches, running total: %d \n", tid, local_total,total ); 
	pthread_exit(NULL);

}







