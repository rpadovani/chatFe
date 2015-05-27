/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <thread_listener.h>
#include <main_client.h>

// thread_listener.h
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

    // Rimaniamo a leggere i messaggi che ci arrivano dal socket
    while (read(socket_id, buffer, sizeof(int)) > 0) {
        // Leggiamo i primi 4 byte: la lunghezza del messaggio
        lunghezza_messaggio = atoi(buffer);

        // Se il messaggio non ha lunghezza, saltiamo la lettura
        if (lunghezza_messaggio <= 0) {
          continue;
        }

        /*
            Allochiamo lo spazio necessario sul buffer e aggiungiamo il
            terminatore, che il server non ci ha inviato
         */
        buffer = realloc(buffer, lunghezza_messaggio + 1);
        buffer[lunghezza_messaggio] = '\0';

        if (read(socket_id, buffer, lunghezza_messaggio) != lunghezza_messaggio) {
            fprintf(stderr, "Si è verificato un errore leggendo dal server\n");
        }

        // Se il messaggio che abbiamo ricevuto è di logout, usciamo
        if (lunghezza_messaggio == 7 && strcmp(buffer, "#logout") == 0) {
            printf("Il server remoto è stato spento, addio!\n");
            break;
        }
        printf("%s\n", buffer);

        // Resettiamo il buffer per il prossimo giro
        buffer= realloc(buffer, sizeof(int));
    }

    // Liberiamo il buffer prima di uscire
    free(buffer);
    pthread_exit(NULL);
}
