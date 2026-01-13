#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h> // nie bedzie zombie procesow
#include "common.h"
#include "tools.h"

using namespace std;

// symulacja 1 procesu
void turist_life(int id, int age, int route, int sem_id, int shm_id) {

    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);

    // ETAP 1 - KLADKA
    // wchodzimy na kladke (semafor 3)
    lock_sem(sem_id, 3);

    // Aktualizacja licznika na kladce
    lock_sem(sem_id, 0); // mutex
    jaskinia->people_on_bridge++;
    unlock_sem(sem_id, 0);

    cout << "(PID: " << getpid() << ") >> Wszedlem na KLADKE. Ide do trasy " << route << endl;

    sleep(2); // czas na kladce (skrocilem do 2s zeby szlo sprawniej przy 100 osobach)

    //  ETAP 2 - SPRAWDZENIE CZY TRASA OTWARTA


    int czy_otwarte = 1;
    if (route == 1 && jaskinia->route1_open == 0) czy_otwarte = 0;
    if (route == 2 && jaskinia->route2_open == 0) czy_otwarte = 0;

    if (czy_otwarte == 0) {
        // TRASA ZAMKNIETA
        cout << "(PID: " << getpid() << ") [!] Trasa " << route << " ZAMKNIETA przez Straznika! Wracam." << endl;


        lock_sem(sem_id, 0);
        jaskinia->people_on_bridge--;
        unlock_sem(sem_id, 0);

        unlock_sem(sem_id, 3); // zwalniamy miejsce na kladce
        detach_memory((int*)jaskinia);
        exit(0);
    }

    // ETAP 3 - WEJSCIE NA KLADKE
    int sem_num = (route == 1) ? 1 : 2;

    // czekamy na miejsce na trasie
    lock_sem(sem_id, sem_num);

    // wchodzimy na trase, schodzimy z kladki
    lock_sem(sem_id, 0);
    jaskinia->people_on_bridge--;      // schodzimy z kladki
    if (route == 1) jaskinia->people_on_route1++; // wchodzimy na trase
    else jaskinia->people_on_route2++;
    unlock_sem(sem_id, 0);

    // zwalniamy miejsce na kladce dla innych
    unlock_sem(sem_id, 3);

    cout << "(PID: " << getpid() << ") !!! ZSZEDLEM Z KLADKI -> JESTEM NA TRASIE " << route << " !!!" << endl;

    // czas zwiedzania
    int czas_zwiedzania = 5 + (rand() % 6);
    sleep(czas_zwiedzania);

    cout << "(PID: " << getpid() << ") Koniec zwiedzania. Wychodze." << endl;

    // wyjscie z trasy
    lock_sem(sem_id, 0);
    if (route == 1) jaskinia->people_on_route1--;
    else jaskinia->people_on_route2--;
    unlock_sem(sem_id, 0);

    // zwalnanie miejsca na trasie
    unlock_sem(sem_id, sem_num);

    detach_memory((int*)jaskinia);
    exit(0);
}

int main() {
    cout << "GENERATOR TURYSTOW " << endl;
    srand(time(NULL));
    setbuf(stdout, NULL);

    int msg_id = msgget(KEY_MSG, 0666);
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0666);
    int sem_id = semget(KEY_SEM, 4, 0666);

    if (msg_id == -1 || shm_id == -1 || sem_id == -1) {
        perror("Blad! Wpisz ./init");
        return 1;
    }


    for (int i = 0; i < 100; i++) {

        int age = rand() % 80;
        int route;

        if (age < 8 || age > 75) route = 2; else route = (rand() % 2) + 1;

        int is_repeater = 0;
        if ((rand() % 100) < 10) is_repeater = 1;

        TicketMessage bilet;
        // priorytet
        if (is_repeater == 1) bilet.mtype = 2; else bilet.mtype = 1;

        bilet.visitor_id = i;
        bilet.age = age;
        bilet.route_choice = route;
        bilet.has_guardian = (age < 8) ? 1 : 0;
        bilet.is_repeater = is_repeater;

        send_ticket(msg_id, bilet);

        pid_t pid = fork();

        if (pid == 0) {
            turist_life(i, age, route, sem_id, shm_id);
        }
        else if (pid > 0) {

            usleep(50000);
        }
        else {
            perror("Blad fork");
        }
    }

    return 0;
}