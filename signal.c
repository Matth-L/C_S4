#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
int num;

void exit_on_success(int sig)
{
    static int count = 0; // compteur permettant de se rapprocher du nombre aléatoire
    count++;
    if (count < num)
    {
        printf("count : %i\n", count);
    }
    else
    {
        printf("fini %i\n", num);
        exit(0); // ça les rends zombies ici
    }
}
// genere le nombre aléatoire dans la doc pas d'erreur a traiter
int randNumber()
{

    pid_t pid = getpid();
    srand(pid);
    num = rand() % 11; // tire au hasard un entier entre 1 et 10
    printf("le pid : %i num : %i\n", pid, num);
    // code a expliquer
    struct sigaction sa;
    // on donne une fonction si jamais signal
    sa.sa_handler = exit_on_success;
    // si le signal est sigterm
    sigaction(SIGTERM, &sa, NULL);
    // on demande de refaire le signal a chaque fois
    sa.sa_flags = SA_RESTART;
    while (1)
    {
        pause();
    }
}

int main(int argc, char *argv[])
{
    errno = 0;
    pid_t child, grp, w;
    int i;
    int nbEnfant = 10;
    for (i = 0; i < nbEnfant; i++)
    {
        child = fork();
        if (child == -1)
        { // le fork échoue
            perror("fork");
        }
        else if (child == 0)
        { // programme effectué par l'enfant car le fork réussit
            randNumber();
        }
        else
        {
            if (i == 0)
            {
                grp = child;
            }
            if (setpgid(child, grp) == -1)
            {
                perror("setfgpid");
            }
        }
    }
    int childStat;
    sleep(1);
    while (1)
    {
        // w attend que un changement du pid du grp
        // fonctionne pas tous le temps voir waitID https://unix.stackexchange.com/questions/453426/use-waitpid-for-child-having-groupid-1
        kill(-grp, SIGTERM);
        sleep(1);
        w = waitpid(-grp, &childStat, WNOHANG); // car il y a WNOHANG c'est 0 tant que pas de changement de pid et -1 sinon
        // il faut mettre -grp (a voir les diapo pour comprendre )
        if (w == -1)
        {
            if (errno == ECHILD) // ECHILD c'est quand + d'enfant a attendre
            {
                printf("success\n");
                exit(EXIT_SUCCESS);
            }
        }
        else if (WEXITSTATUS(childStat))
        {
            // child failed
            printf("child  \n");
        }
        else if (w != 0) // un enfant a fini dans faut envoyer sigkill pour child
        {
        }
    }
    exit(0);

}
