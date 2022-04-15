#define _POSIX_C_SOURCE 200112L // issock etc etc
#define decalageHoraire (+1)
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

struct stat sb;     // va stocker mask du fichier
DIR *d;             // le directory
struct dirent *dir; // struct qui permet d'avoir le nom

static int cmpstringp(const void *p1, const void *p2) // fonction du manuel qsort pour le tri alphabétique
{
    return strcmp(*(char *const *)p1, *(char *const *)p2);
}

// compte le nombre de fichier
int numberOfFiles()
{
    int i;
    for (i = 0; (dir = readdir(d)) != NULL; i++)
    {
    }
    rewinddir(d); // on rembobine, on aura encore besoin de dir pour remplir le tableau
    return i;
}

// va créer un tableau qui va contenir le nom de chaque fichier puis va le trier alphabétiquement
void fillTab(char *tableau[], int len)
{
    int count = 0;
    while ((dir = readdir(d)) != NULL) // readdir lit le prochain lien
    {
        tableau[count] = dir->d_name; // on met le nom dans le tableau
        count++;
    }
    qsort(tableau, len, sizeof(*tableau), cmpstringp); // https://linux.die.net/man/3/qsort va trier le tableau
}

void afficherwx(void) // on peut l'appliquer n'importe ou car sb est une variable globale
{
    S_IRUSR &sb.st_mode ? printf("r") : printf("-");
    S_IWUSR &sb.st_mode ? printf("w") : printf("-");
    S_IXUSR &sb.st_mode ? printf("x") : printf("-");
    S_IRGRP &sb.st_mode ? printf("r") : printf("-");
    S_IWGRP &sb.st_mode ? printf("w") : printf("-");
    S_IXGRP &sb.st_mode ? printf("x") : printf("-");
    S_IROTH &sb.st_mode ? printf("r") : printf("-");
    S_IWOTH &sb.st_mode ? printf("w") : printf("-");
    S_IXOTH &sb.st_mode ? printf("x") : printf("-");
}

// print les informations du tableau
void printTab(char *tab[], int len)
{
    char bufTime[26]; // stock les informations pour le temps
    for (int i = 0; i < len; i++)
    {
        char *buf = (char *)calloc(sb.st_size + 1, sizeof(char)); // on crée un tableau de char du mot en fonction de la taille du fichier
        lstat(tab[i], &sb);                                       // on met les informations du fichiers dans sb
                                                                  // problème ici si on traite les erreurs pas bon badass
        time_t a = sb.st_mtime;                                   // a -> informations temps de la derniere modification
        struct tm *timeInfo = gmtime(&a);                         // fonction qui permet de transformer un temps en une struct
        timeInfo->tm_hour += decalageHoraire;
        if (S_ISREG(sb.st_mode))
        {
            printf("-");
        }
        else if (S_ISDIR(sb.st_mode))
        {
            printf("d");
        }
        else if (S_ISLNK(sb.st_mode)) // lorsque c'est un lien alors on met le nom vers lequel le fichier pointe dans buf
        {
            readlink(tab[i], buf, sizeof(buf)); // readlink prend le nom du lien , le buffer a rempli , et la taille du buff
            printf("l");
        }
        else if (S_ISSOCK(sb.st_mode))
        {
            printf("s");
        }
        else if (S_ISFIFO(sb.st_mode))
        {
            printf("p");
        }
        else if (S_ISBLK(sb.st_mode))
        {
            printf("b");
        }
        else if (S_ISCHR(sb.st_mode))
        {
            printf("c");
        }
        afficherwx();
        strftime(bufTime, 26, "%F %H:%M", timeInfo); // permet de mettre le temps dans le buffer déja formaté comme ça on a plus qu'a l'appeler avec %s
        if (S_ISLNK(sb.st_mode))                     // si c'est un lien alors l'affichage change on doit faire pointer le lien vers le fichier
        {
            printf(" %2li %4i %4i %5li %s %s -> %s\n", sb.st_nlink, sb.st_uid, sb.st_gid, sb.st_size, bufTime, tab[i], buf);
        }
        else
        {
            printf(" %2li %4i %4i %5li %s %s\n", sb.st_nlink, sb.st_uid, sb.st_gid, sb.st_size, bufTime, tab[i]);
        }
        free(buf);
    }
}

int main(int argc, char *argv[])
{

    if (argc == 1) // si la personne rentre juste ./run
    {
        char *c = getcwd(NULL, 0); // on prend le répertoire ou il est dedans
        d = opendir(c);
        free(c);
    }
    else if (argc == 2) // si elle écrit explicitement la ou elle veut aller
    {
        d = opendir(argv[1]); // il faudra vérifier par la suite que ce n'est pas null cas de l'erreur
    }
    else
    {
        printf("exit argc");
        exit(EXIT_FAILURE);
    }
    int nbFile = numberOfFiles(); // détermine le nombre de fichier du répertoire
    char *tab[nbFile];            // crée un tableau du nombre d'élément
    fillTab(tab, nbFile);         // rempli le tableau et le tri
    printTab(tab, nbFile);        // affiche le tableau
    closedir(d);
}
