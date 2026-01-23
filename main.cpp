#include <iostream>
#include <unistd.h>     // fork, exec, sleep, getpid, dup2
#include <sys/wait.h>   // wait, waitpid
#include <signal.h>     // obsluga sygnalow
#include <fcntl.h>      // open, O_CREAT, O_TRUNC - do obslugi plikow
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "common.h"
#include "tools.h"

using namespace std;

// PID procesow potomnych
pid_t pid_kasjer = -1;
pid_t pid_przewodnik = -1;
pid_t pid_klient = -1;

// FUNKCJA OBSLUGI SYGNALU SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    cout << "\n\n[MANAGER] Otrzymano sygnal konca. Zamykanie symulacji..." << endl;

    // wysylamy sygnal SIGINT do procesow potomnych
    if (pid_klient > 0) kill(pid_klient, SIGINT);
    if (pid_kasjer > 0) kill(pid_kasjer, SIGINT);
    if (pid_przewodnik > 0) kill(pid_przewodnik, SIGINT);

    // czekamy na zakonczenie procesow potomnych
    sleep(1);

    // usuwanie zasobow IPC
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600);
    if (shm_id != -1) {
        remove_shared_memory(shm_id);
        cout << "[MANAGER] Pamiec usunieta." << endl;
    }

    int sem_id = semget(KEY_SEM, 4, 0600);
    if (sem_id != -1) {
        remove_semaphores(sem_id);
        cout << "[MANAGER] Semafory usuniete." << endl;
    }

    int msg_id = msgget(KEY_MSG, 0600);
    if (msg_id != -1) {
        remove_msg_queue(msg_id);
        cout << "[MANAGER] Kolejka usunieta." << endl;
    }

    cout << "[MANAGER] Wszystko wyczyszczone. Logi znajduja sie w plikach .txt" << endl;
    exit(0);
}

int main() {
    // ustawienie buforowania stdout na brak (od razu wyswietla komunikaty)
    signal(SIGINT, handle_sigint);

    cout << BOLD << "--- INICJALIZACJA SYSTEMU JASKINI ---" << RESET << endl;

    // TWORZENIE ZASOBOW IPC

    // pamiec dzielona
    int shm_id = create_shared_memory(KEY_SHM, sizeof(CaveState));

    // semafory
    int sem_id = create_semaphores(KEY_SEM, 5);

    // kolejka komunikatow
    int msg_id = create_msg_queue(KEY_MSG);

    cout << "[MANAGER] Zasoby utworzone. ID: SHM=" << shm_id << ", SEM=" << sem_id << ", MSG=" << msg_id << endl;

    // ustawienie wartosci poczatkowych semaforow
    set_sem_value(sem_id, 0, 1);             // mutex do pamieci dzielonej
    set_sem_value(sem_id, 1, LIMIT_ROUTE_1); // limit Trasy 1
    set_sem_value(sem_id, 2, LIMIT_ROUTE_2); // limit Trasy 2
    set_sem_value(sem_id, 3, 100);           // limit Mostu (kontrolowany recznie w kodzie klienta)
    set_sem_value(sem_id, 4, 1);             // 1 - mozna zapisac do logu, 0 - czekaj


    // zerowanie i ustawianie pamieci dzielonej
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);
    jaskinia->people_on_route1 = 0;
    jaskinia->people_on_route2 = 0;
    jaskinia->people_on_bridge = 0;
    jaskinia->bridge_direction = 0;
    jaskinia->tickets_sold = 0;
    jaskinia->tickets_free = 0;

    // otwarcie jaskini na start
    jaskinia->is_open = 1;
    jaskinia->route1_open = 1;
    jaskinia->route2_open = 1;
    jaskinia->pid_manager = getpid(); // zapisanie PID managera do pamieci

    detach_memory((int*)jaskinia);

    cout << "[MANAGER] Konfiguracja ukonczona. Uruchamiam procesy potomne..." << endl;
    sleep(2); // maly pause przed uruchomieniem procesow


    //URUCHAMIANIE PROCESOW (FORK + EXEC)

    //PROCES 1: KASJER (Logi do pliku raport_kasjer.txt)
    pid_kasjer = fork();
    if (pid_kasjer == 0) {
        // Kod Dziecka (Kasjer)

        // przekierowanie wyjscia do pliku
        int log_fd = open("raport_kasjer.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (log_fd == -1) { perror("Blad tworzenia logu kasjera"); exit(1); }
        dup2(log_fd, STDOUT_FILENO); // przekierowanie stdout
        close(log_fd);

        // wykonanie programu kasjer
        execlp("./kasjer", "kasjer", NULL);

        // jesli execlp zwroci blad
        perror("Blad exec kasjer");
        exit(1);
    }

    // PROCES 2: KLIENT/GENERATOR (Logi do pliku raport_ruch.txt)
    pid_klient = fork();
    if (pid_klient == 0) {
        // Kod Dziecka (Klient)

        // przekierowanie wyjscia do pliku
        int log_fd = open("raport_ruch.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (log_fd == -1) { perror("Blad tworzenia logu ruchu"); exit(1); }
        dup2(log_fd, STDOUT_FILENO);
        close(log_fd);


        sleep(1); // maly pause, zeby kasjer zdazyl sie uruchomic przed klientem
        execlp("./klient", "klient", NULL);
        perror("Blad exec klient");
        exit(1);
    }

    // PROCES 3: PRZEWODNIK - wyswietlanie na glownym ekranie
    pid_przewodnik = fork();
    if (pid_przewodnik == 0) {
        // Kod Dziecka (Przewodnik)
        execlp("./przewodnik", "przewodnik", NULL);
        perror("Blad exec przewodnik");
        exit(1);
    }


    // Oczekiwanie managera na zakonczenie procesow potomnych

    // manager czeka, az generator zakonczy prace
    waitpid(pid_klient, NULL, 0);


    sleep(10); // czekamy 10s na zakonczenie sie wszystkich turystow w jaskini

    // Koniec symulacji - sprzatanie
    handle_sigint(0);

    return 0;
}