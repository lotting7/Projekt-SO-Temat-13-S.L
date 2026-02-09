#include <iostream>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "common.h"
#include "tools.h"

using namespace std;

int main() {

    setvbuf(stdout, NULL, _IONBF, 0);

    // POBIERANIE ID ZASOBOW IPC
    // Pamiec dzielona jaskini i semafory sa potrzebne do aktualizacji stanu jaskini
    int msg_id = msgget(KEY_MSG, 0600);
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600);
    int sem_id = semget(KEY_SEM, SEM_COUNT, 0600);

    if (msg_id == -1 || shm_id == -1 || sem_id == -1) {
        perror("KASJER BLAD: Nie moge pobrac ID zasobow (uruchom ./main)");
        return 1;
    }

    // PODLACZENIE DO PAMIECI JASKINI
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);

    // GLOWNA PETLA KASJERA
    while (true) {

        // Odbior biletu z kolejki komunikatow
        TicketMessage bilet = receive_ticket(msg_id, sem_id);

        // Aktualizacja stanu jaskini w pamieci dzielonej
        lock_sem(sem_id, SEM_MUTEX);

        string typ_biletu = "NORMALNY";

        // Logika biletów
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

        unlock_sem(sem_id, SEM_MUTEX);

        // Wyświetlenie informacji o sprzedanym bilecie
        string priorytet;
        if (bilet.mtype == 1) {
            priorytet = "VIP";
        } else {
            priorytet = "STD";
        }

        // Log sprzedazy biletu
        cout << "[SPRZEDAZ] "
             << "PID: " << bilet.visitor_id
             << " | Wiek: " << bilet.age
             << " | Trasa: " << bilet.route_choice
             << " | Typ: " << priorytet
             << " | Oplata: " << typ_biletu
             << endl;
    }
    return 0;
}