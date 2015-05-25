#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <thread_listener.h>
#include <main_client.h>

void *thread_listener(void *connessione) {
    /*
        Il buffer attraverso cui passiamo i messaggi. La prima cosa che leggo
        è la lunghezza del messaggio, e poi il messaggio stesso.
        Alloco dinamicamente la dimensione del buffer per contenere il messaggio
     */
    char *buffer = malloc(sizeof(int));

    // Riferimento alla socket passato per argometno
    int socket_id = *(int*)connessione;

    // Lunghezza del prossimo campo da leggere dal socket
    int lunghezza_messaggio;
    while (read(socket_id, buffer, sizeof(int)) > 0) {
        // Leggiamo i primi 4 byte: la lunghezza del messaggio
        lunghezza_messaggio = atoi(buffer);

        if (lunghezza_messaggio > 0) {
            buffer = realloc(buffer, lunghezza_messaggio);
            if (read(socket_id, buffer, lunghezza_messaggio) != lunghezza_messaggio) {
                printf("WOPS 2\n");
            }

            if (lunghezza_messaggio == 7 && strcmp(buffer, "#logout") == 0) {
                printf("Il server remoto è stato spento, addio!\n");
                pthread_exit(NULL);
            }
            printf("%s\n", buffer);
        }

        bzero(buffer, lunghezza_messaggio);
    }

    pthread_exit(NULL);
}
