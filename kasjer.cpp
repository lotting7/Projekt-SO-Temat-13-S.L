#include <iostream>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include "common.h"
#include "tools.h"

using namespace std;

int main() {

    // Komunikaty
    int msg_id = msgget(KEY_MSG, 0600); // pobranie istniejacej kolejki
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600); // pobranie istniejacej pamieci

    // sprawdzenie czy zasoby istnieja
    if (msg_id == -1 || shm_id == -1) {
        perror("KASJER BLAD: Nie moge pobrac ID kolejki lub pamieci (uruchom ./main)");
        return 1;
    }

    CaveState* jaskinia = (CaveState*)attach_memory(shm_id); // podlaczenie pamieci dzielonej

    // petla odbierania biletow
    while (true) {
        TicketMessage bilet = receive_ticket(msg_id);

        // logika opłat
        string typ_biletu = "NORMALNY"; // domyslnie normalny
        if (bilet.age < 3) {
            jaskinia->tickets_free++;
            typ_biletu = "DARMOWY (<3 lat)";
        }
        else if (bilet.is_repeater == 1) {
            jaskinia->tickets_sold++;
            typ_biletu = "PRIORYTET -50% (Powrot)";
        }
        else {
            jaskinia->tickets_sold++;
            typ_biletu = "NORMALNY";
        }

        // logika typu biletu (VIP/STD)
        string priorytet;
        if (bilet.mtype == 2) {
            priorytet = "VIP";
        } else {
            priorytet = "STD";
        }


        cout << "[SPRZEDAZ] "
             << "PID KLIENTA: " << bilet.visitor_id
             << " | Wiek: " << bilet.age
             << " | Trasa: " << bilet.route_choice
             << " | Typ: " << priorytet
             << " | Oplata: " << typ_biletu
             << endl;

        sleep(0);  // symulacja czasu przetwarzania
    }
    return 0;
}