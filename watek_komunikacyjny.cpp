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
            case FINISH: 
                change_state(InFinish);
                break;
            case ACK: 
                debug("Dostałem ACK od %d",pakiet.src_id);
                ACK_got++;
                if (ACK_got == num_otaku) try_to_enter();
                break;
            case REQUEST: 
                debug("Dostałem REQUEST od %d z cuchami %d",pakiet.src_id, pakiet.cuchy)
                add_by_time(queue, pakiet.ts, pakiet.src_id, pakiet.cuchy);
                print_queue(queue);

                pthread_mutex_lock(&timerMut);
                send_packet(0, pakiet.src_id, ACK);
                pthread_mutex_unlock(&timerMut);
                break;
            case RELEASE:
                debug("Dostałem RELEASE od %d", pakiet.src_id);
                int pos = find_by_src(queue, pakiet.src_id);
                break;
            default:
                break;
            }
    }
}


void try_to_enter() {

}