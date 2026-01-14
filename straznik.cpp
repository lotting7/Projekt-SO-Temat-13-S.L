#include <iostream>
#include <signal.h>

#include <unistd.h>
#include "tools.h"

using namespace std;

int main() {

    cout << "\033[H\033[J";
    cout << BOLD << "STRAZNIK JASKINI" << RESET << endl;

    int pid;
    cout << "Podaj PID procesu Przewodnik: ";
    cin >> pid;

    while (true) {

        cout << "\033[H\033[J";

        cout << BOLD << "--- PANEL STRAZNIKA (Cel PID: " << pid << ") ---" << RESET << endl;

        cout << "\n--- MENU STEROWANIA ---" << endl;
        cout << "1 - " << GREEN << "Przelacz Trase 1 (Otworz/Zamknij)" << RESET << endl;
        cout << "2 - " << GREEN << "Przelacz Trase 2 (Otworz/Zamknij)" << RESET << endl;
        cout << "9 - " << RED << BOLD << "KONIEC" << RESET << endl;
        cout << "0 - Wyjscie ze Straznika" << endl;
        cout << "Wybor: ";

        int opcja;
        cin >> opcja;

        if (opcja == 1) {
            kill(pid, SIGUSR1);
            cout << GREEN << "-> Wyslano sygnal do Trasy 1." << RESET << endl;
            sleep(1); // czekamy 1s zeby zobaczyc komunikat przed odswiezeniem ekranu
        }
        else if (opcja == 2) {
            kill(pid, SIGUSR2);
            cout << GREEN << "-> Wyslano sygnal do Trasy 2." << RESET << endl;
            sleep(1);
        }
        else if (opcja == 9) {
            kill(pid, SIGINT);
            cout << RED << "-> Wyslano rozkaz KONIEC." << RESET << endl;
            break;
        }
        else if (opcja == 0) {
            cout << "Zamykam panel straznika." << endl;
            break;
        }
        else {
            cout << "Nie ma takiej opcji." << endl;
            sleep(1);
        }
    }

    return 0;
}