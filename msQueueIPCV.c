#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define _XOPEN_SOURCE 700
#define SIZE_MSG 256

// on définit la struct qu'on va utiliser
struct msgbuf
{
    long mtype;
    char txt[SIZE_MSG];
};
// strcmp => 1 si différent 0 si égal

void receive(int msqid)
{
    // init struct
    struct msgbuf *rcv;
    rcv = (struct msgbuf *)malloc(SIZE_MSG);
    // (queueId,Size+1,type(0 = premier message),flags)
    if (msgrcv(msqid, rcv, sizeof(rcv) + 1, 0, IPC_NOWAIT | 0400) == -1) // obligé de mettre +1 pour le \0
    {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    printf("%s", rcv->txt);
    free(rcv);
}

void send(int msqid)
{
    // init struct
    struct msgbuf *msgp;
    msgp = (struct msgbuf *)malloc(SIZE_MSG + 1);
    msgp->mtype = 1;

    // boucle pour entrée de text
    char pos, chr;
    for (pos = 0; pos < SIZE_MSG; pos++)
    {
        chr = getchar();

        if (chr == '\n')
        {
            if (pos && msgp->txt[pos - 1] == '\n')
            {
                break;
            }
        }
        msgp->txt[pos] = chr;
    }
    // autorise uniquement l'écriture 0200 donc écriture par l'utilisateur
    if (msgsnd(msqid, msgp, strlen(msgp->txt) + 1, IPC_NOWAIT ) == -1)
    {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    free(msgp);
}

void stat(int msqid)
{ // init struct
    struct msqid_ds info;
    //(queueId,commande,struct msqid_ds)
    if (msgctl(msqid, IPC_STAT, &info) == -1) // IPC STAT renseigne buf
    {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    // c'est cbytes pour avoir la taille du mot
    printf("Nombre de message sur la file : %ld \nTaille : %li \n", info.msg_qnum, info.msg_cbytes);
}

void remve(int msqid)
{
    struct msqid_ds info;

    if (msgctl(msqid, IPC_RMID, &info) == -1)
    {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        perror("arg diff ");
        exit(EXIT_FAILURE);
    }
    key_t key = ftok("exo22.c", 'S');
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int msqid;
    // crée file de message, échoue sinon, file crée sans clef
    msqid = msgget(key, IPC_CREAT | 0600); // donne le read and write sans ça permission denied

    if (msqid == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if (!strcmp(argv[1], "snd"))
    {
        send(msqid);
    }
    else if (!strcmp(argv[1], "rcv"))
    {
        receive(msqid);
    }
    else if (!strcmp(argv[1], "stat"))
    {
        stat(msqid);
    }
    else if (!strcmp(argv[1], "rm"))
    {
        remve(msqid);
    }
    exit(EXIT_SUCCESS);
}
