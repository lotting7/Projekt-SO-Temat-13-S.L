#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <cstdlib>
#include "common.h"
#include "tools.h"

using namespace std;

// ZMIENNE GLOBALNE
pid_t pid_kasjer = -1;
pid_t pid_przewodnik = -1;
pid_t pid_klient = -1;

// Funkcja do czyszczenia zasobów IPC
void cleanup_resources() {

	// Usuwanie pamieci dzielonej
    int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600);
    if (shm_id != -1) {
        remove_shared_memory(shm_id);
        cout << "[MANAGER] Pamiec usunieta." << endl;
    }

	// Usuwanie semaforów
    int sem_id = semget(KEY_SEM, SEM_COUNT, 0600);
    if (sem_id != -1) {
        remove_semaphores(sem_id);
        cout << "[MANAGER] Semafory usuniete." << endl;
    }

	// Usuwanie kolejki komunikatów
    int msg_id = msgget(KEY_MSG, 0600);
    if (msg_id != -1) {
        remove_msg_queue(msg_id);
        cout << "[MANAGER] Kolejka usunieta." << endl;
    }
}
// Obsługa sygnału SIGINT (Ctrl+C) do zamknięcia jaskini
void handle_sigint(int sig) {
    cout << "\n\n[MANAGER] Otrzymano SIGINT. Zamykanie..." << endl;

	// Wysylanie sygnalu SIGINT do procesow jaskini
    if (pid_klient > 0) kill(pid_klient, SIGINT);
    if (pid_kasjer > 0) kill(pid_kasjer, SIGINT);
    if (pid_przewodnik > 0) kill(pid_przewodnik, SIGINT);

	// Zabijanie straznika jesli dziala
   int shm_id = shmget(KEY_SHM, sizeof(CaveState), 0600);
    if (shm_id != -1) {
        CaveState* jaskinia = (CaveState*)attach_memory(shm_id);

        if (jaskinia->pid_straznik > 0) {
            cout << "[MANAGER] Zabijam Straznika (PID: " << jaskinia->pid_straznik << ")" << endl;
            kill(jaskinia->pid_straznik, SIGKILL); // Strażnik dostaje twardy reset
        }

        detach_memory((int*)jaskinia);
}
    sleep(1);

    cleanup_resources();
    cout << "[MANAGER] Gotowe." << endl;
    exit(0);
}

int main() {
	// Rejstracja obsługi (CTRL+C)
    signal(SIGINT, handle_sigint);

    cout << BOLD << "--- INICJALIZACJA SYSTEMU JASKINI ---" << RESET << endl;

	// Tworzenie zasobów IPC
    int shm_id = create_shared_memory(KEY_SHM, sizeof(CaveState));
    int sem_id = create_semaphores(KEY_SEM, SEM_COUNT);
    int msg_id = create_msg_queue(KEY_MSG);

    cout << "[MANAGER] Zasoby: SHM=" << shm_id << ", SEM=" << sem_id << ", MSG=" << msg_id << endl;

	// KONFIGURACJA SEMAFOROW
    set_sem_value(sem_id, SEM_MUTEX, 1); // mutex otwarty

	// Ustawienie limitów miejsc na trasach i moście
    set_sem_value(sem_id, SEM_ROUTE1, LIMIT_ROUTE_1);
    set_sem_value(sem_id, SEM_ROUTE2, LIMIT_ROUTE_2);
    set_sem_value(sem_id, SEM_BRIDGE, LIMIT_BRIDGE);

	// Logi
    set_sem_value(sem_id, SEM_LOG, 1);

	// Sterowanie mostem
    set_sem_value(sem_id, SEM_BRIDGE_ENTRY, 0);
    set_sem_value(sem_id, SEM_BRIDGE_EXIT, 0);

	// Miejsca w kolejce komunikatów
    set_sem_value(sem_id, SEM_QUEUE_SPACE, MSG_QUEUE_MAX);

	// INICJALIZACJA PAMIECI DZIELONEJ JASKINI
    CaveState* jaskinia = (CaveState*)attach_memory(shm_id);

	// Ustawienie początkowego stanu jaskini
    jaskinia->people_on_route1 = 0;
    jaskinia->people_on_route2 = 0;
    jaskinia->people_on_bridge = 0;
    jaskinia->bridge_direction = 0;
    jaskinia->bridge_waiting_in = 0;
    jaskinia->bridge_waiting_out = 0;
    jaskinia->tickets_sold = 0;
    jaskinia->tickets_free = 0;

	// Jaskinia otwarta, obie trasy czynne
    jaskinia->is_open = 1;
    jaskinia->route1_open = 1;
    jaskinia->route2_open = 1;

	// PIDy procesów jaskini
    jaskinia->pid_manager = getpid();
	jaskinia->pid_straznik = 0;
    jaskinia->pid_kasjer = 0;
    jaskinia->pid_klient = 0;

    detach_memory((int*)jaskinia);

    cout << "[MANAGER] Uruchamiam procesy..." << endl;

	// URUCHAMIANIE PROCESÓW JASKINI

    // KASJER
    pid_kasjer = fork();
    if (pid_kasjer == 0) {
        int log_fd = open("raport_kasjer.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (log_fd == -1) { perror("Blad logu kasjera"); exit(1); }
        dup2(log_fd, STDOUT_FILENO);
        close(log_fd);
        execlp("./kasjer", "kasjer", NULL);
        perror("Blad exec kasjer");
        exit(1);
    }

    // KLIENT
    pid_klient = fork();
    if (pid_klient == 0) {
        int log_fd = open("raport_ruch.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (log_fd == -1) { perror("Blad logu ruchu"); exit(1); }
        dup2(log_fd, STDOUT_FILENO);
        close(log_fd);
        execlp("./klient", "klient", NULL);
        perror("Blad exec klient");
        exit(1);
    }

    // PRZEWODNIK
    pid_przewodnik = fork();
    if (pid_przewodnik == 0) {
        execlp("./przewodnik", "przewodnik", NULL);
        perror("Blad exec przewodnik");
        exit(1);
    }

	// OCZEKIWANIE NA KONIEC SYMULACJI

	// Czekamy na zakończenie procesu klienta
    waitpid(pid_klient, NULL, 0);

	// Sprawdzamy czy w jaskini nie ma już turystów
    int shm_check = shmget(KEY_SHM, sizeof(CaveState), 0600);
    CaveState* jask = (CaveState*)attach_memory(shm_check);

    int timeout = 0;
    const int MAX_TIMEOUT = 500000;

    while (timeout < MAX_TIMEOUT) {

		int shm_temp_id = shmget(KEY_SHM, sizeof(CaveState), 0600);
    	CaveState* temp_jaskinia = (CaveState*)attach_memory(shm_temp_id);

        lock_sem(sem_id, SEM_MUTEX);
        int t1 = jask->people_on_route1;
        int t2 = jask->people_on_route2;
        int most = jask->people_on_bridge;
        unlock_sem(sem_id, SEM_MUTEX);

        if (t1 == 0 && t2 == 0 && most == 0) {
            break;
        }

        timeout++;

    }

    detach_memory((int*)jask);

    sleep(2);

    // Zamykamy przewodnika
    if (pid_przewodnik > 0) {
        kill(pid_przewodnik, SIGINT);
        waitpid(pid_przewodnik, NULL, 0);
    }

    // Zamykamy kasjera
    if (pid_kasjer > 0) {
        kill(pid_kasjer, SIGINT);
        waitpid(pid_kasjer, NULL, 0);
    }

    // Zabijamy straznika jesli dziala
    system("pkill -9 -f './straznik'");

    cout << "\n[MANAGER] Symulacja zakonczona. Sprzatanie zasobow..." << endl;

    cleanup_resources();

    cout << "[MANAGER] Gotowe. Logi w plikach raport_*.txt" << endl;

    return 0;
}