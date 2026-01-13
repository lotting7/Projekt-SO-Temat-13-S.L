#include <iostream>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include "common.h"
#include "tools.h"

using namespace std;

int main() {
    cout << "KASJER - Rozpoczecie!" << endl;

    // komunikaty
    int msg_id = msgget(KEY_MSG, 0666);
    if (msg_id == -1) {
        perror("Blad: Brak kolejki!");
        return 1;
    }

    // 2. zapisywanie w pamieci ile sprzedanych zostało biletow
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0666);
    if (shm_id == -1) {
        perror("Blad: Brak pamieci!");
        return 1;
    }
    // wskaznik do struktury pamieci
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);

    cout << "Kasjer - dziala. Oczekiwanie na ./klient" << endl;

    while (true) {

        TicketMessage bilet = receive_ticket(msg_id);

        // wypisanie na ekranie
        string status;
        if (bilet.mtype == 2) {
            status = "[PRIORYTET - OMINAL KOLEJKE!]";
        } else {
            status = "[ZWYKLY]";
        }

        cout << "\n[KASJER] " << status << " Klient ID: " << bilet.visitor_id
             << " (Wiek: " << bilet.age << ", Trasa: " << bilet.route_choice << ")" << endl;

        sleep(0);

        // logika oplat
        if (bilet.age < 3) {
            jaskinia->tickets_free++;
            cout << "         Wstep WOLNY (Dziecko < 3 lat)." << endl;
        }
        else if (bilet.is_repeater == 1) {
            jaskinia->tickets_sold++;
            cout << "         Bilet ZNIZKOWY (-50% dla powracajacego)." << endl;
        }
        else {
            jaskinia->tickets_sold++;
            cout << "         Bilet SPRZEDANY (Cena normalna)." << endl;
        }

        cout << "RAZEM: Sprzedano: " << jaskinia->tickets_sold
             << " | Darmowe wejscia: " << jaskinia->tickets_free << endl;
    }

    detach_memory((int*)jaskinia);
    return 0;
}