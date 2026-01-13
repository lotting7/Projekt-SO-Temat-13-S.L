#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int main() {
    cout << "--- STRAZNIK JASKINI ---" << endl;

    int pid;
    cout << "Podaj PID procesu Przewodnik: ";
    cin >> pid;

    while (true) {
        cout << "\n--- MENU STEROWANIA ---" << endl;
        cout << "1 - Przelacz Trase 1 (Otworz/Zamknij)" << endl;
        cout << "2 - Przelacz Trase 2 (Otworz/Zamknij)" << endl;
        cout << "9 - Ewakuacja CALKOWITA (Koniec)" << endl;
        cout << "0 - Wyjscie ze Straznika" << endl;
        cout << "Wybor: ";

        int opcja;
        cin >> opcja;

        if (opcja == 1) {
            kill(pid, SIGUSR1);
            cout << "-> Wyslano sygnal do Trasy 1." << endl;
        }
        else if (opcja == 2) {
            kill(pid, SIGUSR2);
            cout << "-> Wyslano sygnal do Trasy 2." << endl;
        }
        else if (opcja == 9) {
            kill(pid, SIGINT);
            cout << "-> Wyslano rozkaz KONIEC." << endl;
            break;
        }
        else if (opcja == 0) {
            break;
        }
    }

    return 0;
}