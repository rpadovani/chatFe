/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <thread_main.h>
#include <thread_worker.h>
#include <thread_dispatcher.h>
#include <gestione_utenti.h>
#include <main_server.h>
#include <log.h>

/*
    La cdefine SERVER_PORT definisce la porta su cui il server rimarrà in
    ascolto.

    XXX; se aggiornata qua, aggiornare anche la define in main_server.c
 */
#define SERVER_PORT 57223

#define K 256

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
        che sarà bindata alla porta definita da SERVER_PORT

        La sockaddr_in client serve invece per gestire le connessioni in
        ingresso
     */
    struct sockaddr_in server;
    struct sockaddr_in client;

    // La variabile contiene l'id del thread che viene creato per il worker
    pthread_t tid;
    pthread_t tid_dispatcher;

    /*
        Il thread deve essere creato come detached, per farlo bisogna passare
        un attributo (che qua creiamo) alla funzione pthread_create
     */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Per prima cosa ci occupiamo di caricare gli utenti nella tabella hash
    // gestione_utenti.h
    carica_utenti();

    // Inizializziamo la tabella di log
    // log.h
    init_log();

    /*
        Abbiamo caricato gli utenti, ora proviamo a creare la socket.
        Salviamo il suo descrittore
     */
    socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1) {
        fprintf(stderr, "Impossibile creare la socket\n");
        // Senza socket non ha senso continuare
        pthread_exit(NULL);
    }

    // Prepariamo la nostra sockaddr
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);

    // Creiamo la bind per associare il nostro demone alla porta
    if (bind(socket_id, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Bind fallita\n");
        // Senza bind non ha senso continuare
        pthread_exit(NULL);
    }

    /*
        La socket è stata creata con successo e bindata. Creiamo il thread
        dispatcher ed entriamo nel ciclo che continua fintanto che il
        programma è in esecuzione
     */
    if (pthread_create(&tid_dispatcher, NULL, &thread_dispatcher, NULL)) {
         fprintf(stderr, "Fallimento nella creazione del thread dispatcher\n");
         // Senza thread_dispatcher non ha senso continuare
         pthread_exit(NULL);
    }

    while (go != 0) {
        // Rimaniamo in ascolto, massimo 5 connessioni pendenti
        listen(socket_id, 5);

        // Accettiamo le connessioni entranti
        connessione = sizeof(struct sockaddr_in);

        nuova_socket = accept(
            socket_id,
            (struct sockaddr *)&client,
            (socklen_t*)&connessione
        );

        if (nuova_socket < 0) {
            fprintf(stderr, "Fallimento nella creazione della connessione\n");
            /*
                La connessione può essere fallita per i motivi più svariati.
                Non ha comunque senso fermare il server per il fallimento
                nella connessione di un client, quindi semplicmente passiamo
                al prossimo giro
             */
            continue;
        }
        /*
            Abbiamo creato una nuova connessione, creaiamo un nuovo
            thread e passiamo la connessione.
            Se la funzione di creazione ritorna qualcosa diverso da zero
            vuol dire che la creazione del thread è fallita, quindi
            stampiamo un'errore
         */
        if (pthread_create(&tid, &attr, &thread_worker, (void*)&nuova_socket)) {
            fprintf(stderr, "Fallimento nella creazione del thread\n");
            // Chiudiamo la connessione
            close(connessione);
        }
    } // end while

    // Aspettiamo che il dispatcher torni a noi!
    pthread_join(tid_dispatcher, NULL);

    /*
        ...tutti quei momenti andranno perduti nel tempo come lacrime nella
        pioggia. È tempo di morire.
     */
    pthread_exit(NULL);
}
