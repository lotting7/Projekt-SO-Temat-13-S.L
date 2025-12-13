#include <iostream>
#include <unistd.h> //  sleep()
#include "common.h"
#include "tools.h"

using namespace std;

int main() {
    cout << "TEST" << endl;


    // tworzenie tablicy w memory shared
    int shm_id = create_shared_memory(KEY_SHM, sizeof(CaveState));
    // 4 semafory dla kazdej trasy
    int sem_id = create_semaphores(KEY_SEM, 4);
    // kolejka na "bilety"
    int msg_id = create_msg_queue(KEY_MSG);

    cout << "Udalo sie utworzyc zasoby!" << endl;
    cout << "ID Pamieci: " << shm_id << endl;
    cout << "ID Semaforow: " << sem_id << endl;
    cout << "ID Kolejki: " << msg_id << endl;


    // dolaczenie do tablicy
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);

    // zerowanie aby wszystko bylo puste
    jaskinia->people_on_route1 = 0;
    jaskinia->people_on_route2 = 0;
    jaskinia->people_on_bridge = 0;
    jaskinia->bridge_direction = 0;
    jaskinia->tickets_sold = 0;
    jaskinia->is_open = 1;          // 1 - otwarte

    cout << "\n  SPRAWDZENIE LIMITOW (z pliku common.h) " << endl;
    cout << "Limit Trasa 1: " << LIMIT_ROUTE_1 << endl;
    cout << "Limit Kladka (waskie gardlo): " << LIMIT_BRIDGE << endl;

    // usuwanie wszystkiego, zeby nic nie zostało
    detach_memory((int*)jaskinia); // odlaczanie

    remove_shared_memory(shm_id);
    remove_semaphores(sem_id);
    remove_msg_queue(msg_id);

    cout << "\n koniec testu" << endl;

    return 0;
}