/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include <thread_main.h>
#include <gestione_utenti.h>
#include <main_server.h>
#include <lista.h>
#include <log.h>

// main_server.h
char *file_utenti;

// gestione_utenti.h
static hash_t HASH_TABLE;

// Evitiamo race condition
static pthread_mutex_t mutex;

/*
    Per gestire gli utenti connessi utilizziamo una lista che contiene gli
    username, creata e mantenuta con le funzioni indicate in lista.h.

    Teniamo anche una lista di tutti gli utenti registrati, sia quelli che si
    erano registrati prima di questa sessione, che quelli che si sono registrati
    in questa sessione.

    Ci servirà alla fine per scrivere facilmente tutti gli utenti sul file.
 */
static lista utenti_connessi;
static lista utenti_registrati;

// Un paio di messaggi standard
#define MSG_OK 'O'
#define MSG_ERROR 'E'

// gestione_utenti.h
void carica_utenti(void)
{
    // Struttura che utilizziamo per caricare i dati dal file
    hdata_t *utente;

    // Inizializziamo le lista degli utenti
    utenti_connessi = CREALISTA();
    utenti_registrati = CREALISTA();
    posizione ultimo_utente_registrato;

    // Inizializziamo la mutex
    pthread_mutex_init(&mutex, NULL);

    HASH_TABLE = CREAHASH();

    /*
        Controlliamo se il file esiste con la funzione access(). È disponibile
        solo sui sistemi Unix, ma le specifiche indicano chiaramente l'utilizzo
        di sistemi Linux.

        La funzione prende in input il file da controllare e uno dei seguenti
        parametri:
        - 0         Controlla se il file esiste
        - 2         Controlla se ci sono i permessi di scrittura
        - 4         Controlla i permessi di lettura
        - 6         Controlla sia i permessi di lettura che quelli di scrittura

        La funzione ritorna 0 se il file soddisfa i requisiti, -1 altrimenti.

        Nel caso il file non esista la funzione ha finito il suo compito,
        il file verrà creato in un secondo momento
    */

    if (access(file_utenti, 4) != 0) {
        // Niente file, abbiamo finito.
        return;
    }

    // Il file esiste, lo apriamo in lettura e lo assegniamo alla variabile file
    FILE *p_file = fopen(file_utenti, "r");

    /*
        Facciamo comunque un controllo se il file è stato caricato, potrebbe
        fallire per vari motivi, come per esempio mancanza di spazio in
        memoria
    */
    if (p_file == NULL) {
        fprintf(stderr, "Impossibile aprire il file degli utenti\n");
        // Non logghiamo l'errore perché il file di log non c'è ancora.
    }

    /*
        Il file è stato aperto, leggiamolo linea per linea.
        La dimensione della linea letta è massimo 770 caratteri,
        data da 256 (dimensione massima di ogni campo) * 3 più due
        caratteri separatori (:) più il terminatore di stringa (\0)
    */
    char linea[771];

    /*
        Leggiamo ogni linea del file, una alla volta, e la inseriamo
        nella variabile linea
    */
    while (fgets (linea, sizeof(linea), p_file)) {
         // Puntatore a struct che sarà usata per inserire la linea letta
        utente = (hdata_t *) malloc(sizeof(hdata_t));

        /*
            Dalla linea estraiamo i tre campi che ci interessano,
            delimitati dai due punti
        */
        utente->uname = strdup(strtok(linea, ":"));
        utente->fullname = strdup(strtok(NULL, ":"));
        /*
            Visto che la mail è l'ultimo campo della stringa vogliamo
            evitare che abbia la newline al suo interno
        */
        utente->email = strdup(strtok(NULL, ":\n"));

        /*
            Controlliamo di aver estratto tutti e tre i campi.
            Se non abbiamo tutti e tre i campi (quindi la linea è
            nel formato sbagliato), ignoriamo la linea e continuiamo
            con quella successiva
        */
        if (utente->uname != NULL &&
            utente->fullname != NULL &&
            utente->email != NULL) {
            /*
                Popoliamo la struct nuovo utente e inseriamola in hash
                con le funzioni che ci sono state fornite
            */
            utente->sockid   = -1;
            INSERISCIHASH(utente->uname, (void*) utente, HASH_TABLE);

            // Inseriamo l'utente anche nella lista degli utenti registrati
            ultimo_utente_registrato = ULTIMOLISTA(utenti_registrati);
            INSLISTA(utente->uname, &ultimo_utente_registrato);
        }
    }
    // Abbiamo fatto tutto, possiamo chiudere il file
    fclose (p_file);
}

// gestione_utenti.h
char login_utente(char *username, int socket_id)
{
    hdata_t *risultato_ricerca = NULL;

    /*
        Creiamo una mutex, il resto del codice soffre di race conditions
        Sia la tabella hash che l'inserimento nella lista degli utenti loggati
     */
    pthread_mutex_lock(&mutex);
    risultato_ricerca = CERCAHASH(username, HASH_TABLE);
    posizione ultimo_elemento = ULTIMOLISTA(utenti_connessi);

    // Controlliamo l'utente esista e NON sia loggato
    if (risultato_ricerca != NULL && risultato_ricerca->sockid == -1) {
        /*
            Inseriamo l'utente nella lista degli utenti connessi, nella hash
            table e logghiamo nel file di log il suo login
         */
        INSLISTA(username, &ultimo_elemento);
        risultato_ricerca->sockid = socket_id;
        //log.h
        log_login(username);
        pthread_mutex_unlock(&mutex);
        return MSG_OK;
    }

    // L'utente non esiste o non è loggato, ritorniamo un errore
    pthread_mutex_unlock(&mutex);
    return MSG_ERROR;
}

// gestione_utenti.h
char registrazione_utente(char *messaggio, int socket_id, char **username)
{
    hdata_t *utente;

    // Il messaggio è nel formato username:nome:mail, estraimo lo username
    *username = strdup(strtok(messaggio, ":"));

    pthread_mutex_lock(&mutex);
    // Lo username non deve essere registrato
    if (CERCAHASH(*username, HASH_TABLE) != NULL) {
        pthread_mutex_unlock(&mutex);
        return MSG_ERROR;
    }
    pthread_mutex_unlock(&mutex);

    // Prepariamo la struttura con gli elementi necessari
    utente = (hdata_t *) malloc(sizeof(hdata_t));
    utente->uname = strdup(*username);
    utente->fullname = strdup(strtok(NULL, ":"));
    utente->email = strdup(strtok(NULL, ":"));
    utente->sockid = socket_id;

    pthread_mutex_lock(&mutex);
    // Facciamo il login
    posizione ultimo_elemento = ULTIMOLISTA(utenti_connessi);
    INSLISTA(utente->uname, &ultimo_elemento);

    log_login(*username);

    posizione ultimo_utente_registrato = ULTIMOLISTA(utenti_registrati);
    INSLISTA(utente->uname, &ultimo_utente_registrato);

    // Inseriamo la struttura appena popolata.
    INSERISCIHASH(utente->uname, (void*) utente, HASH_TABLE);
    pthread_mutex_unlock(&mutex);

    return MSG_OK;
}

// gestione_utenti.h
void elenca_utenti_connessi(char *risposta)
{
    pthread_mutex_lock(&mutex);
    posizione elemento_lista = PRIMOLISTA(utenti_connessi);
    pthread_mutex_unlock(&mutex);

    // La variabile risposta è stata allocata prima di essere passata
    risposta[0] = '\0';

    // Il primo nome utente non ha il separatore all'inizio
    bool is_first = true;

    pthread_mutex_lock(&mutex);
    // Il while continua finché non arriviamo all'ultimo elemento
    while (PREDLISTA(elemento_lista) != ULTIMOLISTA(utenti_connessi)) {
      // Se non è il primo elemento prima inseriamo un : e poi lo username
      if (!is_first) {
          risposta = realloc(risposta, strlen(risposta) + 1);
          risposta = strcat(risposta, ":");
      } else {
          is_first = false;
      }

      risposta = realloc(risposta,
                        strlen(risposta) + strlen(elemento_lista->elemento));
      risposta = strcat(risposta, elemento_lista->elemento);
      elemento_lista = SUCCLISTA(elemento_lista);
    }
    pthread_mutex_unlock(&mutex);
}

// gestione_utenti.h
void logout_utente(char *username)
{
    hdata_t *risultato_ricerca = NULL;

    pthread_mutex_lock(&mutex);
    posizione elemento_lista = PRIMOLISTA(utenti_connessi);

    // Lo username deve essere nella hash table
    if ((risultato_ricerca = CERCAHASH(username, HASH_TABLE)) == NULL) {
        pthread_mutex_unlock(&mutex);
        fprintf(stderr, "Un utente non esistente si è disconnesso\n");
        return;
    }

    // log.h
    log_logout(username);

    risultato_ricerca->sockid = -1;

    // Cerchiamo l'utente nella lista degli utenti connessi ed eliminiamolo
    while (PREDLISTA(elemento_lista) != ULTIMOLISTA(utenti_connessi)) {
        if (strcmp(elemento_lista->elemento, username) == 0) {
          CANCLISTA(&elemento_lista);
          break;
        }
        elemento_lista = SUCCLISTA(elemento_lista);
    }
    pthread_mutex_unlock(&mutex);
}

// gestione_utenti.h
int esiste_utente_loggato(char *username)
{
    hdata_t *risultato_ricerca = NULL;

    pthread_mutex_lock(&mutex);
    if ((risultato_ricerca = CERCAHASH(username, HASH_TABLE)) == NULL) {
        pthread_mutex_unlock(&mutex);
        return 1;
    }

    if (risultato_ricerca->sockid == -1) {
        pthread_mutex_unlock(&mutex);
        return 1;
    }

    pthread_mutex_unlock(&mutex);
    return 0;
}

// gestione_utenti.h
int sockid_username(char *username)
{
    hdata_t *risultato_ricerca = NULL;
    pthread_mutex_lock(&mutex);
    if ((risultato_ricerca = CERCAHASH(username, HASH_TABLE)) == NULL) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    pthread_mutex_unlock(&mutex);
    return risultato_ricerca->sockid;
}

// gestione_utenti.h
void salva_hashtable(void)
{
    FILE *p_file = fopen(file_utenti, "w+");

    if (p_file == NULL) {
        fprintf(stderr, "Problemi con file utenti. Il file non sarà salvato\n");
        return;
    }

    pthread_mutex_lock(&mutex);
    /*
        Recuperiamo la lista degli utenti dalla lista utenti_registrati, poi
        con ogni username così trovato andiamo a recuperare dalla hast table
        tutti gli altri dati
     */
    posizione elemento_lista = PRIMOLISTA(utenti_registrati);
    hdata_t *risultato_ricerca = NULL;

    while (PREDLISTA(elemento_lista) != ULTIMOLISTA(utenti_registrati)) {
        if ((risultato_ricerca = CERCAHASH(elemento_lista->elemento, HASH_TABLE)) == NULL) {
            /*
                L'elemento non è presente nella hashtable, ignoriamolo e
                saltiamo al prossimo
             */
             continue;
        }

        fprintf(p_file, "%s:%s:%s\n",
            risultato_ricerca->uname,
            risultato_ricerca->fullname,
            risultato_ricerca->email
        );

        elemento_lista = SUCCLISTA(elemento_lista);
    }
    pthread_mutex_unlock(&mutex);
    fclose(p_file);
}
