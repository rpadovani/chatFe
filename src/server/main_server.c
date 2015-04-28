#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <thread_main.h>

// main_server.h
int go;
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
    /*
        La variabile memorizza un riferimento al thread principale
     */
    pthread_t main_thread;

    /*
        argc deve essere uguale a 3, il primo parametro è il nome del comando,
        gli altri due parametri sono il nome del file degli utenti e il nome
        del file di log
     */
    if (argc != 3) {
        fprintf(stderr, "Il programma deve avere esattamente due parametri\n");
        return -1;
    }

    file_utenti = argv[2];
    file_log = argv[3];

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
        int codice_ritorno = pthread_create(
                                                &main_thread,
                                                NULL,
                                                &thread_main,
                                                NULL
                                            );

        /*
            Qualcosa è andato storto nella creazione del thread!
            Segnaliamo l'errore e moriamo
         */
        if (codice_ritorno) {
            fprintf(stderr, "Impossibile creare il thread main\n");
            return -1;
        }

        pthread_join(main_thread, NULL);
    } else if (pid == -1) {
        /*
            Il pid è -1, la fork è fallita: registriamo l'errore e terminiamo
            il programma con un valore negativo
         */
        fprintf(stderr, "Impossibile lanciare il demone\n");
        return -1;
    }

    return 0;
}
/*

int main()

    fork()

    if figlio

        thread main()
            while go
                apri socket
                inizializza tabella

                if connessione
                    lancia worker


        thread worker()
            while go
                attendi richiesta
                registra il comando nel log file

                se comando == registrazione || listing
                    esegui
                altrimenti
                    scrivi in buffer


        thread dispatcher
            while go
                leggi buffer
                estrai destinatario

                se broadcast
                    invia a tutti
                altrimenti se connesso
                    invia messaggio a utente
                altrimenti
                    scarta

    else if error
        std error

    return
*/
