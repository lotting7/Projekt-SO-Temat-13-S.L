#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include "common.h"
#include "tools.h"

using namespace std;

// Funkcja do bezpiecznego logowania z użyciem semafora
// Zapobiega mieszaniu się logów z różnych procesów
void safe_log(int sem_id, string msg) {
    lock_sem(sem_id, SEM_LOG);
    cout << "[PID: " << getpid() << "] " << msg << endl;
    unlock_sem(sem_id, SEM_LOG);
}

// Funkcja obsługująca wejście na most z odpowiednimi warunkami
// (kierunek: 1 - wejście, 2 - wyjście)
int wejdz_na_most(int sem_id, CaveState* jaskinia, int kierunek, int route) {

    // Sprawdzenie czy trasa jest otwarta PRZED czekaniem (tylko przy wejściu do jaskini)
    if (kierunek == 1) {
        lock_sem(sem_id, SEM_ACCESS);
        int otwarta = 1;
        if (route == 1 && jaskinia->route1_open == 0) otwarta = 0;
        if (route == 2 && jaskinia->route2_open == 0) otwarta = 0;
        unlock_sem(sem_id, SEM_ACCESS);
        if (otwarta == 0) {
            return 0;
        }
    }

	// Zwiekszenie liczby czekajacych na most w danym kierunku
    lock_sem(sem_id, SEM_ACCESS);
    if (kierunek == 1) {
        jaskinia->bridge_waiting_in++;
    } else {
        jaskinia->bridge_waiting_out++;
    }
    unlock_sem(sem_id, SEM_ACCESS);

	// Petla oczekujaca na mozliwosc wejscia na most
    while (true) {

        // Sprawdzenie czy trasa jest otwarta (tylko przy wejściu do jaskini)
        // (mogła się zamknąć podczas czekania na slot)
        if (kierunek == 1) {
            lock_sem(sem_id, SEM_ACCESS);
            int otwarta = 1;
            if (route == 1 && jaskinia->route1_open == 0) otwarta = 0;
            if (route == 2 && jaskinia->route2_open == 0) otwarta = 0;
            if (otwarta == 0) {
                jaskinia->bridge_waiting_in--;
                unlock_sem(sem_id, SEM_ACCESS);
                return 0;
            }
            unlock_sem(sem_id, SEM_ACCESS);
        }

        lock_sem(sem_id, SEM_ACCESS);

        bool mozna = false;

		// Sprawdzenie warunkow wejscia na most
        if (jaskinia->people_on_bridge < LIMIT_BRIDGE) {
            if (jaskinia->bridge_direction == 0 || jaskinia->bridge_direction == kierunek) {
                mozna = true;
            }

            // wchodzący muszą poczekać.
            if (kierunek == 1 && jaskinia->bridge_waiting_out > 0) {
                mozna = false;
            }
        }

		// Jesli mozna wejsc na most, aktualizujemy stan jaskini i wychodzimy z funkcji
        if (mozna) {
            if (kierunek == 1) {
                jaskinia->bridge_waiting_in--;
            } else {
                jaskinia->bridge_waiting_out--;
            }

			// Aktualizacja stanu mostu
            jaskinia->people_on_bridge++;
            jaskinia->bridge_direction = kierunek;
            unlock_sem(sem_id, SEM_ACCESS);

            return 1;
        }

		// Jesli nie mozna wejsc na most, zwalniamy semafor i czekamy
        unlock_sem(sem_id, SEM_ACCESS);

    usleep(5000); // Czekanie 5ms przed ponownym sprawdzeniem
    }
}


// Funkcja obsługująca zejście z mostu
// Aktualizuje stan jaskini po zejściu z mostu
void zejdz_z_mostu(int sem_id, CaveState* jaskinia) {
    lock_sem(sem_id, SEM_ACCESS);

    jaskinia->people_on_bridge--;

	// Jeśli most jest pusty, resetujemy kierunek mostu na 0
	// Dzięki temu kolejna osoba może wejść z drugiego kierunku
    if (jaskinia->people_on_bridge == 0) {
        jaskinia->bridge_direction = 0;
    }

    unlock_sem(sem_id, SEM_ACCESS);
}


// Główna funkcja symulująca życie turysty
void turist_life(int id, int sem_id, int shm_id, int msg_id) {

    setvbuf(stdout, NULL, _IONBF, 0);
    // Inicjalizacja generatora liczb losowych
    srand(time(NULL) ^ getpid());

    // Losowanie wieku turysty
    int age = rand() % 80;
    int route;

    // Wybór trasy na podstawie wieku
    if (age < AGE_CHILD || age > AGE_SENIOR) {
        route = 2;
    } else {
        route = (rand() % 2) + 1;
    }


    // Szansa 10% bycia turystą powracającym
    int is_repeater = 0;
    if ((rand() % 100) < 10) is_repeater = 1;


    // Zakup biletu
    TicketMessage bilet = {};

    // Ustawienie typu biletu (1 = VIP, 2 = normalny)
    if (is_repeater) {
        bilet.mtype = 1;
    } else {
        bilet.mtype = 2;
    }

    bilet.visitor_id = getpid();
    bilet.age = age;
    bilet.route_choice = route;
    bilet.is_repeater = is_repeater;

    // Ustawienie informacji o opiekunie dla dzieci
    if (age < AGE_CHILD) {
        bilet.has_guardian = 1; // Dziecko z opiekunem
    } else {
        bilet.has_guardian = 0; // Bez opiekuna
    }

    send_ticket(msg_id, sem_id, bilet);


   // sleep(30);

    // WEJSCIE DO JASKINI
    // Podłączenie do pamięci dzielonej jaskini
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);

    // Rezerwacja miejsca na trasie
    int sem_trasa = (route == 1) ? SEM_ROUTE1 : SEM_ROUTE2;
    lock_sem(sem_id, sem_trasa);

    // Wejście na most
    int wszedl = wejdz_na_most(sem_id, jaskinia, 1, route);

    if (wszedl == 0) {
        safe_log(sem_id, "[!] Trasa " + to_string(route) + " ZAMKNIETA! Rezygnuje przed mostem.");
        unlock_sem(sem_id, sem_trasa);
        detach_memory((int*)jaskinia);
        exit(0);
    }

    safe_log(sem_id, ">> Na KLADCE, ide do trasy " + to_string(route));

   usleep(10000); // Symulacja czasu przejścia po moście


    // Sprawdzenie czy trasa jest otwarta
    lock_sem(sem_id, SEM_ACCESS);
    int czy_otwarte = 1;
    if (route == 1 && jaskinia->route1_open == 0) czy_otwarte = 0;
    if (route == 2 && jaskinia->route2_open == 0) czy_otwarte = 0;
    unlock_sem(sem_id, SEM_ACCESS);

    // Obsluga przypadku zamkniętej trasy
    if (czy_otwarte == 0) {
        safe_log(sem_id, "[!] Trasa " + to_string(route) + " ZAMKNIETA! Wracam.");
        zejdz_z_mostu(sem_id, jaskinia);
        unlock_sem(sem_id, sem_trasa);
        detach_memory((int*)jaskinia);
        exit(0);
    }

    // ZWIEDZANIE
    // Zejście z mostu - zwalnia slot na moście dla innych
    zejdz_z_mostu(sem_id, jaskinia);

    // Aktualizacja stanu jaskini po wejściu na trasę
    lock_sem(sem_id, SEM_ACCESS);
    if (route == 1) jaskinia->people_on_route1++;
    else jaskinia->people_on_route2++;
    unlock_sem(sem_id, SEM_ACCESS);

    safe_log(sem_id, "!!! JESTEM NA TRASIE " + to_string(route) + " !!!");

    usleep(20000 + rand() % 30000); // Symulacja czasu zwiedzania losowo od 20ms do 50ms


    safe_log(sem_id, "Koncze zwiedzanie, wracam...");

    // POWROT
    // Wejście na most w kierunku wyjścia
    int wyszedl = wejdz_na_most(sem_id, jaskinia, 2, route);

    // Zwolnienie miejsca na trasie
    lock_sem(sem_id, SEM_ACCESS);
    if (route == 1) jaskinia->people_on_route1--;
    else jaskinia->people_on_route2--;
    unlock_sem(sem_id, SEM_ACCESS);

    // Zwolnienie rezerwacji miejsca na trasie
    unlock_sem(sem_id, sem_trasa);

    safe_log(sem_id, "<< Na KLADCE, ide do wyjscia.");

   usleep(10000); // Symulacja czasu przejścia po moście


    zejdz_z_mostu(sem_id, jaskinia);

    safe_log(sem_id, "Wyszedlem z jaskini!");

    detach_memory((int*)jaskinia);
    exit(0);
}

int main() {

    srand(time(NULL));

	// Pobranie ID zasobów IPC utworzonych przez main
    int msg_id = msgget(KEY_MSG, 0600);
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600);
    int sem_id = semget(KEY_SEM, SEM_COUNT, 0600);

    if (msg_id == -1 || shm_id == -1 || sem_id == -1) {
        perror("Blad klienta - brak zasobow (uruchom ./main)");
        return 1;
    }

	// Petla tworząca procesy turystów
    for (int i = 0; i < 1000; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            turist_life(i, sem_id, shm_id, msg_id);
        }
        else if (pid < 0) {
            perror("Blad fork");
			break;
        }

    usleep(10000); // Opóźnienie między tworzeniem turystów

    }

    while (wait(NULL) > 0) {} // Czekaj na zakończenie wszystkich procesów turystów

    return 0;
}

