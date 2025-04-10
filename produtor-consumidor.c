#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#define BUFFER_FILE "buffer.txt"
#define LOCK_FILE "buffer.txt.lock"

void cleanup(int sig) {
    remove(LOCK_FILE);
    printf("[PID %d] Lock removido e processo encerrado com sinal %d\n", getpid(), sig);
    exit(0);
}

void wait_for_lock() {
    int lock_fd;
    while ((lock_fd = open(LOCK_FILE, O_CREAT | O_EXCL, 0666)) == -1) {
        usleep(50000);
    }
    close(lock_fd);
}

void produtor() {
    srand(time(NULL) ^ getpid());

    while (1) {
        int numero = rand() % 100;

        wait_for_lock();

        FILE *file = fopen(BUFFER_FILE, "a");
        if (!file) {
            perror("Erro ao abrir buffer.txt");
            remove(LOCK_FILE);
            exit(1);
        }

        fprintf(file, "%d\n", numero);
        fclose(file);

        remove(LOCK_FILE);
        printf("[Produtor PID %d] %d\n", getpid(), numero);
        sleep((rand() % 3) + 1);
    }
}

void consumidor() {
    srand(time(NULL) ^ getpid());

    while (1) {
        wait_for_lock();

        FILE *file = fopen(BUFFER_FILE, "r");
        if (!file) {
            perror("Erro ao abrir buffer.txt");
            remove(LOCK_FILE);
            exit(1);
        }

        int numero, encontrado = 0;
        FILE *temp = fopen("temp.txt", "w");
        if (!temp) {
            perror("Erro ao criar temp.txt");
            fclose(file);
            remove(LOCK_FILE);
            exit(1);
        }

        while (fscanf(file, "%d", &numero) != EOF) {
            if (!encontrado) {
                printf("[Consumidor PID %d] %d\n", getpid(), numero);
                encontrado = 1;
            } else {
                fprintf(temp, "%d\n", numero);
            }
        }

        fclose(file);
        fclose(temp);

        remove(BUFFER_FILE);
        rename("temp.txt", BUFFER_FILE);
        remove(LOCK_FILE);

        sleep((rand() % 3) + 1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <n_produtores> <n_consumidores>\n", argv[0]);
        return 1;
    }

    int n_produtores = atoi(argv[1]);
    int n_consumidores = atoi(argv[2]);

    // Configura os sinais para limpeza do lock
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    for (int i = 0; i < n_produtores; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            produtor();
        }
    }

    for (int i = 0; i < n_consumidores; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            consumidor();
        }
    }

    // Processo pai apenas espera
    while (1) {
        pause(); // Espera sinal
    }

    return 0;
}
