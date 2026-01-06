#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include "common.h"
#include "tools.h"

using namespace std;

int main() {
    cout << "Sprzatanie - CleanUP wszystkiego" << endl;

    // znalezienie pamieci dzielonej
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0666);
    if (shm_id != -1) {
        remove_shared_memory(shm_id);
        cout << "[OK] Pamiec usunieta." << endl;
    } else {
        cout << "[INFO] Pamiec nie istniala." << endl;
    }

    // semafory
    int sem_id = semget(KEY_SEM, 4, 0666);
    if (sem_id != -1) {
        remove_semaphores(sem_id);
        cout << "[OK] Semafory usuniete." << endl;
    } else {
        cout << "[INFO] Semafory nie istnialy." << endl;
    }

    // kolejka komunikatow
    int msg_id = msgget(KEY_MSG, 0666);
    if (msg_id != -1) {
        remove_msg_queue(msg_id);
        cout << "[OK] Kolejka usunieta." << endl;
    } else {
        cout << "[INFO] Kolejka nie istniala." << endl;
    }

    cout << "Wyczyszczone wszystko" << endl;
    return 0;
}