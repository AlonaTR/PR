#include "main.hpp"
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
bool wyzerowanie_kolejki = false;   // nwm czy potrzebne
bool policono_X = false;
int ptn_num_w_kolejce_policzony = -1;

struct Queue *queue;

/* programme part */
state_t stan = InLobby;
int size_comm, rank_comm, timer = 0;
MPI_Datatype MPI_PAKIET_T;
pthread_t threadKom, threadMon;

pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timerMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t roomMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t leaveRoomMut = PTHREAD_MUTEX_INITIALIZER;


void init_program_vars(int argc, char** argv) {
    shower_stand_num = atoi(argv[1]);
    M = atoi(argv[2]);
    X = atoi(argv[3]);

    num_otaku = size_comm;
    // in range 1 to X
    my_cuchy = rand() % X + 1;

    // pthread_create(&threadKom, nullptr, startKomWatek, 0);
    debug("jestem");
}


void init_MPI(int argc, char** argv) {
    int provided;
    // MPI_THREAD_MULTIPLE - Multiple threads may call MPI, with no restrictions. 
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", MPI_THREAD_MULTIPLE);
    if (provided == MPI_THREAD_MULTIPLE) {
        /* tego chcemy. Wszystkie inne powodują problemy */
        cout << "Pełne wsparcie dla wątków\n"; 
    } else {
        cout << "otrzymane wsparcie: " << provided;
        cout << "To nie jest odpowiednie wsparcie dla wątków - finalizing\n";
        fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - finalizing!\n");
        MPI_Finalize();
        exit(-1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size_comm);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_comm);
}


void finalize() {
    MPI_Finalize();
}


int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "Too little arguments passed; needs 3 got " << int(argc)-1 << '\n';
        return -1; 
    }

    init_MPI(argc, argv);
    init_program_vars(argc, argv);
    int processes;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    cout << "Hello world from proc. " << rank << " out of " << processes << " proc." << endl;
    
    finalize();
    return 0;
}
