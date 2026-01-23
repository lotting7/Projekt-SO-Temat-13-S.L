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

// funkcja do zapisywania logow w sposob bezpieczny (semafor)
// zapobiega mieszaniu sie logow z wielu procesow
void safe_log(int sem_id, string msg) {
    // bierzemy "klucz" do logu
    lock_sem(sem_id, 4);

    // zapisujemy log
    cout << "[PID KLIENTA: " + to_string(getpid()) + "] " + msg << endl;

    // zwalniamy semafor
    unlock_sem(sem_id, 4);
}


// wejscie na most
// dir_req: 1 = wchodzimy do jaskini, 2 = wychodzimy z jaskini
void czekaj_na_most(int sem_id, CaveState* jaskinia, int dir_req) {
    while (true) {
        // blokada semafora
        lock_sem(sem_id, 0);

        // sprawdzenie warunkow wejscia na most
        bool jest_miejsce = (jaskinia->people_on_bridge < LIMIT_BRIDGE);
        bool dobry_kierunek = (jaskinia->bridge_direction == 0 || jaskinia->bridge_direction == dir_req);

        if (jest_miejsce && dobry_kierunek) {
            // aktualizacja licznika na kladce
            jaskinia->people_on_bridge++;
            jaskinia->bridge_direction = dir_req; // ustawiamy kierunek

            // zwalniamy semafor i wychodzimy z petli
            unlock_sem(sem_id, 0);
            break;
        }

        // nie mozna wejsc na most, zwalniamy semafor
        unlock_sem(sem_id, 0);

       // czekanie w kolejce
        usleep(50000);
    }
}

void zejdz_z_mostu(int sem_id, CaveState* jaskinia) {
    lock_sem(sem_id, 0); // uzycie mutex, czyli blokada ktora pozwala jednemu procesowi uzywac danej zmiennej

    jaskinia->people_on_bridge--;
    // reset kierunku jesli most pusty
    if (jaskinia->people_on_bridge == 0) {
        jaskinia->bridge_direction = 0;
    }

    unlock_sem(sem_id, 0);
}

// funkcja zycia turysty
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


    bilet.visitor_id = getpid(); // id turysty to jego PID
    bilet.age = age; // wiek turysty
    bilet.route_choice = route; // wybor trasy
    bilet.is_repeater = is_repeater; // czy turysta jest powracajacy

    // dziecko ma opieke doroslego (ponizej 8 lat)
    if (age < 8) {
        bilet.has_guardian = 1;
    } else {
        bilet.has_guardian = 0;
    }

    send_ticket(msg_id, bilet); // wysylka biletu do kasjera przez turyste

    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);


    // ETAP 0 REZERWACJA MIEJSCA W JASKINI - przed kładką

    // wybieramy semafor do rezerwacji miejsca na trasie
    int sem_num;
    if (route == 1) {
        sem_num = 1;
    } else {
        sem_num = 2;
    }

    // blokada semafora az nie zwolni sie miejsce
    lock_sem(sem_id, sem_num);


   // ETAP 1 WEJSCIE NA KLADKE
    czekaj_na_most(sem_id, jaskinia, 1); // kierunek 1 = wchodzi proces

    safe_log(sem_id, ">> Wchodze na KLADKE. Ide do trasy " + to_string(route));

    // czas na kladce
    usleep(200000);

    //  ETAP 2 SPRAWDZENIE CZY TRASA OTWARTA
    int czy_otwarte = 1;
    if (route == 1 && jaskinia->route1_open == 0) czy_otwarte = 0;
    if (route == 2 && jaskinia->route2_open == 0) czy_otwarte = 0;

    if (czy_otwarte == 0) {

        safe_log(sem_id, "[!] Trasa " + to_string(route) + " ZAMKNIETA przez Straznika! Powrot.");

        zejdz_z_mostu(sem_id, jaskinia);

        // odblokowujemy semafor z etapu 0
        unlock_sem(sem_id, sem_num);

        detach_memory((int*)jaskinia);
        exit(0);
    }

    // ETAP 3 - WEJSCIE NA TRASE

    // aktualizacja stanu jaskini - zejscie z kladki, wejscie na trase
    lock_sem(sem_id, 0); // mutex dla pamieci jaskini

    jaskinia->people_on_bridge--;      // schodzimy z kladki
    if (jaskinia->people_on_bridge == 0) jaskinia->bridge_direction = 0; // reset kierunku jesli pusty

    if (route == 1) jaskinia->people_on_route1++; // wchodzimy na trase
    else jaskinia->people_on_route2++;

    unlock_sem(sem_id, 0);


    safe_log(sem_id, "!!! ZSZEDLEM Z KLADKI -> JESTEM NA TRASIE " + to_string(route) + " !!!");

    // czas zwiedzania
    int czas_zwiedzania = 1500000 + rand() % 1000000;
    usleep(czas_zwiedzania);

    // ETAP 4 POWROT

    safe_log(sem_id, "Koniec zwiedzania. Czekam na powrot...");

    // ponowne wejscie na kladke (czekamy az most zwolni sie dla wychodzacych)
    czekaj_na_most(sem_id, jaskinia, 2);

    // wejscie na kladke - zwolnienie slotu na danej trasie - aktualizacja do przewodnika - aby wyswietlil poprawnie
    lock_sem(sem_id, 0);
    if (route == 1) jaskinia->people_on_route1--;
    else jaskinia->people_on_route2--;
    unlock_sem(sem_id, 0);

    // zwolnienie miejsca na trasie (semafor z etapu 0)
    unlock_sem(sem_id, sem_num);

    safe_log(sem_id, "<< Wchodze na KLADKE (do wyjscia).");

    // czas powrotu
    usleep(200000);

    // wyjscie z jaskini (zejscie z mostu na zewnatrz)
    zejdz_z_mostu(sem_id, jaskinia);


    safe_log(sem_id, "Wyszedlem z jaskini.");

    detach_memory((int*)jaskinia);
    exit(0);
}

int main() {

    // inicjalizacja randomu
    srand(time(NULL));
    setbuf(stdout, NULL);

    // pobranie ID zasobow IPC
    int msg_id = msgget(KEY_MSG, 0600);
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600);

    // pobranie semaforow
    int sem_id = semget(KEY_SEM, 5, 0600);

    if (msg_id == -1 || shm_id == -1 || sem_id == -1) {
        perror("Blad klienta - brak zasobow (uruchom ./main)");
        return 1;
    }

    // generowanie turystow
    for (int i = 0; i < 1000; i++) {

        // fork procesu turysty

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


    while (wait(NULL) > 0){}


    return 0;
}