
#include <mpi.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#include "queue.hpp"

#ifndef GLOBALH
#define GLOBALH
#define _GNU_SOURCE

/* używane w wątku głównym,
 determinuje jak często i na jak długo zmieniają się stany */
#define STATE_CHANGE_PROB 50
#define MAX_SEC_IN_LOBBY 1
#define MAX_SEC_IN_ROOM 1
#define MAX_CUCH_INCREASE 1

#define ROOT 0

typedef enum {InLobby, InQueue, InRoom, InFinish} state_t;

extern int shower_stand_num;
extern int num_otaku;
extern int M;
extern int X;
extern int my_cuchy;
extern int current_x;
extern int ACK_got;

extern bool ubiegam_sie;
extern bool wyzerowanie_kolejki;
extern bool policono_X;
extern int ptn_num_w_kolejce_policzony;

extern struct Queue *queue;

/* programme part */
extern state_t stan;
extern int size_comm;
extern int rank_comm;
extern int timer;
extern MPI_Datatype MPI_PAKIET_T;
extern pthread_t threadKom;
extern pthread_t threadMon;

extern pthread_mutex_t stateMut;
extern pthread_mutex_t timerMut;
extern pthread_mutex_t roomMut;
extern pthread_mutex_t leaveRoomMut;

/* stan globalny wykryty przez monitor */
extern int globalState;

#define FIELDNO 3
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src_id;      /* pole nie przesyłane, ale ustawiane w main_loop */
    int cuchy;      /* wartość własna cuchów*/
} packet_t;
extern MPI_Datatype MPI_PAKIET_T;

/* Typy wiadomości */
#define FINISH 1
#define REQUEST 2
#define RELEASE 3
#define ACK 4

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
					   "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape. 
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów
                                            
*/
#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d]: %d " FORMAT "%c[%d;%dm\n",  27, (1+(rank_comm/7))%2, 31+(6+rank_comm)%7, rank_comm, timer, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#define P_WHITE printf("%c[%d;%dm",27,1,37);
#define P_BLACK printf("%c[%d;%dm",27,1,30);
#define P_RED printf("%c[%d;%dm",27,1,31);
#define P_GREEN printf("%c[%d;%dm",27,1,33);
#define P_BLUE printf("%c[%d;%dm",27,1,34);
#define P_MAGENTA printf("%c[%d;%dm",27,1,35);
#define P_CYAN printf("%c[%d;%d;%dm",27,1,36);
#define P_SET(X) printf("%c[%d;%dm",27,1,31+(6+X)%7);
#define P_CLR printf("%c[%d;%dm",27,0,37);

/* printf ale z kolorkami i automatycznym wyświetlaniem RANK. Patrz debug wyżej po szczegóły, jak działa ustawianie kolorków */
#define println(FORMAT, ...) printf("%c[%d;%dm [%d]: %d " FORMAT "%c[%d;%dm\n",  27, (1+(rank_comm/7))%2, 31+(6+rank_comm)%7, rank_comm, timer, ##__VA_ARGS__, 27,0,37);

// void sendPacket(packet_t *pkt, int destination, int tag);
void changeState( state_t );
void sendPacket(packet_t *pkt, int destination, int tag);
#endif

void init_MPI(int argc, char** argv);
void finalize();
void init_program_vars(int argc, char** argv);
