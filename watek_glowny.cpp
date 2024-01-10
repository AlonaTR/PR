#include "main.hpp"
#include "watek_glowny.hpp"


void mainLoop() {
    bool printed = false;
    srand(time(NULL) + rank_comm);
    while (stan != InFinish) {                  //dopóki nie jest w stanie InFinish
        int perc = rand() % 100; 
        // try to enter with some probability
        if (perc < STATE_CHANGE_PROB) {      //STATE_CHANGE_PROB determinuje jak często i na jak długo zmieniają się stany
            switch(stan) {
                case InLobby:
                /* wyślij REQUEST o wejście do pokoju */
                    if (my_cuchy > M) {                         //Jesli cuchy Otaku większe za M, to oznacza że on nigdy nie zmoże wejść do pokoju
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
                    timer++;
                    for (int i=0; i<size_comm; i++){
                        send_packet(0, i, REQUEST);
                    }
                    pthread_mutex_unlock( &timerMut );
                    pthread_mutex_lock( &roomMut );
                    break;
                case InQueue:
                /* czekanie na wejście do pokoju (mutex w wątku komunikacyjnym) */
                    debug("Czekam na wejście do pokoju");
                    pthread_mutex_lock( &roomMut );       
                    pthread_mutex_lock( &leaveRoomMut );                                                //czy można to zabrać?
                    change_state( InRoom );
                    debug("Zmieniam stan na \"Jestem w pokoju\"");                                      //przeniść do stanu pokój??
                    
                    break;
                case InRoom:
                /*jestem w pokoju */
                    sleep(rand()% MAX_SEC_IN_ROOM + 1); //spędza czas w pokoju
                    debug("Chcę wyjść z pokoju");
                    pthread_mutex_lock(&timerMut);
                    timer++;
                    for (int i=0; i<size_comm; i++) send_packet(0, i, RELEASE);  //wysyłamy komunikat że wychodzimy z pokoju
                    pthread_mutex_unlock(&timerMut);
                    pthread_mutex_unlock(&roomMut);
                    pthread_mutex_lock(&leaveRoomMut);
                    change_state(InLobby);

                    // TODO: send ACK to everyone in queue

                    ACK_got = 0;
                    println("Wyszedłem z pokoju");
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