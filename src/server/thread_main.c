#include <stdio.h>
#include <sys/socket.h>

#include <thread_main.h>
#include <gestione_utenti.h>
/*
    Per la descrizione delle funzioni presenti in questo file consultare i
    rispettivi header, indicati da un commento all'inizio della funzione
    stessa
 */

/*
    La costante PROTOCOLLO indica con quale protocollo creare la socket.
    Accetta due valori:
    - AF_UNIX se il server e i client girano sullo stesso sistema
    - AF_INET per connettersi ad un server esterno
 */
#define PROTOCOLLO AF_INET

// main_server.h
int go;

// thread_main.h
void thread_main(char *file_utenti, char *file_log)
{
    /*
        La variabile rappresenta il riferimento alla socket che verrà aperta
        e sarà usata da altre funzioni riguardanti la socket
     */
    int socket_id;

    // Per prima cosa ci occupiamo di caricare gli utenti nella tabella hash
    // gestione_utenti.h
    carica_utenti(file_utenti);

    /*
        Abbiamo caricato gli utenti, ora proviamo a creare la socket.
        Salviamo il suo descrittore
     */
    socket_id = socket(PROTOCOLLO, SOCK_STREAM, 0);
    if (socket_id != -1) {
        /*
            La socket è stata creata con successo. Entriamo nel ciclo
            che continua fintanto che il programma è in esecuzione
         */
        while (go != 0) {
            // Rimaniamo in ascolto
            listen(socket_id, 3);
        }

    } else {
        // TODO: implementa errori
        printf("Impossibile creare la socket\n");
    }
}
