#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "common.h"
#include "tools.h"

using namespace std;

int main() {
    cout << "--- PRZEWODNIK (MONITORING) ---" << endl;

    // podlaczenie do memory
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0666);
    if (shm_id == -1) {
        perror("Jaskinia 'nie uruchomiona' - ./init");
        return 1;
    }
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);

    // podlaczenie do semaforow - pobieranie id
    int sem_id = semget(KEY_SEM, 4, 0666);

    cout << "Przewodnik: Aktywny. Obserwuje ruch" << endl;

    while (true) {
        system("clear");

        cout << "STATUS JASKINI" << endl;

        //  odczytanie danych z shared memory
        cout << " Stan otwarcia: " << (jaskinia->is_open ? "OTWARTA" : "ZAMKNIETA") << endl;
        cout << "----------------------------------" << endl;
        cout << " BILANS: Sprzedane: " << jaskinia->tickets_sold
             << " | Darmowe (dzieci <3): " << jaskinia->tickets_free << endl;
        cout << "----------------------------------" << endl;
        cout << " Ruch w srodku:" << endl;
        cout << " -> Kladka (Wejscie): " << jaskinia->people_on_bridge << " / " << LIMIT_BRIDGE << " osob" << endl;
        cout << " -> Trasa 1: " << jaskinia->people_on_route1 << " / " << LIMIT_ROUTE_1 << " osob" << endl;
        cout << " -> Trasa 2: " << jaskinia->people_on_route2 << " / " << LIMIT_ROUTE_2 << " osob" << endl;
        cout << "----------------------------------" << endl;
        cout << "Przewodnik AKTYWNY..." << endl;

        sleep(1);
    }

    detach_memory((int*)jaskinia);
    return 0;
}