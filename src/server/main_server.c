/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <thread_main.h>
#include <thread_dispatcher.h>
#include <main_server.h>
#include <gestione_utenti.h>

#define PROTOCOLLO AF_INET
#define MSG_BRDCAST 'B'

// Indirizzo del server su cui è in esecuzione il programma server
#define SERVER_IP "127.0.0.1"

/*
   La define SERVER_PORT definisce la porta su cui ci aspettiamo che il server
   sia in ascolto

   XXX; se aggiornata qua, aggiornare anche la define in thread_main.c
*/
#define SERVER_PORT 57223

// main_server.h
sig_atomic_t go;
char *file_log;
char *file_utenti;

/*
    La funzione main prende in input gli elementi passati da linea di comando:
    se non sono esattamente due, come previsto, stampa un errore ed esce.

    Altrimenti, continua l'esecuzione: il programma si forka. Se ha successo,
    il padre esce e il figlio continua l'esecuzione.

    Se la fork fallisce stampa un errore ed esce
*/
int main(int argc, char *argv[])
{
    // La variabile memorizza un riferimento al thread principale
    pthread_t main_thread_id;

    /*
        Il gestore dei segnali deve occuparsi anche di resettare SA_RESTART
        quindi lo invochiamo con sigaction.
        Questa struct è necessaria per definire come chiamare sigaction
        signal_handler è la funzione che si occupa di gestire i Segnali,
        così come definita in main_server.h
     */
    struct sigaction setup_action;
    setup_action.sa_handler = signal_handler;
    sigaction(SIGINT,  &setup_action, 0);
    sigaction(SIGTERM, &setup_action, 0);

    /*
        argc deve essere uguale a 3, il primo parametro è il nome del comando,
        gli altri due parametri sono il nome del file degli utenti e il nome
        del file di log
     */
    if (argc != 3) {
        fprintf(stderr, "Il programma deve avere esattamente due parametri\n");
        return -1;
    }

    // Recuperiamo i file passati come argomento
    file_utenti = argv[1];
    file_log = argv[2];

    /*
        Il programma può partire, impostiamo il valore di go a un valore diverso
        da zero
     */
    go = 1;

    /*
        Il programma si forka: per distinguere il padre dal figlio prendo il
        valore di ritorno della funzione fork(): sarà -1 se la fork fallisce,
        altrimenti 0 per il figlio e un numero intero positivo non nullo per il
        padre (uguale al pid del figlio)
     */
    int pid = fork();

    if (pid == 0) {
        /*
            Primo branch dell'if: il pid è zero, quindi la fork è andata a buon
            fine e questo è il figlio, quindi possiamo lanciare thread main.
         */
        if (pthread_create(&main_thread_id, NULL, &thread_main, NULL) != 0) {
            /*
                Qualcosa è andato storto nella creazione del thread!
                Segnaliamo l'errore e usciamo
             */
            fprintf(stderr, "Impossibile creare il thread main\n");
            return -1;
        }

        // Attendiamo con pazienza la fine del thread
        pthread_join(main_thread_id, NULL);
    } else if (pid == -1) {
        /*
            Il pid è -1, la fork è fallita: registriamo l'errore e terminiamo
            il programma con un valore negativo
         */
        fprintf(stderr, "Impossibile lanciare il demone\n");
        return -1;
    }

    // Tutte le cose belle finiscono...
    return 0;
}

// main_server.h
void signal_handler(int signal_number)
{
    // Fermiamo il server
    go = 0;

    // gestione_utenti.h
    salva_hashtable();

    // Prepariamoci a una falsa connessione
    int socket_id;
    struct sockaddr_in fake;

    // Creiamo una socket
    socket_id = socket(PROTOCOLLO, SOCK_STREAM, 0);

    // Prepariamo la nostra sockaddr_in in una struttra dedicata
    fake.sin_family = PROTOCOLLO;
    fake.sin_addr.s_addr = inet_addr(SERVER_IP);
    fake.sin_port = htons(SERVER_PORT);

    // Sblocchiamo l'accept e poi chiudiamo la connessione
    connect(socket_id, (struct sockaddr *)&fake, sizeof(fake));
    close(socket_id);

    // Mandiamo il messaggio di logout a tutti i client connessi
    // thread_dispatcher.h
    inserisci("0007#logout", MSG_BRDCAST, -1);
}
