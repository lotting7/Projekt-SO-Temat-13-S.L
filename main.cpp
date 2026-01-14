#include <iostream>
#include "common.h"
#include "tools.h"

using namespace std;

int main() {
    cout << "Inicjalizacja Jaskini!" << endl;

    // 1. zasoby

    // pamięć dzielona
    int shm_id = create_shared_memory(KEY_SHM, sizeof(CaveState));

    int sem_id = create_semaphores(KEY_SEM, 4);

    // kolejka komunikatów (na bilety)
    int msg_id = create_msg_queue(KEY_MSG);

    cout << "Zasoby utworzone (ID): SHM=" << shm_id << ", SEM=" << sem_id << ", MSG=" << msg_id << endl;

    // 2. wartosci poczatkowe (semafory)

    set_sem_value(sem_id, 0, 1);             // 1 = otwarte (nikt nie korzysta z pamieci)
    set_sem_value(sem_id, 1, LIMIT_ROUTE_1); // Limit miejsc na trasie 1
    set_sem_value(sem_id, 2, LIMIT_ROUTE_2); // Limit miejsc na trasie 2

    // 100 ustawione, poniewaz limit mostu mozna sprawdzic recznie w kliencie i dzieki temu nie blokuje semafor powrotu
    set_sem_value(sem_id, 3, 100);

    // Zerujemy pamięć dzieloną
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);
    jaskinia->people_on_route1 = 0;
    jaskinia->people_on_route2 = 0;
    jaskinia->people_on_bridge = 0;
    jaskinia->bridge_direction = 0; //
    jaskinia->tickets_sold = 0;
    jaskinia->tickets_free = 0;

    // ustawiamy na otwarte
    jaskinia->is_open = 1;
    jaskinia->route1_open = 1;
    jaskinia->route2_open = 1;

    detach_memory((int*)jaskinia); // bez usuwania - odzielny plik jest od tego

    cout << "Semafory ustawione" << endl;
    cout << "./clean aby usunac wszystko" << endl;

    return 0;
}