#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include "common.h"
#include "tools.h"

using namespace std;

// globalny wskaznik
CaveState* jaskinia_ptr = NULL;

// --- FUNKCJE OBSLUGI SYGNALOW (TERAZ DZIALAJA JAKO PRZELACZNIK) ---

void przelacz_trase1(int sig) {
    if (jaskinia_ptr != NULL) {
        // Logika toggle: jesli 1 to 0, jesli 0 to 1
        if (jaskinia_ptr->route1_open == 1) {
            jaskinia_ptr->route1_open = 0;
            cout << "\n!!! SYGNAL 1: ZAMYKAM TRASE NR 1 !!!" << endl;
        } else {
            jaskinia_ptr->route1_open = 1;
            cout << "\n!!! SYGNAL 1: OTWIERAM TRASE NR 1 !!!" << endl;
        }
    }
}

void przelacz_trase2(int sig) {
    if (jaskinia_ptr != NULL) {
        if (jaskinia_ptr->route2_open == 1) {
            jaskinia_ptr->route2_open = 0;
            cout << "\n!!! SYGNAL 2: ZAMYKAM TRASE NR 2 !!!" << endl;
        } else {
            jaskinia_ptr->route2_open = 1;
            cout << "\n!!! SYGNAL 2: OTWIERAM TRASE NR 2 !!!" << endl;
        }
    }
}

void koniec_programu(int sig) {
    cout << "\nEWAKUACJA! Koncze prace..." << endl;
    if (jaskinia_ptr != NULL) {
        jaskinia_ptr->is_open = 0;
        detach_memory((int*)jaskinia_ptr);
    }
    exit(0);
}

int main() {
    cout << "--- PRZEWODNIK (MONITORING) ---" << endl;

    cout << "MOJ PID: " << getpid() << " (Wpisz go w ./straznik)" << endl;

    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0666);
    if (shm_id == -1) {
        perror("Jaskinia 'nie uruchomiona' - ./init");
        return 1;
    }
    jaskinia_ptr = (CaveState*)attach_memory(shm_id);

    // REJESTRACJA SYGNALOW
    signal(SIGUSR1, przelacz_trase1);
    signal(SIGUSR2, przelacz_trase2);
    signal(SIGINT, koniec_programu);

    int sem_id = semget(KEY_SEM, 4, 0666);

    cout << "Przewodnik: Aktywny. Obserwuje ruch i czekam na sygnaly..." << endl;

    while (true) {
        // czyszczenie ekranu
        cout << "\033[H\033[J";

        cout << "STATUS JASKINI (PID: " << getpid() << ")" << endl;

        cout << " Stan otwarcia: " << (jaskinia_ptr->is_open ? "OTWARTA" : "ZAMKNIETA") << endl;

        // wyswietlanie czy trasy sa czynne
        cout << " Trasa 1: " << (jaskinia_ptr->route1_open ? "CZYNNA" : "ZAMKNIETA [X]") << endl;
        cout << " Trasa 2: " << (jaskinia_ptr->route2_open ? "CZYNNA" : "ZAMKNIETA [X]") << endl;

        cout << "----------------------------------" << endl;
        cout << " BILANS: Sprzedane: " << jaskinia_ptr->tickets_sold
             << " | Darmowe (dzieci <3): " << jaskinia_ptr->tickets_free << endl;
        cout << "----------------------------------" << endl;
        cout << " Ruch w srodku:" << endl;
        cout << " -> Kladka (Wejscie): " << jaskinia_ptr->people_on_bridge << " / " << LIMIT_BRIDGE << " osob" << endl;
        cout << " -> Trasa 1: " << jaskinia_ptr->people_on_route1 << " / " << LIMIT_ROUTE_1 << " osob" << endl;
        cout << " -> Trasa 2: " << jaskinia_ptr->people_on_route2 << " / " << LIMIT_ROUTE_2 << " osob" << endl;
        cout << "----------------------------------" << endl;
        cout << "Sterowanie: ./straznik" << endl;

        sleep(1);
    }

    detach_memory((int*)jaskinia_ptr);
    return 0;
}