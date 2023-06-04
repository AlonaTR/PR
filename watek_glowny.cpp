#include "main.hpp"
#include "watek_glowny.hpp"


void mainLoop() {
    bool printed = false;
    // srand(time(NULL) + rank_comm);
    while (stan != InFinish) {
        int perc = random()%100; 
        // try to enter with some probability
        if (perc < STATE_CHANGE_PROB) {
            switch(stan) {
                case InLobby:
                // wyślij REQUEST o wejście do pokoju
                    if (my_cuchy > M) {
                        if(!printed) {
                            printed = true;
                            debug("Koniec przetwarzania (M:%d, moje_cuchy: %d)", M, my_cuchy);
                        }
                        continue;
                    }
                    pthread_mutex_unlock( &leaveRoomMut );
                    change_state( InQueue );
                    
                    pthread_mutex_lock( &timerMut );
                    debug("Zmieniam stan na \"Ubiegam się\"");
                    for (int i=0; i<size_comm; i++){
                        send_packet(0, i, REQUEST);
                    }
                    pthread_mutex_unlock( &timerMut );
                    pthread_mutex_lock( &roomMut );
                    break;
                case InQueue:
                // czekanie na wejście do pokoju (mutex w wątku komunikacyjnym)
                    debug("Czekam na wejście do pokoju");
                    pthread_mutex_lock( &roomMut );
                    pthread_mutex_lock( &leaveRoomMut );
                    change_state( InRoom );
                    debug("Zmieniam stan na \"Jestem w pokoju\"");
                    break;
                case InRoom:
                // jestem w pokoju
                    pthread_mutex_lock(&timerMut);
                    for (int i=0; i<size_comm; i++) send_packet(0, i, RELEASE);
                    pthread_mutex_unlock(&timerMut);
                    pthread_mutex_unlock(&roomMut);
                    pthread_mutex_lock(&leaveRoomMut);
                    change_state(InLobby);
                    debug("Wyszedłem z pokoju");
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