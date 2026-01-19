#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include "common.h"
#include "tools.h"

using namespace std;


// wejscie na most
// dir_req: 1 = wchodzimy do jaskini, 2 = wychodzimy z jaskini
void czekaj_na_most(int sem_id, CaveState* jaskinia, int dir_req) {
    while (true) {
        // blokowanie dostepu do pamieci
        lock_sem(sem_id, 0);

        // sprawdzamy czy sa spelnione warunki
        bool jest_miejsce = (jaskinia->people_on_bridge < LIMIT_BRIDGE);
        bool dobry_kierunek = (jaskinia->bridge_direction == 0 || jaskinia->bridge_direction == dir_req);

        if (jest_miejsce && dobry_kierunek) {
            // aktualizacja licznika na kladce
            jaskinia->people_on_bridge++;
            jaskinia->bridge_direction = dir_req; // ustawiamy kierunek

            // zwolnienie blokady
            unlock_sem(sem_id, 0);
            break;
        }

        // jesli nie mozna wejsc - zwalniamy semafor i czekamy
        unlock_sem(sem_id, 0);

       // czekanie w kolejce
        usleep(50000);
    }
}

void zejdz_z_mostu(int sem_id, CaveState* jaskinia) {
    lock_sem(sem_id, 0); // uzycie mutex, czyli blokada ktora pozwala jednemu procesowi uzywac danej zmiennej

    jaskinia->people_on_bridge--;
    // jesli ostatnia osoba zeszla, most jest pusty (kierunek 0)
    if (jaskinia->people_on_bridge == 0) {
        jaskinia->bridge_direction = 0;
    }

    unlock_sem(sem_id, 0);
}

// symulacja procesu
void turist_life(int id, int sem_id, int shm_id, int msg_id) {

    srand(time(NULL) ^ getpid());


    int age = rand() % 80;
    int route;
    // dzieci ponizej 8 rz i seniozy powyzej 75 - trasa 2
    if (age < 8 || age > 75) route = 2; else route = (rand() % 2) + 1;

    int is_repeater = 0;
    if ((rand() % 100) < 10) is_repeater = 1;

    TicketMessage bilet = {};
    if (is_repeater == 1) bilet.mtype = 2; else bilet.mtype = 1; // priorytet

    bilet.visitor_id = id;
    bilet.age = age;
    bilet.route_choice = route;
    bilet.has_guardian = (age < 8) ? 1 : 0; // wymog: dzieci < 8 z opiekunem
    bilet.is_repeater = is_repeater;

    send_ticket(msg_id, bilet); // turysta wysyla bilet do kasjera

    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);


    // ETAP 0 REZERWACJA MIEJSCA W JASKINI - przed kładką

    // Ustalamy, ktory semafor nas interesuje
    int sem_num = (route == 1) ? 1 : 2;

    // blokada semafora az nie zwolni sie miejsce
    lock_sem(sem_id, sem_num);


   // ETAP 1 WEJSCIE NA KLADKE
    czekaj_na_most(sem_id, jaskinia, 1); // kierunek 1 = wchodzi proces

    cout << CYAN << "(PID: " << getpid() << ") " << GREEN << ">> Wszedlem na KLADKE. Ide do trasy " << route << RESET << endl;

    // czas na kladce
    usleep(200000);

    //  ETAP 2 SPRAWDZENIE CZY TRASA OTWARTA
    int czy_otwarte = 1;
    if (route == 1 && jaskinia->route1_open == 0) czy_otwarte = 0;
    if (route == 2 && jaskinia->route2_open == 0) czy_otwarte = 0;

    if (czy_otwarte == 0) {
        // TRASA ZAMKNIETA
        cout << CYAN << "(PID: " << getpid() << ") " << RED << "[!] Trasa " << route << " ZAMKNIETA przez Straznika! Powrot." << RESET << endl;

        zejdz_z_mostu(sem_id, jaskinia);

        // odblokowujemy semafor z etapu 0
        unlock_sem(sem_id, sem_num);

        detach_memory((int*)jaskinia);
        exit(0);
    }

    // ETAP 3 - WEJSCIE NA TRASE


    // wchodzimy na trase, schodzimy z kladki
    lock_sem(sem_id, 0); // mutex

    jaskinia->people_on_bridge--;      // schodzimy z kladki
    if (jaskinia->people_on_bridge == 0) jaskinia->bridge_direction = 0; // reset kierunku jesli pusty

    if (route == 1) jaskinia->people_on_route1++; // wchodzimy na trase
    else jaskinia->people_on_route2++;

    unlock_sem(sem_id, 0);

    cout << CYAN << "(PID: " << getpid() << ") " << BLUE << "!!! ZSZEDLEM Z KLADKI -> JESTEM NA TRASIE " << route << " !!!" << RESET << endl;

    // czas zwiedzania
    int czas_zwiedzania = 1500000 + rand() % 1000000;
    usleep(czas_zwiedzania);

    // ETAP 4 POWROT

    cout << CYAN << "(PID: " << getpid() << ") " << RESET << "Koniec zwiedzania. Czekam na powrot..." << endl;

    // ponowne wejscie na kladke (czekamy az most zwolni sie dla wychodzacych)
    czekaj_na_most(sem_id, jaskinia, 2);

    // wejscie na kladke - zwolnienie slotu na danej trasie - aktualizacja do przewodnika - aby wyswietlil poprawnie
    lock_sem(sem_id, 0);
    if (route == 1) jaskinia->people_on_route1--;
    else jaskinia->people_on_route2--;
    unlock_sem(sem_id, 0);

    // faktycznie zwalnianie miejsca (semaforu) na trasie dla tych czekajacych na trawie
    unlock_sem(sem_id, sem_num);

    cout << CYAN << "(PID: " << getpid() << ") " << MAGENTA << "<< Wchodze na KLADKE (do wyjscia)." << RESET << endl;

    // czas powrotu
    usleep(200000);

    // wyjscie z jaskini (zejscie z mostu na zewnatrz)
    zejdz_z_mostu(sem_id, jaskinia);

    cout << CYAN << "(PID: " << getpid() << ") " << RESET << "Wyszedlem z jaskini." << endl;

    detach_memory((int*)jaskinia);
    exit(0);
}

int main() {
    cout << BOLD << "GENERATOR TURYSTOW" << RESET << endl;
    srand(time(NULL));
    setbuf(stdout, NULL);

    int msg_id = msgget(KEY_MSG, 0666);
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0666);
    int sem_id = semget(KEY_SEM, 4, 0666);

    if (msg_id == -1 || shm_id == -1 || sem_id == -1) {
        perror("Blad! Wpisz ./init");
        return 1;
    }

    // tworzenie turystow
    for (int i = 0; i < 300; i++) {

        // Rodzic tylko robi fork

        pid_t pid = fork();

        if (pid == 0) {
            turist_life(i, sem_id, shm_id, msg_id);
        }
        else if (pid > 0) {
             usleep(100000);
        }
        else {
            perror("Blad fork");
        }
    }

    cout << "Koniec generowania. Czekam na zakonczenie procesow" << endl;
    while (wait(NULL) > 0){}


    return 0;
}