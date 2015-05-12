#ifndef THREAD_WRITER
#define THREAD_WRITER

void *thread_writer(void *connessione);

/*
  Dato in input una stringa che identifica un tipo di messaggio, ritorna
  il tipo di messaggi cosi come sara letto dal server
 */
char tipo_messaggio(char *tipo);

#endif
