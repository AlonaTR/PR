
#include <mpi.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#include "queue.hpp"

extern int shower_stand_num;    /* liczba stanowisk */
extern int num_otaku;   /* liczba otaku */
extern int M;   /* dopuszczalna liczba cuchów w pomieszczeniu */
extern int X;   /* max cuchów do wymiany przewodniczącego */
extern int my_cuchy;    /* moja wartość cuchów */
extern int current_x;   /* aktualna ilosc cuchow*/
extern int ACK_got;    /* liczba ACk otrzymana od innych procesów */

extern bool ubiegam_sie; /* czy chcę dostępu do kolejki */
extern bool wyzerowanie_kolejki;    /* czy opróżnienić kolejke */
extern bool counted_X; /* czy limit cuchów uaktualniony */

extern int ptn_num_w_kolejce_policzony;    /* wskaźnik-pozycja na której zokończono liczenie cuchó */

extern struct Queue *queue;

typedef enum {InLobby, InQueue, InRoom} state_t;
/* programme part */
extern state_t stan;
extern int size_comm;
extern int rank_comm;
extern int timer;
extern MPI_Datatype MPI_PAKIET_T;
extern pthread_t threadKom;

extern pthread_mutex_t stateMut;
extern pthread_mutex_t timerMut;
extern pthread_mutex_t roomMut;
extern pthread_mutex_t leaveRoomMut;


typedef struct {
    int timestamp;       //timestamp (zegar lamporta 
    int src_id;      // pole nie przesyłane, ale ustawiane w main_loop 
    int cuchy;      // wartość własna cuchów
} packet_t;
extern MPI_Datatype MPI_PAKIET_T;



void change_state(state_t);
void send_packet(packet_t *packet, int destination, int tag);
void init_MPI(int argc, char** argv);
void finalize();
void init_program_vars(int argc, char** argv);
