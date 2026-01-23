#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include "common.h"
#include "tools.h"

using namespace std;


CaveState* jaskinia_ptr = NULL; // wskaznik do pamieci dzielonej

// FUNKCJE OBSLUGI SYGNALOW

// sygnal do przelaczenia trasy 1 (otworz/zamknij)
void przelacz_trase1(int sig) {
    if (jaskinia_ptr != NULL) {
        // Logika toggle: jesli 1 to 0, jesli 0 to 1
        if (jaskinia_ptr->route1_open == 1) {
            jaskinia_ptr->route1_open = 0;
            cout << RED << "\n!!! SYGNAL 1: ZAMYKAM TRASE NR 1 !!!" << RESET << endl;
        } else {
            jaskinia_ptr->route1_open = 1;
            cout << GREEN << "\n!!! SYGNAL 1: OTWIERAM TRASE NR 1 !!!" << RESET << endl;
        }
    }
}

// sygnal do przelaczenia trasy 2 (otworz/zamknij)
void przelacz_trase2(int sig) {
    if (jaskinia_ptr != NULL) {
        if (jaskinia_ptr->route2_open == 1) {
            jaskinia_ptr->route2_open = 0;
            cout << RED << "\n!!! SYGNAL 2: ZAMYKAM TRASE NR 2 !!!" << RESET << endl;
        } else {
            jaskinia_ptr->route2_open = 1;
            cout << GREEN << "\n!!! SYGNAL 2: OTWIERAM TRASE NR 2 !!!" << RESET << endl;
        }
    }
}

// sygnal do zakonczenia programu
void koniec_programu(int sig) {
    cout << RED << "\nEWAKUACJA! Koncze prace..." << RESET << endl;
    if (jaskinia_ptr != NULL) {
        jaskinia_ptr->is_open = 0;
        detach_memory((int*)jaskinia_ptr);
    }
    exit(0);
}


int main() {
    cout << "--- PRZEWODNIK (MONITORING) ---" << endl;

    cout << "MOJ PID: " << BOLD << getpid() << RESET << " (Wpisz go w ./straznik)" << endl;

    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600); // pobranie istniejacej pamieci
    if (shm_id == -1) {
        perror("Jaskinia 'nie uruchomiona' - ./init");
        return 1;
    }
    jaskinia_ptr = (CaveState*)attach_memory(shm_id); // podlaczenie pamieci dzielonej
    jaskinia_ptr->pid_przewodnik = getpid(); // zapisanie PID przewodnika w pamieci

    // REJESTRACJA SYGNALOW
    signal(SIGUSR1, przelacz_trase1);
    signal(SIGUSR2, przelacz_trase2);
    signal(SIGINT, koniec_programu);

    int sem_id = semget(KEY_SEM, 4, 0600); // pobranie istniejacych semaforow

    cout << "Przewodnik: Aktywny." << endl;

    while (true) {
        // czyszczenie ekranu
       system("clear");

        cout << BOLD << "STATUS JASKINI (PID: " << getpid() << ")" << RESET << endl;

        cout << " Stan otwarcia: ";
        if (jaskinia_ptr->is_open) cout << GREEN << "OTWARTA" << RESET << endl;
        else cout << RED << "ZAMKNIETA" << RESET << endl;

        // wyswietlanie czy trasy sa czynne
        cout << " Trasa 1: ";
        if (jaskinia_ptr->route1_open) cout << GREEN << "CZYNNA" << RESET << endl;
        else cout << RED << "ZAMKNIETA [X]" << RESET << endl;

        cout << " Trasa 2: ";
        if (jaskinia_ptr->route2_open) cout << GREEN << "CZYNNA" << RESET << endl;
        else cout << RED << "ZAMKNIETA [X]" << RESET << endl;

        cout << "----------------------------------" << endl;
        cout << " BILANS: Sprzedane: " << YELLOW << jaskinia_ptr->tickets_sold << RESET
             << " | Darmowe (dzieci <3): " << CYAN << jaskinia_ptr->tickets_free << RESET << endl;
        cout << "----------------------------------" << endl;
        cout << " Ruch w srodku:" << endl;

        // strzalki kierunkowe
        string arrow = "-";
        if (jaskinia_ptr->bridge_direction == 1) arrow = GREEN ">>> (WCHODZA)" RESET;
        if (jaskinia_ptr->bridge_direction == 2) arrow = MAGENTA "<<< (WYCHODZA)" RESET;

        cout << " -> Kladka (Wejscie): " << BLUE << jaskinia_ptr->people_on_bridge << RESET << " / " << LIMIT_BRIDGE << " osob | Kierunek: " << arrow << endl;
        cout << " -> Trasa 1: " << BLUE << jaskinia_ptr->people_on_route1 << RESET << " / " << LIMIT_ROUTE_1 << " osob" << endl;
        cout << " -> Trasa 2: " << BLUE << jaskinia_ptr->people_on_route2 << RESET << " / " << LIMIT_ROUTE_2 << " osob" << endl;
        cout << "----------------------------------" << endl;
        cout << "Sterowanie: ./straznik" << endl;

        sleep(1);
    }

}