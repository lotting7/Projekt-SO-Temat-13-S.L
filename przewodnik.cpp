#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include "common.h"
#include "tools.h"

using namespace std;

// Zmienne globalne do obsługi sygnałów i stanu jaskini
CaveState* jaskinia_ptr = NULL;
int g_sem_id = -1;
int koniec = 0;

// -- OBSLUGA SYGNALOW --

// Funkcje przełączające stan tras w jaskini
void przelacz_trase1(int sig) {
    if (jaskinia_ptr != NULL && g_sem_id != -1) {
        lock_sem(g_sem_id, SEM_ACCESS);
        if (jaskinia_ptr->route1_open == 1) {
            jaskinia_ptr->route1_open = 0;
            cout << RED << "\n!!! SYGNAL 1: ZAMYKAM TRASE NR 1 !!!" << RESET << endl;
        } else {
            jaskinia_ptr->route1_open = 1;
            cout << GREEN << "\n!!! SYGNAL 1: OTWIERAM TRASE NR 1 !!!" << RESET << endl;
        }
        unlock_sem(g_sem_id, SEM_ACCESS);
    }
}

void przelacz_trase2(int sig) {
    if (jaskinia_ptr != NULL && g_sem_id != -1) {
        lock_sem(g_sem_id, SEM_ACCESS);
        if (jaskinia_ptr->route2_open == 1) {
            jaskinia_ptr->route2_open = 0;
            cout << RED << "\n!!! SYGNAL 2: ZAMYKAM TRASE NR 2 !!!" << RESET << endl;
        } else {
            jaskinia_ptr->route2_open = 1;
            cout << GREEN << "\n!!! SYGNAL 2: OTWIERAM TRASE NR 2 !!!" << RESET << endl;
        }
        unlock_sem(g_sem_id, SEM_ACCESS);
    }
}
// Obsługa sygnalu SIGINT do zakończenia programu (Ctrl+C)
void koniec_programu(int sig) {
    koniec = 1;
}

int main() {
    cout << "--- PRZEWODNIK (MONITORING) ---" << endl;
    cout << "MOJ PID: " << BOLD << getpid() << RESET << endl;

	// Podłączenie do istniejącej pamięci dzielonej jaskini
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600);
    if (shm_id == -1) {
        perror("Jaskinia 'nie uruchomiona' - ./main");
        return 1;
    }

    g_sem_id = semget(KEY_SEM, SEM_COUNT, 0600);
    if (g_sem_id == -1) {
        perror("Brak semaforow");
        return 1;
    }

	// Podłączenie do pamięci dzielonej
    jaskinia_ptr = (CaveState*)attach_memory(shm_id);

	// Zapisanie swojego PIDu do pamięci dzielonej
    lock_sem(g_sem_id, SEM_ACCESS);
    jaskinia_ptr->pid_przewodnik = getpid();
    unlock_sem(g_sem_id, SEM_ACCESS);

	// Ustawienie obsługi sygnałów
    signal(SIGUSR1, przelacz_trase1);
    signal(SIGUSR2, przelacz_trase2);
    signal(SIGINT, koniec_programu);

    cout << "Przewodnik: Aktywny." << endl;

    // Licznik do sprawdzenia czy jaskinia jest pusta
    int pusta_licznik = 0;

    while (!koniec) {
        system("clear");

		// Odczyt i wyświetlenie stanu jaskini
        lock_sem(g_sem_id, SEM_ACCESS);

        // Zapisujemy wartosci do sprawdzenia po wyswietleniu. Potrzebne do automatycznego zakonczenia
        int t1 = jaskinia_ptr->people_on_route1;
        int t2 = jaskinia_ptr->people_on_route2;
        int most = jaskinia_ptr->people_on_bridge;
        int w_in = jaskinia_ptr->bridge_waiting_in;
        int w_out = jaskinia_ptr->bridge_waiting_out;
        int suma_biletow = jaskinia_ptr->tickets_sold + jaskinia_ptr->tickets_free;

        cout << BOLD << "STATUS JASKINI (PID: " << getpid() << ")" << RESET << endl;

		// Stan jaskini
        cout << " Stan otwarcia: ";
        if (jaskinia_ptr->is_open) cout << GREEN << "OTWARTA" << RESET << endl;
        else cout << RED << "ZAMKNIETA" << RESET << endl;

		// Stan Tras
        cout << " Trasa 1: ";
        if (jaskinia_ptr->route1_open) cout << GREEN << "CZYNNA" << RESET << endl;
        else cout << RED << "ZAMKNIETA [X]" << RESET << endl;

        cout << " Trasa 2: ";
        if (jaskinia_ptr->route2_open) cout << GREEN << "CZYNNA" << RESET << endl;
        else cout << RED << "ZAMKNIETA [X]" << RESET << endl;

		// Bilans biletów
        cout << "----------------------------------" << endl;
        cout << " BILANS: Sprzedane: " << YELLOW << jaskinia_ptr->tickets_sold << RESET
             << " | Darmowe (dzieci <3): " << CYAN << jaskinia_ptr->tickets_free << RESET << endl;
        cout << "----------------------------------" << endl;
        cout << " Ruch w srodku:" << endl;

		// Wizualizacja kierunku mostu
        string arrow = "-";
        if (jaskinia_ptr->bridge_direction == 1) arrow = GREEN ">>> (WCHODZA)" RESET;
        if (jaskinia_ptr->bridge_direction == 2) arrow = MAGENTA "<<< (WYCHODZA)" RESET;

        cout << " -> Kladka (Wejscie): " << BLUE << jaskinia_ptr->people_on_bridge << RESET << " / " << LIMIT_BRIDGE << " osob | Kierunek: " << arrow << endl;
        cout << " -> Czeka wejscie: " << jaskinia_ptr->bridge_waiting_in
             << " | wyjscie: " << jaskinia_ptr->bridge_waiting_out << endl;
        cout << " -> Trasa 1: " << BLUE << jaskinia_ptr->people_on_route1 << RESET << " / " << LIMIT_ROUTE_1 << " osob" << endl;
        cout << " -> Trasa 2: " << BLUE << jaskinia_ptr->people_on_route2 << RESET << " / " << LIMIT_ROUTE_2 << " osob" << endl;
        cout << "----------------------------------" << endl;
        cout << "Sterowanie: ./straznik" << endl;

        unlock_sem(g_sem_id, SEM_ACCESS);

       // Instrukcja automatycznego zakończenia pracy przewodnika, gdy jaskinia jest pusta i nie ma już klientów
       // (po 10 kolejnych odczytach bez zmian)
        if (t1 == 0 && t2 == 0 && most == 0 && w_in == 0 && w_out == 0
            && suma_biletow > 0) {

            // Sprawdzenie czy klient nadal żyje
            lock_sem(g_sem_id, SEM_ACCESS);
            int pid_kl = jaskinia_ptr->pid_klient;
            unlock_sem(g_sem_id, SEM_ACCESS);

            // kill(pid_kl, 0) zwraca 0 jeśli proces istnieje, -1 jeśli nie istnieje lub brak uprawnień
            bool klient_zyje = (pid_kl > 0 && kill(pid_kl, 0) == 0);

            // Jeśli klient nie żyje, zwiększamy licznik pustych odczytów
            if (!klient_zyje) {
                pusta_licznik++;
                if (pusta_licznik >= 10) {
                    cout << GREEN << "\n[PRZEWODNIK] Jaskinia pusta - koncze monitoring." << RESET << endl;
                    break;
                }
            }
            // Jeśli klient nadal żyje, resetujemy licznik pustych odczytów
            } else {
                pusta_licznik = 0;
            }

    usleep(200000); // Odświeżanie co 200ms interfejsu
    }

    // Zakończenie pracy przewodnika
    cout << RED << "\nPrzewodnik: Koncze prace." << RESET << endl;
    if (jaskinia_ptr != NULL) {
        detach_memory((int*)jaskinia_ptr);
    }

    return 0;

}