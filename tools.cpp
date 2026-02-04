#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "tools.h"
#include <errno.h>

// -- SEMAFORY --

// Tworzy semafory z użyciem flagi IPC_CREAT i praw dostępu 0600
int create_semaphores(int key, int number) {
    int id = semget(key, number, 0600 | IPC_CREAT);
    if (id == -1) {
        perror("Error creating semaphores!");
        exit(1);
    }
    return id;
}

// Usuwa semafory
void remove_semaphores(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("Error removing semaphores");
    }
}


// -- OPERACJE NA SEMAFORACH --

// Zmniejsza wartosc semafora o 1 (blokowanie)
// Jeśli semafor jest 0, proces zostaje zablokowany do czasu zwolnienia
void lock_sem(int sem_id, int sem_num) {
    struct sembuf op;
    op.sem_num = sem_num;
    op.sem_op = -1; // -1 oznacza blokowanie
    op.sem_flg = 0; // 0 oznacza operację blokującą

	// EIDRM - semafor został usunięty
    // EINVAL - semafor nie istnieje
	// W takich przypadkach wychodzimy z procesu
    if (semop(sem_id, &op, 1) == -1) {
        if (errno == EIDRM || errno == EINVAL) {
            exit(0);
        }
        perror("Error locking semaphore");
        exit(1);
    }
}

// Zwiększa wartosc semafora o 1 (odblokowanie)
// Jeśli są procesy oczekujące na semafor, jeden z nich zostaje odblokowany
void unlock_sem(int sem_id, int sem_num) {
    struct sembuf op;
    op.sem_num = sem_num;
    op.sem_op = 1; // 1 oznacza odblokowanie
    op.sem_flg = 0;

    if (semop(sem_id, &op, 1) == -1) {
        if (errno == EIDRM || errno == EINVAL) {
            exit(0);
        }
        perror("Error unlocking semaphore");
        exit(1);
    }
}

// Ustawia wartosc początkową semafora SETVAL
void set_sem_value(int sem_id, int sem_num, int val) {
    if (semctl(sem_id, sem_num, SETVAL, val) == -1) {
        perror("Error setting semaphore value");
        exit(1);
    }
}

// Pobiera aktualną wartość semafora GETVAL
// Funkcja zwraca wartość semafora
int get_sem_value(int sem_id, int sem_num) {
    int val = semctl(sem_id, sem_num, GETVAL);
    if (val == -1) {
        perror("Error getting semaphore value");
        exit(1);
    }
    return val;
}

// -- SHARED MEMORY --

// Tworzy segment pamieci dzielonej
int create_shared_memory(int key, int size) {
    int id = shmget(key, size, 0600 | IPC_CREAT);
    if (id == -1) {
        perror("Error creating shared memory");
        exit(1);
    }
    return id;
}

// Oznacza segment pamieci do usuniecia IPC_RMID
// Usuwa segment pamieci dzielonej
void remove_shared_memory(int shm_id) {
    shmctl(shm_id, IPC_RMID, NULL);
}

// Podlacza pamiec dzielona do przestrzeni adresowej procesu
// Zwraca wskaznik do pamieci dzielonej, ktora potem rzutujemy na strukture CaveState*
int* attach_memory(int shm_id) {
    void* addr = shmat(shm_id, NULL, 0);
    if (addr == (void*)-1) {
        perror("Error attaching memory");
        exit(1);
    }
    return (int*)addr;
}

// Odłącza pamiec dzielona od przestrzeni adresowej procesu
void detach_memory(int* addr) {
    shmdt(addr);
}

// -- KOLEJKA WIADOMOSCI --

// Tworzy kolejke komunikatow
int create_msg_queue(int key) {
    int id = msgget(key, 0600 | IPC_CREAT);
    if (id == -1) {
        perror("Error creating queue");
        exit(1);
    }
    return id;
}

// Usuwa kolejke komunikatow IPC_RMID
void remove_msg_queue(int msg_id) {
    msgctl(msg_id, IPC_RMID, NULL);
}

// WYSYLANIE I ODBIERANIE BILETOW
// Czekamy na wolne miejsce w kolejce (semafor SEM_QUEUE_SPACE)
void send_ticket(int msg_id, int sem_id, TicketMessage msg) {
    lock_sem(sem_id, SEM_QUEUE_SPACE); // czekaj na miejsce w kolejce

    int size = sizeof(msg) - sizeof(long); // rozmiar bez pola mtype

	// Wysylamy bilet do kolejki komunikatow
    if (msgsnd(msg_id, &msg, size, 0) == -1) {
        perror("Error sending ticket");
        unlock_sem(sem_id, SEM_QUEUE_SPACE);
        exit(1);
    }
}

// Odbieramy bilet z kolejki komunikatow
// Implementacja odbierania z priorytetem VIP
TicketMessage receive_ticket(int msg_id, int sem_id) {
    TicketMessage msg;
    int size = sizeof(msg) - sizeof(long);

    // msgrvc z typem -2 odbiera najpierw typ 1 (VIP), potem typ 2 (normalny)
	// vip zostanie wybrany jako pierwszy, jesli jest w kolejce, bo ma nizszy numer typu
    if (msgrcv(msg_id, &msg, size, -2, 0) == -1) {
        if (errno == EIDRM || errno == EINVAL) {
            exit(0);
        }
        perror("Error receiving ticket");
        exit(1);
    }
	// Po odebraniu biletu zwalniamy miejsce w kolejce
    unlock_sem(sem_id, SEM_QUEUE_SPACE);
    return msg;
}