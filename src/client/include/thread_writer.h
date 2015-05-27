/*****************************************************************
 *  ChatFe - Progetto di Sistemi Operativi '14/'15 UniFe         *
 *                                                               *
 *  Riccardo Padovani (115509) riccardo@rpadovani.com            *
 *****************************************************************/
#ifndef THREAD_WRITER
#define THREAD_WRITER

/*
    Il thread writer si occupa di leggere da stdin l'input dell'utente,
    formattarlo in una maniera che sia capibile dal server e inviare il
    messaggio al server.
    La connessione è già stata creata dal main_client
 */
void *thread_writer(void *connessione);

/*
    Dato in input una stringa che identifica un tipo di messaggio, ritorna
    il tipo di messaggio cosi come sarà letto dal server
 */
char tipo_messaggio(char *tipo);

#endif
