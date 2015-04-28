#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <thread_main.h>
#include <thread_worker.h>
#include <gestione_utenti.h>
#include <main_server.h>

/*
    Per la descrizione delle funzioni presenti in questo file consultare i
    rispettivi header, indicati da un commento all'inizio della funzione
    stessa
 */

/*
    La define PROTOCOLLO indica con quale protocollo creare la socket.
    Accetta due valori:
    - AF_UNIX se il server e i client girano sullo stesso sistema
    - AF_INET per connettersi ad un server esterno
 */
#define PROTOCOLLO AF_INET

#define MAX_CLIENT 50

/*
    La cdefine SERVER_PORT definisce la porta su cui il server rimarrà in
    ascolto
 */
#define SERVER_PORT 8888

// main_server.h
int go;

// thread_main.h
void *thread_main(void *arg)
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

    // La variabile contiene l'id del thread che viene creato per il worker
    pthread_t tid;

    /*
        Il thread deve essere creato come detached, per farlo bisogna passare
        un attributo che qua creiamo alla funzione pthread_create
     */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Per prima cosa ci occupiamo di caricare gli utenti nella tabella hash
    // gestione_utenti.h
    carica_utenti();

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
            pthread_exit(NULL);
        }

        /*
            La socket è stata creata con successo e bindata. Entriamo nel ciclo
            che continua fintanto che il programma è in esecuzione
         */
        while (go != 0) {
            // Rimaniamo in ascolto
            listen(socket_id, 3);

            // Accettiamo le connessioni entranti
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
                /*
                    Abbiamo creato una nuova connessione, creaiamo un nuovo
                    thread e passiamo la connessione.
                    Se la funzione di creazione ritorna qualcosa diverso da zero
                    vuol dire che la creazione del thread è fallita, quindi
                    stampiamo un'errore
                 */
                if (pthread_create(
                                    &tid,
                                    &attr,
                                    &thread_worker,
                                    (void *)&nuova_socket
                                )
                    ) {
                    // TODO error
                    printf("Fallimento nella creazione del thread\n");
                    close(connessione);
                }
            }
        } // end while
    } else {
        // TODO: implementa errori
        printf("Impossibile creare la socket\n");
    }

    /*
        ...tutti quei momenti andranno perduti nel tempo come lacrime nella
        pioggia. È tempo di morire.
     */
    pthread_exit(NULL);
}
