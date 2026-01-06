#include <iostream>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include "common.h"
#include "tools.h"

using namespace std;

int main() {
    cout << "KASJER - Rozpoczecie" << endl;

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

        cout << "\n[KASJER] Klient ID: " << bilet.visitor_id << " (Wiek: " << bilet.age << ")" << endl;

        sleep(1);

        // 3. zapisywanie w pamiedzi dzielonej
        jaskinia->tickets_sold++;

        cout << "[KASJER] Bilet sprzedany! Razem sprzedano: " << jaskinia->tickets_sold << endl;
    }

    //
    detach_memory((int*)jaskinia);
    return 0;
}