// Cleanup jako narzedzie awaryjne do czyszczenia zasobow IPC i zabicia procesow jaskini

#include <iostream>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <cstdlib>
#include <unistd.h>
#include "common.h"

using namespace std;

int main() {
    cout << "CLEANUP - czyszczenie wszystkiego..." << endl;

    // Wymuszone zabicie procesow jaskini
    system("pkill -9 -f './main'");
    system("pkill -9 -f './klient'");
    system("pkill -9 -f './kasjer'");
    system("pkill -9 -f './przewodnik'");
    system("pkill -9 -f './straznik'");

    // Usun zasoby IPC
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600);
    if (shm_id != -1) shmctl(shm_id, IPC_RMID, NULL);

    int sem_id = semget(KEY_SEM, SEM_COUNT, 0600);
    if (sem_id != -1) semctl(sem_id, 0, IPC_RMID);

    int msg_id = msgget(KEY_MSG, 0600);
    if (msg_id != -1) msgctl(msg_id, IPC_RMID, NULL);

    cout << "Gotowe!" << endl;
    return 0;
}