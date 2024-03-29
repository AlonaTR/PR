#include "main.hpp"
#include "const.hpp"
#include "watek_glowny.hpp"
#include "watek_komunikacyjny.hpp"

using namespace std;

int shower_stand_num;
int num_otaku;
int M;
int X;
int my_cuchy;
int current_x = 0;
int ACK_got = 0;


bool ubiegam_sie = false;
bool wyzerowanie_kolejki = false;
bool counted_X = false;
int ptn_num_w_kolejce_policzony = -1;

struct Queue *queue;

/* programme part */
state_t stan = InLobby;
int size_comm = 0;               //ilość otaku
int rank_comm =0;                //indeks MPI otaku
int timer = 0;   
MPI_Datatype MPI_PAKIET_T;
pthread_t threadKom;


pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;      // wchodzimy gdy zminiamy stan
pthread_mutex_t timerMut = PTHREAD_MUTEX_INITIALIZER;    //sekcja krytyczna zegara
pthread_mutex_t roomMut = PTHREAD_MUTEX_INITIALIZER;        //sekcja krytyczna stanowiska w pokoju
pthread_mutex_t leaveRoomMut = PTHREAD_MUTEX_INITIALIZER;   //wchodzimy gdy chcemy wyjsć z pokoju



int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "Too little arguments passed; needs 3 got " << int(argc)-1 << '\n';
        return -1; 
    }

    init_MPI(argc, argv);
    init_program_vars(argc, argv);
    
    mainLoop();
    
    finalize();
    return 0;
}


void init_program_vars(int argc, char** argv) {
    shower_stand_num = atoi(argv[1]);
    M = atoi(argv[2]);
    X = atoi(argv[3]);

    num_otaku = size_comm; 
    srand (rank_comm);
    // in range 1 to X
    my_cuchy = rand() % M + 1;
    queue = create_queue();

    pthread_create(&threadKom, nullptr, startKomWatek, 0);
    printf("%d jestem \n", rank_comm);
}


void init_MPI(int argc, char** argv) {
    int provided;
    // MPI_THREAD_MULTIPLE - Multiple threads may call MPI, with no restrictions. 
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    /*  check thread support */
    cout << "THREAD SUPPORT: chcemy " << MPI_THREAD_MULTIPLE << ". Co otrzymamy?\n";
    if (provided == MPI_THREAD_MULTIPLE) {
        /* tego chcemy. Wszystkie inne powodują problemy */
        cout << "Pełne wsparcie dla wątków\n"; 

        
    } else {
        cout << "otrzymane wsparcie: " << provided << "\n";
        cout << "To nie jest odpowiednie wsparcie dla wątków - finalizing\n";
        fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - finalizing!\n");
        MPI_Finalize();
        exit(-1);
    }

    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    const int nitems=FIELDNO; /* bo packet_t ma FIELDNO pól */
    int       blocklengths[FIELDNO] = {1,1,1};
    MPI_Datatype typy[FIELDNO] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[FIELDNO]; 
    offsets[0] = offsetof(packet_t, timestamp);
    offsets[1] = offsetof(packet_t, src_id);
    offsets[2] = offsetof(packet_t, cuchy);

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);


    MPI_Comm_size(MPI_COMM_WORLD, &size_comm);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_comm);
    //cout << "Our print : " << size_comm << rank_comm << "\n";
}


void finalize() {
    /* Czekamy, aż wątek potomny się zakończy */
    printf("%d czekam na wątek \"komunikacyjny\"\n", rank_comm );
    pthread_join(threadKom,NULL);
    if (rank_comm == 0) pthread_join(threadKom, NULL);
    pthread_mutex_destroy( &stateMut);
    pthread_mutex_destroy( &timerMut);
    pthread_mutex_destroy( &roomMut);
    pthread_mutex_destroy( &leaveRoomMut);
    MPI_Type_free(&MPI_PAKIET_T);
    delete queue;
    MPI_Finalize();
}


void send_packet(packet_t *packet, int destination, int tag) {
    bool packet_created = false;
    
    if (packet == 0) {
        packet = new packet_t();                                        //czy można usunąć? tutaj sprawdzamy czy pakey jest pusty
        packet_created = true;
    }

    // timer++;
    packet->timestamp = timer;
    packet->cuchy = my_cuchy;
    packet->src_id = rank_comm;
    MPI_Send(packet, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);

    switch(tag) {
        case REQUEST:
        printf("%d Wysłano REQUEST do %d z cuchami %d \n",rank_comm, destination, packet->cuchy);
            break;
        case RELEASE:
            printf("%d Wysłano RELEASE do %d \n",rank_comm, destination);
            break;
        case ACK:
            printf("%d Wysłano ACK do %d \n",rank_comm, destination);
            break;
        default:
            printf("%d Packet sent to %d, unknown tag \n",rank_comm, destination);
            break;
    }
    if (packet_created) delete packet;
}


void change_state(state_t newState) {
    pthread_mutex_lock(&stateMut);
    stan = newState;
    pthread_mutex_unlock(&stateMut);
}
