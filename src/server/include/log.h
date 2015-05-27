#ifndef LOG_H
#define LOG_H

void init_log(void);
void timestamp(char *ts);
void log_login(char *username);
void log_logout(char *username);
void log_messaggio_singolo(char *mittente, char *destinatario, char *messaggio);
void log_messaggio_broadcast(char *mittente, char *messaggio);

#endif
