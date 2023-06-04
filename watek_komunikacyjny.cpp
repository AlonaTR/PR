#include "main.hpp"
#include "watek_komunikacyjny.hpp"
#include "queue.hpp"

void *startKomWatek(void *ptr) {
    MPI_Status status;
    bool is_message = false;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	    debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&timerMut);
        timer = std::max(timer, pakiet.ts) + 1;
        pthread_mutex_unlock(&timerMut);

        switch ( status.MPI_TAG ) {
            case FINISH: {
                change_state(InFinish);
                break;
                }
            case ACK: {
                debug("Dostałem ACK od %d z zegarem:%d", pakiet.src_id, pakiet.ts);
                ACK_got++;
                if (ACK_got == num_otaku) try_to_enter();
                break;
                }
            case REQUEST: {
                debug("Dostałem REQUEST od %d z zegarem:%d i cuchami:%d", pakiet.src_id, pakiet.ts, pakiet.cuchy);
                add_by_time(queue, pakiet.ts, pakiet.src_id, pakiet.cuchy);
                print_queue(queue);

                pthread_mutex_lock(&timerMut);
                send_packet(0, pakiet.src_id, ACK);
                pthread_mutex_unlock(&timerMut);

                if(pakiet.src_id == rank_comm) {
                    ubiegam_sie = true;
                }
                break;
                }
            case RELEASE: {
                debug("Dostałem RELEASE od %d z zegarem:%d", pakiet.src_id, pakiet.ts);
                int pos = find_by_src(queue, pakiet.src_id);
                if (ptn_num_w_kolejce_policzony < pos) {
                    update_cuchy(pos);
                }
                pop_by_src(queue, pakiet.src_id);
                print_queue(queue);
                ptn_num_w_kolejce_policzony--;
                if (ptn_num_w_kolejce_policzony == -1) wyzerowanie_kolejki = false;

                if (ubiegam_sie) try_to_enter();
                else if (pakiet.src_id == rank_comm) {
                    my_cuchy += rand() % MAX_CUCH_INCREASE + 1;
                    pthread_mutex_unlock(&leaveRoomMut);
                }
                break;
                }
            case NOACK: {
                debug("Dostałem NOACK od %d z zegarem:%d", pakiet.src_id, pakiet.ts);
                NO_ACK_got++;
                if (ACK_got == num_otaku) try_to_enter();
                break;
                }
        }
    }
}


void try_to_enter() {
    debug(" --- Sprawdzanie możliwości wejścia do pokoju ---");

    // wszystkie ACK
    if (ACK_got < num_otaku) {
        debug("1. Otrzymane ACK (ACK:%d, aktualne:%d): NIE", num_otaku, ACK_got);
        debug("---NIE ZEZWOLONO NA DOSTĘP---");
        return;
    } else debug("1. Otrzymano wszystkie ACK: TAK");

    // istnieje wolne stanowisko
    int moj_idx = find_by_src(queue, rank_comm);
    if (moj_idx+1 > shower_stand_num) {
        debug("2. Jest miejsce (S:%d, ja:%d): NIE", shower_stand_num, moj_idx + 1);
        debug("---NIE ZEZWOLONO NA DOSTĘP---");
        return;
    } else debug("2. Jest miejsce w pokoju: TAK");
    
    // my_cychy mieszczą się w limicie M
    int suma_cuchow = 0;
    for (int i = 0; i < moj_idx; i++) {
        struct part* element = get_by_id(queue, i);
        suma_cuchow += element->cuchy;
        if (suma_cuchow > M) {
            debug("3. Cuchy w pomieszczeniu przekroczone (M:%d, aktualne:%d): NIE", M, suma_cuchow);
            debug("---NIE ZEZWOLONO NA DOSTĘP---");
            return;
        }
    } 
    debug("3. Moje cuchy mieszczą się w pokoju: TAK");
    
    // nie trzeba wymieniać przestawiciela (X)
    if (wyzerowanie_kolejki) {
        if (ptn_num_w_kolejce_policzony > -1) {
            debug("4. Wyzerowano kolejkę: NIE");
            debug("---NIE ZEZWOLONO NA DOSTĘP---");
            return;
        } else debug("4. Wyzerowano kolejkę: TAK");
    } else if (! counted_X) {
        update_cuchy(find_by_src(queue, rank_comm));
        if (wyzerowanie_kolejki) {
            if (ptn_num_w_kolejce_policzony > -1) {
                debug("4. Wyzerowano kolejkę: NIE");
                debug("---NIE ZEZWOLONO NA DOSTĘP---");
                return;
            } else debug("4. Wyzerowano kolejkę: TAK");
        }
    }

    debug("--- ZEZWOLONO NA DOSTĘP ---");
    ACK_got = 0;
    ubiegam_sie = false;
    counted_X = false;
    pthread_mutex_unlock(&roomMut);
}

void update_cuchy(int pos) {
    int my_idx = find_by_src(queue, rank_comm);
    while(ptn_num_w_kolejce_policzony < pos) {
        ptn_num_w_kolejce_policzony++;
        struct part *request = get_by_id(queue, ptn_num_w_kolejce_policzony);
        current_x += request->cuchy;
        if(current_x >= X) {
            debug("Proces %d przepełnił X (X:%d, aktualny_X:%d): zerowanie X", request->src_id, X, current_x);
            current_x = 0;
            if(ubiegam_sie && ptn_num_w_kolejce_policzony != my_idx) {
                wyzerowanie_kolejki = true;
            }
            if(ubiegam_sie && ptn_num_w_kolejce_policzony >= my_idx) {
                counted_X = true;
            }
            break;
        }
    }
    if(ubiegam_sie && ptn_num_w_kolejce_policzony >= my_idx) {
        counted_X = true;
    }
    debug("KONTROLA X (X:%d, aktualny_X:%d)", X, current_x);
}

