#include <iostream>
#include <unistd.h>
#include <cstdlib>  // do rand()
#include <ctime>    // do time()
#include <sys/msg.h>
#include "common.h"
#include "tools.h"

using namespace std;

int main() {
    cout << "--- GENERATOR KLIENTOW ---" << endl;
    
    // losowanie liczb
    srand(time(NULL));

    // ID
    int msg_id = msgget(KEY_MSG, 0666);
    if (msg_id == -1) {
        perror("Blad: Brak kolejki!");
        return 1;
    }

    // symulacja dla 5 klientow - narazie dla testu
    for (int i = 0; i < 5; i++) {
        
        TicketMessage bilet;

        bilet.mtype = MSG_TICKET; 
        
        // Dane losowe klienta
        bilet.visitor_id = getpid() + i; // id procesu + licznik
        bilet.age = rand() % 80;         // Wiek 0-79 lat
        bilet.route_choice = (rand() % 2) + 1; // Trasa 1 lub 2
        
        // sprawdzanie wieku
        if (bilet.age < 18) {
            bilet.has_guardian = 1; 
        } else {
            bilet.has_guardian = 0;
        }

        cout << "[KLIENT] Wysylam turyste (Lat: " << bilet.age << ") na trase " << bilet.route_choice << endl;

        // wysylanie
        send_ticket(msg_id, bilet);

        // sleep na 2 aby nie pushowalo wszystko naraz
        sleep(2); 
    }

    cout << "Gotowe." << endl;
    return 0;
}