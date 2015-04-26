#include <stdio.h>
#include <unistd.h>     /* close() */
#include <arpa/inet.h>
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

/*
    La costnate SERVER_PORT definisce la porta su cui il server rimarrà in
    ascolto
 */
#define SERVER_PORT 8888

// main_server.h
int go;

// thread_main.h
int thread_main(char *file_utenti, char *file_log)
{
    /*
        La variabile rappresenta il riferimento alla socket che verrà aperta
        e sarà usata da altre funzioni riguardanti la socket
     */
    int socket_id;

    /*
        La variabile mantiene il riferimeno alle nuove connessioni in entrata
        e viene utilizzata per associarla a una socket
     */
    int connessione;

    /*
        La variabile mantiene il riferimento alle nuove socket create quando
        accetta una nuova connessione
     */
    int nuova_socket;

    /*
        La struct serve per configurare la connessione della socket del server
        che sarà bindata alla porta definita dalla costante SERVER_PORT
     */
    struct sockaddr_in server;

    struct sockaddr_in client;

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
            Prepariamo la nostra sockaddr_in in una struttra dedicata
         */
        server.sin_family = PROTOCOLLO;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(SERVER_PORT);

        /*
            Creiamo la bind per associare il nostro demone alla porta
         */
        if (bind(
                socket_id,
                (struct sockaddr *)&server,
                sizeof(server)
            ) < 0) {
            // TODO: gestione errori
            printf("Bind fallita\n");
            return -1;
        }

        /*
            La socket è stata creata con successo e bindata. Entriamo nel ciclo
            che continua fintanto che il programma è in esecuzione
         */
        while (go != 0) {
            // Rimaniamo in ascolto
            listen(socket_id, 3);

            // Accettiamo le connessioni entranti
            printf("Attendo connessioni...\n");

            connessione = sizeof(struct sockaddr_in);

            nuova_socket = accept(
                socket_id,
                (struct sockaddr *)&client,
                (socklen_t*)&connessione
            );

            if (nuova_socket < 0) {
                //TODO error
                printf("Fallimento nella creazione della connessione\n");
            } else {
            printf("FUCK YEAH\n");
                //close(connessione);
            }
        } // end while
    } else {
        // TODO: implementa errori
        printf("Impossibile creare la socket\n");
    }

    return 0;
}
