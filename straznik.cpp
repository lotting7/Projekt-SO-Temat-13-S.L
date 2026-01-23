#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/shm.h>

#include "tools.h"
#include "common.h"

using namespace std;

// Funkcja pomocnicza do czyszczenia ekranu
void clear_screen() {
    system("clear");
}


int main() {

    clear_screen();
    cout << BOLD << "STRAZNIK JASKINI" << RESET << endl;

    // podlaczamy sie do pamieci jaskini
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600); // pobranie istniejacej pamieci
    if (shm_id == -1) { // blad - brak jaskini
        perror(RED "Blad: Nie wykryto Jaskini (uruchom ./main)!" RESET);
        return 1;
    }
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id); // podlaczenie pamieci dzielonej

    int pid_guide = jaskinia->pid_przewodnik; // pobranie PID przewodnika z pamieci dzielonej
    int pid_mgr = jaskinia->pid_manager; // pobranie PID managera z pamieci dzielonej

    // sprawdzenie czy PIDy sa poprawne
    if (pid_guide <= 0 || pid_mgr <= 0) {
        cout << RED << "Blad: System jeszcze nie wstal poprawnie (brak PID)." << RESET << endl;
        return 1;
    }

    cout << "Namierzono Przewodnika (PID: " << BOLD << pid_guide << RESET << ")" << endl;
    cout << "Namierzono Managera    (PID: " << BOLD << pid_mgr << RESET << ")" << endl;
    sleep(1); // maly pause

    while (true) {

        clear_screen();

        cout << BOLD << "--- PANEL STRAZNIKA ---" << RESET << endl;

        cout << "\n--- MENU STEROWANIA ---" << endl;
        cout << "1 - " << GREEN << "Przelacz Trase 1 (Otworz/Zamknij)" << RESET << endl;
        cout << "2 - " << GREEN << "Przelacz Trase 2 (Otworz/Zamknij)" << RESET << endl;
        cout << "9 - " << RED << BOLD << "KONIEC" << RESET << endl;
        cout << "0 - Wyjscie ze Straznika" << endl;
        cout << "Wybor: ";

        int opcja;
        cin >> opcja;

        // sprawdzenie bledu wejscia
        if (cin.fail()) {
            cin.clear(); // czysci flagi bledu
            cin.ignore(1000, '\n'); // usuwa bledne znaki z bufora
            cout << RED << "Blad: Wpisz cyfre!" << RESET << endl;
            sleep(1);
            continue;
        }
        // wysylanie sygnalow do przewodnika
        if (opcja == 1) {
            kill(pid_guide, SIGUSR1);
            cout << GREEN << "-> Wyslano sygnal do Trasy 1." << RESET << endl;
            sleep(1); // czekamy 1s zeby zobaczyc komunikat przed odswiezeniem ekranu
        }

        // wysylanie sygnalow do przewodnika
        else if (opcja == 2) {
            kill(pid_guide, SIGUSR2);
            cout << GREEN << "-> Wyslano sygnal do Trasy 2." << RESET << endl;
            sleep(1);
        }

        // wysylanie sygnalu konca do przewodnika
        else if (opcja == 9) {
            kill(pid_mgr, SIGINT);
            cout << RED << "-> Wyslano rozkaz KONIEC." << RESET << endl;
            break;
        }
        // wyjscie z panelu straznika
        else if (opcja == 0) {
            cout << "Zamykam panel straznika." << endl;
            detach_memory((int*)jaskinia);
            break;
        }
        // nieznana opcja
        else {
            cout << "Nie ma takiej opcji." << endl;
            sleep(1);
        }
    }

    return 0;
}