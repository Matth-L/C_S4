#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
// ldd: -lcrypto
#include <openssl/md5.h>
#include <string.h>
#include <stdio.h>
#define _XOPEN_SOURCE 700

char hash[1 + 2 * MD5_DIGEST_LENGTH] = {0};

char *md5hash(char *str)
{
    unsigned char md5[MD5_DIGEST_LENGTH] = {0};
    MD5(str, strlen(str), md5);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(hash + 2 * i, "%02x", md5[i]);
    }
    return hash;
}

int zeros(char *s, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (s[i] != '0')
        {
            return 0;
        }
    }
    return 1;
}

void bruteforce(int first, int step, int zero, int pipeWrite) // on passe le pipe d'écriture
{
    unsigned long long i;
    char s[128];
    for (i = first;; i += step)
    {
        sprintf(s, "%llu", i);
        md5hash(s);
        if (zeros(hash, zero))
        {
            break;
        }
    }
    sprintf(s, "found.%i", getpid());
    printf("le fichier aura comme nom :  %s\n", s);
    // pas besoin du numéro de fichier car on a un pipe cette fois ci
    if (open(s, O_WRONLY | O_CREAT | O_TRUNC, 0600) == -1) // mais on a quand meme besoin de créer le fichier
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    printf("fichier crée | pid : %i\n", getpid());
    // on écrit dans pipeWrite le buffer de i
    if (write(pipeWrite, &i, sizeof(unsigned long long)) < sizeof(unsigned long long))
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    // ferme pipefd[1] l'écriture
    if (close(pipeWrite) == -1)
    {
        perror("close bruteforce");
        exit(EXIT_FAILURE);
    }
    exit(0);
}

int main(void)
{
    // init for pipe
    int pipefd[2]; // [0] -> read | [1] -> write
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    // init pour poll
    struct pollfd pfds[10]; //.fd | .events | .revents
    // init for fork
    pid_t child;
    pid_t grp = -1;
    int status, i;
    char path[128];
    unsigned long long winner;
    // fork 10 children
    for (i = 0; i < 10; i++)
    {
        child = fork();
        if (child == -1)
        {
            perror("fork");
            // don't exit since we have other children to manage
        }
        else if (child == 0)
        {
            // dans l'enfant
            // il ne souhaite qu'écrire donc on lui ferme la lecture
            if (close(pipefd[0]) == -1)
            {
                perror("close enfant");
                exit(EXIT_FAILURE);
            }
            //[0] -> fd | [1]-> events | [2] -> revents
            int pipechild[2];
            if (pipe(pipechild) == -1)
            {
                perror("pipe");
            }
            if (close(pipechild[0] == -1)) // on ferme la partie lecture du pipe
            {
                perror("close");
            }
            dup2(pipechild[1], pipefd[1]);      // connecte le pipe écriture de l'enfant au pipe du parent
            bruteforce(i, 10, 6, pipechild[1]); // on donne la lecture a bruteforce
            // dans ce cas la c'est un pipe pour tous les enfants qu'ils se partagent
        }
        else
        {
            // dans le parent
            pfds[i].fd = child;      // on donne a la struct le bon pid
            pfds[i].events = POLLIN; // indique que des données sont dispo en lecture
            if (grp == -1)
            {
                grp = child; // pour le premier enfant
            }
            if (setpgid(child, grp) == -1)
            {
                perror("setpgid");
            }
        }
    }
    // une fois que tous les enfants sont init on lui demande de fermé son pipe d'écriture
    if (close(pipefd[1]) == -1)
    {
        perror("close parent");
        exit(EXIT_FAILURE);
    }
    // -1 fais attendre infini tant que 5 processus contenu dans pfds ont pas fini
    if (poll(pfds, 5, -1) == -1)
    {
        perror("poll");
        exit(EXIT_FAILURE);
    }
    printf("fini d'attendre\n");
    // si on arrive la le poll a fonctionner
    for (i = 0; i < 10; i++)
    {
        // si le processus a fini les données alors on ne peut plus écrire dedans
        if (pfds[i].revents && POLLIN)
        {
            if (read(pipefd[0], &winner, sizeof(unsigned long long)) < sizeof(unsigned long long))
            {
                perror("read");
                exit(EXIT_FAILURE);
            }
            printf("%llu\n", winner);
        }
        printf("fin de traitement pour cette cas \n");
    }
    // kill all processes in the group
    kill(-grp, SIGKILL);
    if (close(pipefd[0]) == -1)
    {
        perror("close lecture fin ");
        exit(EXIT_FAILURE);
    }
    // Unlink sert enlever le fichier pas besoin car il demande de garder les fichiers
    // le kernel s'occupe de tuer les zombies seules
    exit(EXIT_SUCCESS);
}