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

   // kladka
    lock_sem(sem_id, 3);

    // Aktualizacja licznika na kladce
    lock_sem(sem_id, 0); // mutex
    jaskinia->people_on_bridge++;
    unlock_sem(sem_id, 0);

    cout << "(PID: " << getpid() << ") >> Wszedlem na KLADKE. Ide do trasy " << route << endl;


    sleep(4); // czas spedzony na kladce

    // zejscie z kladki - wejscie na trase 1 lub 2
    int sem_num = (route == 1) ? 1 : 2;

    // czekamy na miejsce na trasie (stojac jeszcze na kladce)
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

    // czas zwiedzania 5-10sek
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
    cout << "GENERATOR TURYSTOW (PROCESY)" << endl;
    srand(time(NULL));
    setbuf(stdout, NULL);

    // pobieranie id
    int msg_id = msgget(KEY_MSG, 0666);
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0666);
    int sem_id = semget(KEY_SEM, 4, 0666);

    if (msg_id == -1 || shm_id == -1 || sem_id == -1) {
        perror("Blad! Wpisz ./init");
        return 1;
    }

    // tworzenie 15 turystow
    for (int i = 0; i < 15; i++) {

        // losowe dane
        int age = rand() % 80;
        int route = (rand() % 2) + 1; // 1 lub 2

        TicketMessage bilet;
        bilet.mtype = MSG_TICKET;
        bilet.visitor_id = i;
        bilet.age = age;
        bilet.route_choice = route;
        bilet.has_guardian = (age < 18) ? 1 : 0;

        send_ticket(msg_id, bilet);

        // tworzenie procesow
        pid_t pid = fork();

        if (pid == 0) { // proces stworzony dodatkowo - dziecko
            turist_life(i, age, route, sem_id, shm_id);
        }
        else if (pid > 0) {
            // rodzic czyli generator tworzoacy w petli kolejne procesory
            usleep(900000); // zwolnilem troche generowanie zebys widzial ruch
        }
        else {
            perror("Blad fork");
        }
    }

    return 0;
}