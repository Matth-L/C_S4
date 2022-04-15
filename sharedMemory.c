#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

// on en a besoin pour qsort
int intComparator(const void *first, const void *second)
{
    int firstInt = *(const int *)first;
    int secondInt = *(const int *)second;
    return firstInt - secondInt;
}

//* struct qui contient ce que l'énoncé demande
typedef struct
{
    int count;
    int tab[100];
} str;

int checkPrime(int num)
{
    int count = 0;

    if (num == 1)
    {
        count = 1;
    }
    for (int i = 2; i <= num / 2; i++)
    {
        if (num % i == 0)
        {
            count = 1;
            break;
        }
    }
    return count;
}

// attachement au segment de mémoire partagé
void *shrmemory(int shmid)
{
    //* shmid,adresse spécifique a laquelle attacher,flag
    // NULL -> on laisse le systeme choisir l'adresse
    void *shr_mem = shmat(shmid, NULL, 0);
    if (shr_mem == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    return shr_mem;
}

//* rend un pointeur vers la struct
str *initStruct(void *shr_mem)
// déclaration de variable dedans et initialisation
{
    str *ptrStr = (str *)shr_mem; // on est obligé de crée un pointeur de notre struct vers la mém partagé
    ptrStr->count = 0;
    ptrStr->tab[100] = 0;
    return ptrStr;
}

void printTab(int tab[], int count)
{
    // vérification que chaque élément est plus petit que le précedent
    printf("\ntab parent:");
    for (int i = 0; i < count; i++)
    {
        if (tab[i] > tab[i + 1] && i < count - 1)
        {
            perror("something is wrong i can feel it ");
            exit(EXIT_FAILURE);
        }
        printf("%i,", tab[i]);
    }
    printf("\n");
}
int main(int argc, char *argv[])
{

    // taille d'un int et d'un tableau de 100 int
    size_t s = sizeof(int) + sizeof(int[100]);

    // ouvre un segment de mémoire partagé
    int shmid = shmget(IPC_PRIVATE, s, IPC_CREAT | 0600); // private -> partagé par aucun processus sauf child
    if (shmid == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // attachement au segment de memoire partagé
    void *shrmem = shrmemory(shmid);

    // initialisation de la struct
    str *ptrStr = initStruct(shrmem);

    // création de 10 processus enfant
    int status = 0;
    pid_t child, waitPid;
    for (int i = 0; i < 10; i++)
    {
        //* duplique le processus renvoie 0 dans child et le pid dans le parent
        child = fork();
        if (child == -1)
        // si l'enfant échoue on ne bloque pas le processus car on souhaite que 10 enfants soit lancés
        {
            perror("fork");
        }
        else if (child == 0)
        {
            // instruction pour l'enfant
            // génére 10 entiers aléatoires

            // init rng grace au pid
            srand(getpid());
            int rng, pos;
            for (int count = 0; count < 10; count++)
            {
                //? le meilleur moyen so far d'avoir un nb aléatoire car le systeme est trop rapide donc on peut pas utiliser time
                rng = rand() % 100 + 2;
                if (!checkPrime(rng)) // 0 si premier 1 sinon
                {
                    printf("rn : %i\n", rng);
                    status = 1; // on réutilise status car on la déja déclaré dans le parent
                    // regarde si le nombre n'est pas déja dans le tableau
                    for (pos = 0; pos < 100; pos++)
                    {
                        if (ptrStr->tab[pos] == rng)
                        {
                            status = 0;
                            break;
                        }
                    }

                    //* si on est la alors le nombre n'est pas dans le tableau car état est resté a 1
                    if (status)
                    {
                        printf("ajout nombre dans tableau\n");
                        // on le met a la position 1 car après le quick sort va le trier
                        ptrStr->tab[1] = rng;
                        // tri du tableau croissant uniquement utile si on rajoute un nouveau nombre
                        qsort(ptrStr->tab, 100, sizeof(int), intComparator);
                    }
                }
            }

            // détachement du tableau
            if (shmdt(shrmem) == -1)
            {
                perror("shmdt");
                exit(EXIT_FAILURE);
            }
            //* réussite de l'enfant
            exit(EXIT_SUCCESS);
        }
        // le parent ici n'a pas besoin de traiter le pid des enfant en attendant donc pas de else
    }
    //! dans le parent

    // attend que tous les enfants termine
    while (waitPid = wait(&status) > 0) // wait renvoie le pid du child en cas de succes et 0 sinon
        ;

    printTab(ptrStr->tab, 100);
    // détachement puis suppression
    if (shmdt(shrmem) == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    struct shmid_ds a;
    // rempli ce struct pour le vider par la suite
    //?on aurait du le déclarer plus haut
    if (shmctl(shmid, IPC_STAT, &a) == -1)
    {
        perror("shmtcl");
        exit(EXIT_FAILURE);
    }

    // supprime la mémoire partagé
    if (shmctl(shmid, IPC_RMID, &a) == -1)
    {
        perror("shmtcl");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}