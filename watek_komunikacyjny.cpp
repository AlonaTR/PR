#include "main.hpp"
#include "watek_komunikacyjny.hpp"
#include "queue.hpp"
#include "const.hpp"


void *startKomWatek(void *ptr) {
    MPI_Status status;
    bool is_message = false;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	    printf("%d czekam na recv \n", rank_comm);
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&timerMut);
        timer = std::max(timer, pakiet.timestamp) + 1;
        pthread_mutex_unlock(&timerMut);

        switch ( status.MPI_TAG ) {
            case FINISH: {
                change_state(InFinish);
                break;
                }
            case ACK: {
                printf("%d Dostałem ACK od %d z zegarem:%d \n",rank_comm, pakiet.src_id, pakiet.timestamp);
                ACK_got++;
                if (ACK_got == num_otaku) try_to_enter();                //kiedy otrzymałam wszystkie ACK wtedy probuje wejść
                break;
                }
            case REQUEST: {
                printf("%d Dostałem REQUEST od %d z zegarem:%d i cuchami:%d \n",rank_comm, pakiet.src_id, pakiet.timestamp, pakiet.cuchy);
                add_by_time(queue, pakiet.timestamp, pakiet.src_id, pakiet.cuchy);  //dodajemy element do kolejki według znacznika czasowego
                print_queue(queue);  //wypisujemy kolejke

                pthread_mutex_lock(&timerMut);
                timer++;
                send_packet(0, pakiet.src_id, ACK);  //wysyłamy potwierdzenie
                pthread_mutex_unlock(&timerMut);

                if(pakiet.src_id == rank_comm) {                                                            // nie rozumiem po co to jest
                    ubiegam_sie = true;
                }
                break;
                }
            case RELEASE: {
                int del_src = pakiet.src_id;                //zapisujemy do zmiennej id otaku bo nie jest już w kolejce i mysimy go usunąć
                printf("%d Dostałem RELEASE od %d  %dz zegarem:%d \n",rank_comm, pakiet.src_id, del_src, pakiet.timestamp);
                int pos = find_by_src(queue, del_src);          //znachodzimy pozycju w kolejce otaku którego chcemy usunąć
                if (ptn_num_w_kolejce_policzony < pos) {        //sprawdzamy czy X jest przekroczony
                    update_cuchy(pos);
                }
                pop_by_src(queue, pakiet.src_id);       //usuwamy z kolejki
                print_queue(queue);
                ptn_num_w_kolejce_policzony--;
                if (ptn_num_w_kolejce_policzony == -1) wyzerowanie_kolejki = false;    

                if (ubiegam_sie) try_to_enter();        //probujemy wejść do pokoju
                else if (pakiet.src_id == rank_comm) {  //jeśli otrzymaliśmy REALESE od siebie, to zwjększmy cuchy
                    // srand(rank_comm);
                    my_cuchy = my_cuchy + rand() % MAX_CUCH_INCREASE + 1;
                    pthread_mutex_unlock(&leaveRoomMut);
                }
                break;
                }
        }
    } return nullptr;
}


void try_to_enter() {
    if (! ubiegam_sie) return;
    printf("%d  --- Sprawdzanie możliwości wejścia do pokoju --- \n", rank_comm);

    // wszystkie ACK
    if (ACK_got < num_otaku) {
        printf("%d 1. Otrzymane ACK (ACK:%d, aktualne:%d): NIE \n",rank_comm, num_otaku, ACK_got);
        printf("%d ---NIE ZEZWOLONO NA DOSTĘP--- \n", rank_comm);
        return;
    } else printf("%d 1. Otrzymano wszystkie ACK: TAK \n", rank_comm);

    // istnieje wolne stanowisko
    int moj_idx = find_by_rank(queue);   
    if (moj_idx+1 > shower_stand_num) {
        printf("%d 2. Jest miejsce (S:%d, ja:%d): NIE \n",rank_comm, shower_stand_num, moj_idx + 1);
        printf("%d ---NIE ZEZWOLONO NA DOSTĘP--- \n", rank_comm);
        return;
    } else printf("%d 2. Jest miejsce w pokoju: TAK \n", rank_comm);
    
    // my_cychy mieszczą się w limicie M
    int suma_cuchow = 0;
    for (int i = 0; i <= moj_idx; i++) {     //sumujemy cuchy wszystkich kto stoi przed nami w kolejce + siebie
        struct part* element = get_by_id(queue, i);
        suma_cuchow += element->cuchy;
        if (suma_cuchow > M) {             
            printf("%d 3. Cuchy w pomieszczeniu przekroczone (M:%d, aktualne:%d): NIE \n",rank_comm, M, suma_cuchow);
            printf("%d ---NIE ZEZWOLONO NA DOSTĘP--- \n", rank_comm);
            return;
        }
    } 
    printf("%d 3. Moje cuchy mieszczą się w pokoju (M:%d, aktualne:%d): TAK \n",rank_comm, M, suma_cuchow);
    
    // nie trzeba wymieniać przestawiciela (X)
    if (wyzerowanie_kolejki) {
        if (ptn_num_w_kolejce_policzony > -1) {
            printf("%d 4. Wyzerowano kolejkę: NIE \n", rank_comm);
            printf("%d ---NIE ZEZWOLONO NA DOSTĘP--- \n", rank_comm);
            return;
        } else printf("%d 4. Wyzerowano kolejkę: TAK \n", rank_comm);
    } else if (! counted_X) {
        update_cuchy(find_by_rank(queue));                  //tu bieżemy tylko jeden pierwszy element w kolejce, a potrzedujemy wziąć wszystkich poprzedników jeśli liczba stanowisk zezwała na wejscie tych osób + mnie
        if (wyzerowanie_kolejki) {
            if (ptn_num_w_kolejce_policzony > -1) {
                printf("%d 4. Wyzerowano kolejkę: NIE \n", rank_comm);
                printf("%d ---NIE ZEZWOLONO NA DOSTĘP--- \n", rank_comm);
                return;
            } else printf("%d 4. Wyzerowano kolejkę: TAK \n", rank_comm);
        }
    }

    printf("%d --- ZEZWOLONO NA DOSTĘP --- \n", rank_comm);
    ACK_got = 0;
    ubiegam_sie = false;
    counted_X = false;
    pthread_mutex_unlock(&roomMut);
}

void update_cuchy(int pos) {
    while(ptn_num_w_kolejce_policzony < pos) {
        ptn_num_w_kolejce_policzony++;
        struct part *request = get_by_id(queue, ptn_num_w_kolejce_policzony);
        current_x += request->cuchy;
        if(current_x >= X) {
            printf("%d !!! PROCES %d PRZEPEŁNIŁ X (X:%d, aktualny_X:%d): zerowanie X \n",rank_comm, request->src_id, X, current_x);
            printf("%d Przedstawiciel zemdlał przez proces: %d \n",rank_comm, request->src_id);
            current_x = 0;
            if(ubiegam_sie && ptn_num_w_kolejce_policzony != pos) {
                wyzerowanie_kolejki = true;
            }
            if(ubiegam_sie && ptn_num_w_kolejce_policzony >= pos) {
                counted_X = true;
            }
            break;
        }
    }
    if(ubiegam_sie && ptn_num_w_kolejce_policzony >= pos) {
        counted_X = true;
    }
    printf("%d KONTROLA X (X:%d, aktualny_X:%d) \n",rank_comm, X, current_x);
}

