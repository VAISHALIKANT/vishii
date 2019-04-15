#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/types.h>
#define random rand
#define srandom srand
#ifndef MIN
#define MIN(_x,_y) ((_x)<(_y)) ? (_x):(_y)
#endif
struct station
{
	pthread_mutex_t tpLock;
    pthread_cond_t trainArrived;
    pthread_cond_t passengerSettled;
    int boarded_passengers; 
	int passengers_inStation; 
	int seats_vacant;
};

void station_init(struct station *station);
void station_load_train(struct station *station, int count);
void station_wait_for_train(struct station *station);
void station_on_board(struct station *station);

void station_init(struct station *station)
{
	pthread_mutex_init(&station->tpLock , NULL); 
	pthread_cond_init(&station->trainArrived ,NULL); 
	pthread_cond_init(&station->passengerSettled,NULL);
	station->boarded_passengers=0;
    station->passengers_inStation = 0;
	station->seats_vacant = 0;

}
void station_load_train(struct station *station, int count)
{


    pthread_mutex_lock(&station->tpLock);
    station->seats_vacant = count;
    while(station->seats_vacant > 0 && station->passengers_inStation > 0)
	{
        pthread_cond_broadcast(&station->trainArrived);
        pthread_cond_wait(&station->passengerSettled , &station->tpLock);
	}

    station->seats_vacant = 0;
	pthread_mutex_unlock(&station->tpLock);
}




void
station_wait_for_train(struct station *station)
{
    
    pthread_mutex_lock(&station->tpLock);
	station->passengers_inStation++;

	while(station->boarded_passengers == station->seats_vacant)
	{
        pthread_cond_wait(&station->trainArrived , &station->tpLock);
	}

	station->boarded_passengers++;
	station->passengers_inStation--;
	pthread_mutex_unlock(&station->tpLock);
}





void station_on_board(struct station *station)
{
    
	pthread_mutex_lock(&station->tpLock);
	station->boarded_passengers--;
	station->seats_vacant--;

	if((station->seats_vacant == 0) || (station->boarded_passengers == 0))
	{
        pthread_cond_signal(&station->passengerSettled);
	}

	pthread_mutex_unlock(&station->tpLock);

}

volatile int threads_completed = 0;
void* passenger_thread(void *arg)
{
	struct station *station = (struct station*)arg;
	station_wait_for_train(station);
        threads_completed++;
	return NULL;
}

struct TrainLoaded_Para 
{
	struct station *station;
	int free_seats;
};

volatile int return_LoadTrain = 0;

void* load_train_thread(void *args)
{
	struct TrainLoaded_Para *temp = (struct TrainLoaded_Para*)args;
	station_load_train(temp->station, temp->free_seats);
	return_LoadTrain = 1;
	return NULL;
}

int main()
{
	struct station station;
	station_init(&station);
	printf("\nIndian Railways is Booting.\n");
	int i,num;
	for(i =0 ; i<4;i++)
	{
		printf("..\n");	
		sleep(1);
	}
	printf("\n\n");
	printf("\n-----------------------------------------------------------------------WELCOME TO INDIAN RAILWAYS-----------------------------------------------------------------------\n\n");
	printf("\n  Train will be arriving soon. ");
	printf("\n  Be Seated. ");
	while(1)
	{
	printf("\n  To leave the Station at any time, press CTRL + C.");
	printf("\n  Enter the number of Passengers at the Station: ");
    scanf("%d",&num);
	if(num<0)
	{
		printf("\n  You have entered number of passengers as %d which is not possible.",num);	
		printf("\n  Please enter a valid number."); 
		scanf("%d",&num);
	}
	if(num==0)
	{
		printf("\n  No Passengers are at the Station. ");
		return 0;
	}	
	const int total_Passngrs=num;
	int remaining_Passngrs = total_Passngrs;
	for (i = 0; i < total_Passngrs; i++) 
	{
		pthread_t tid;
 		int ret = pthread_create(&tid, NULL, passenger_thread, &station);
	}
	
    int total_Passngrs_boarded = 0;
	const int tot_FreeSeats_PerTrain =100;
	int pass = 0;
    int j=1,p=1;
	while (remaining_Passngrs > 0) 
	{

		int free_seats = random() % tot_FreeSeats_PerTrain;
        printf("\n  Train %d has arrived at the station, Free Seats Available: %d ",j,free_seats);
        j++;
		return_LoadTrain = 0;
		struct TrainLoaded_Para args = { &station, free_seats };
		pthread_t lt_tid;
		int ret = pthread_create(&lt_tid, NULL, load_train_thread, &args);
		if (ret != 0) 
		{
			perror("pthread_create");
			//exit(1);
		}
                 
		int threads_to_reap = MIN(remaining_Passngrs, free_seats);
		int threads_reaped = 0;

		while (threads_reaped < threads_to_reap)
		{
			if(return_LoadTrain)
			{
			//exit(1);
			}
			if (threads_completed > 0) 
			{
				if ((pass % 2) == 0)
					usleep(random() % 2);
				threads_reaped++;
				station_on_board(&station);
                threads_completed++;
		
			}
		}

		remaining_Passngrs -= threads_reaped;
		total_Passngrs_boarded += threads_reaped;
        printf("\n  Train %d has Departed from the Station with Number of Passengers: %d",p,threads_to_reap);
		printf("\n  Number of Passengers left on the station: %d \n",remaining_Passngrs);
		pass++; 
        p++;
	}

	if (total_Passngrs_boarded == total_Passngrs) 
	{
		printf("\n  ALL PASSENGERS BOARDED!\n\n");
		
	
	}
}
}git remote add origin https://github.com/VAISHALIKANT/vishii.git
git push -u origin master
	
