#include "main.hpp"
#include "main_thread.hpp"
#include "const.hpp"


void mainLoop() {
    bool printed = false;
    srand(time(NULL) + rank_comm);
    while (true) {               
        int perc = rand() % 100; 
        // spróbuj wejść z pewnym prawdopodobieństwem
        if (perc < STATE_CHANGE_PROB) {      //STATE_CHANGE_PROB determinuje jak często i na jak długo zmieniają się stany
            switch(stan) {
                case InLobby:
                
                    if (my_cuchy > M) {                         //Jesli cuchy Otaku większe za M, to oznacza że on nigdy nie zmoże wejść do pokoju
                        if(!printed) {
                            printed = true;
                            printf("%d Koniec przetwarzania (M:%d, moje_cuchy: %d) \n",rank_comm, M, my_cuchy);
                            
                        }
                        continue;
                    }
                    pthread_mutex_unlock( &leaveRoomMut );
                    change_state( InQueue );
                    
                    pthread_mutex_lock( &timerMut );
                    printf("%d Zmieniam stan na \"Ubiegam się\" \n", rank_comm);
                    timer++;
                    /* wyślij REQUEST o wejście do pokoju */
                    for (int i=0; i<size_comm; i++){
                        send_packet(0, i, REQUEST);
                    }
                    pthread_mutex_unlock( &timerMut );
                    pthread_mutex_lock( &roomMut );
                    break;
                case InQueue:
                /* czekanie na wejście do pokoju (mutex w wątku komunikacyjnym) */
                    printf("%d Czekam na wejście do pokoju \n", rank_comm);
                    pthread_mutex_lock( &roomMut );       
                    pthread_mutex_lock( &leaveRoomMut );                                                
                    change_state( InRoom );
                    printf("%d Zmieniam stan na \"Jestem w pokoju\" \n", rank_comm);                                  
                    
                    break;
                case InRoom:
                /*jestem w pokoju */
                    sleep(rand()% MAX_SEC_IN_ROOM + 1); //spędza czas w pokoju
                    printf("%d Chcę wyjść z pokoju \n", rank_comm);
                    pthread_mutex_lock(&timerMut);
                    timer++;
                    for (int i=0; i<size_comm; i++) send_packet(0, i, RELEASE);  //wysyłamy komunikat że wychodzimy z pokoju
                    pthread_mutex_unlock(&timerMut);
                    pthread_mutex_unlock(&roomMut);
                    pthread_mutex_lock(&leaveRoomMut);
                    change_state(InLobby);

                    ACK_got = 0;
                    printf("%d Wyszedłem z pokoju \n", rank_comm);
                    printed = false;
                    break;
                default:
                    break;
            }
        } 

        if (stan == InLobby) {
            sleep(random() % MAX_SEC_IN_LOBBY + 1);
        } 
        else if (stan == InRoom) {
            sleep(random() % MAX_SEC_IN_ROOM + 1);
        }
    }

}