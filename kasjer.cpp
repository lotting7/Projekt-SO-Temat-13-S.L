#include <iostream>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include "common.h"
#include "tools.h"

using namespace std;

int main() {
    cout << "KASJER - Rozpoczecie!" << endl;

    int msg_id = msgget(KEY_MSG, 0666);
    if (msg_id == -1) {
        perror("Blad: Brak kolejki!");
        return 1;
    }

    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0666);
    if (shm_id == -1) {
        perror("Blad: Brak pamieci!");
        return 1;
    }
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);

    cout << "Kasjer - dziala. Oczekiwanie na ./klient" << endl;

    while (true) {
        TicketMessage bilet = receive_ticket(msg_id);

        cout << "\n[KASJER] Klient ID: " << bilet.visitor_id << " (Wiek: " << bilet.age << ")" << endl;

        sleep(1);


        if (bilet.age < 3) {
            jaskinia->tickets_free++;
            cout << "         Wstep WOLNY (Dziecko < 3 lat)." << endl;
        }
        else {
            jaskinia->tickets_sold++;
            cout << "         Bilet SPRZEDANY." << endl;
        }


        cout << "RAZEM: Sprzedano: " << jaskinia->tickets_sold
             << " | Darmowe wejscia: " << jaskinia->tickets_free << endl;
    }

    detach_memory((int*)jaskinia);
    return 0;
}