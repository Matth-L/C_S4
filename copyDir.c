#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utime.h>
#include <errno.h>
#include <string.h>
#define PATH_MAX 4096

struct stat sb;

void cp_mode(char *src, char *tgt) // répliquer permissions du fichier 1 sur le fichier 2
{
    if (stat(src, &sb) == -1)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    } // on met les info de la source dans le struct stat

    if (chmod(tgt, sb.st_mode) == -1) // chmod change les bits rwx... du fichier
    {
        perror("chmod");
        exit(EXIT_FAILURE);
    }
    if (chown(tgt, sb.st_uid, sb.st_gid) == -1) // change les permissions d'appartenance "ownership" cad uid et guid
    {
        perror("chown");
        exit(EXIT_FAILURE);
    }
    // il faut également copier l'heure d'accés
    struct utimbuf time;           // struct avec heurre d'accès et de modif
    time.actime = sb.st_atime;     // copie heure d'accès
    time.modtime = sb.st_mtime;    // copie heure de modif
    if ((utime(tgt, &time)) == -1) // modifie les date de tgt avec celle contenu dans la struct
    {
        perror("utime");
        exit(EXIT_FAILURE);
    }
}
void cp_file(char *src, char *tgt) // recopier le fichier 1 sur le tgt , si le fichier existe on ecrase
{
    char buf[256];
    int idSrc = open(src, O_RDONLY); // lit un fichier en lecture seule et prend son id
    if (idSrc == -1)                 // si open échoue pour la source
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    int idTgt = open(tgt, O_WRONLY | O_TRUNC | O_CREAT); // vide le fichier s'il existe , le crée sinon
    if (idTgt == -1)                                     // pour la target
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    int testRead = read(idSrc, buf, sizeof(buf)); // testRead donne le nombre d'octet lu en fonction de la taille du buf
    while (testRead > 0)                          // on teste uniquement si > 0 car peut etre plus petit que buf mais 0 = EOF et -1 erreur
    {
        if ((write(idTgt, buf, testRead) == -1)) // écrit de la taille qui a été lu
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if ((fsync(idTgt)) == -1) // on synchronise pour qu'il écrive dans le fichier
        {
            perror("fsync");
            exit(EXIT_FAILURE);
        }
        testRead = read(idSrc, buf, sizeof(buf));
    }
    // si read échoue
    if (testRead == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    // a partir de la le fichier est entierement copier
    cp_mode(src, tgt); // copie des permissions
    if (close(idSrc) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    if (close(idTgt) == -1) // on ferme le fichier on test aussi l'erreur
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}
char *getPath(char *directory) // permet d'avoir le chemin absolu nécessaire pour commencer
{
    char *cwd = getcwd(NULL, 0);
    ssize_t taille = strlen(cwd) + strlen(directory) + 1 + 1; // faut pas oublier des deux /0
    char *dir = (char *)calloc(taille, sizeof(char));         // va contenir le chemin
    strcpy(dir, cwd);                                         // copie le cwd dedans
                                                              // strcat concatène 2 chaines de characteres
    strcat(dir, "/");
    strcat(dir, directory);
    free(cwd);
    return dir;
}
char *changePath(char *oldPath, char *newFichier) // prend un ancien chemin et rajoute un fichier (meme explication)

{
    ssize_t taille = strlen(oldPath) + strlen(newFichier) + 1 + 1; // faut pas oublier des deux /0
    char *dir = (char *)calloc(taille, sizeof(char));
    strcpy(dir, oldPath); // strcat concatène 2 chaines de characteres
    strcat(dir, "/");
    strcat(dir, newFichier);
    return dir;
}
int count = 0; // compteur qui va permettre de savoir si cwd et de savoir il y a cb de dossier si on veut
void cp_dir(char *src, char *tgt)
{
    DIR *dTgt, *dSrc;
    struct dirent *dir;

    // test si le dossier source existe bien et le dossier target n'existe pas

    if ((dTgt = opendir(tgt)) != NULL) // il faut qu'il renvoie NULL car le dossier n'existe pas normalement
    {
        cp_mode(src, tgt);        // si le dossier existe copie les permissions et quitte
        if (closedir(dTgt) == -1) // s'il est ouvert il faut le fermer
        {
            perror("opendir");
            exit(EXIT_FAILURE);
        }
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    if ((dSrc = opendir(src)) == NULL) // si le dossier n'existe pas
    {
        printf("exit src");
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    printf("dans mkdir: %s\n", tgt);
    if (mkdir(tgt, 0000) == -1) // on crée le dossier les permissions  sont changé juste après (mais au cas ou on autorise rien )
    // le truc un peu spécial c'est qu'on donne juste au début le nom du dossier
    // puis le nom complet pour avoir accès au dossier
    // puis le nom a partir du dossier
    {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }
    cp_mode(src, tgt);
    while ((dir = readdir(dSrc)) != NULL) // tant qu'on lit dans le fichier source
    {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) // si ce n'est pas le fichier . ou .. alors on le copie
        {
        }
        else
        {
            // contrairement aux originaux ceux là change à chaque appel
            char *srcPath;
            char *tgtPath;
            srcPath = changePath(src, dir->d_name);
            tgtPath = changePath(tgt, dir->d_name);
            if (stat(srcPath, &sb) == -1) // on aurait pu utiliser lstat aussi mais pas demandé
            {
                perror("stat");
                exit(EXIT_FAILURE);
            }
            if (S_ISDIR(sb.st_mode)) // si on tombe sur un dossier alors il faut le copier donc récursion
            {
                printf("\ncp_dir\n\n");
                count++;                  // si c'est un fichier alors on a besoin que pour la prochaine récursion
                cp_dir(srcPath, tgtPath); // le nom passé en paramètre n'est qu'un dossier ça dérange pas avec mkdir
            }
            else
            {
                printf("\ncp_file\n\n");
                cp_file(srcPath, tgtPath);
                printf("fin copie des fichiers\n");
            }
            // si on arrive on a fini soit de copier le dossier ou le fichier dans tous les cas srcPath et tgtPath vont changer
            free(srcPath);
            free(tgtPath);
        }
    }
    closedir(dSrc);
    // on ne closedir pas dTgt car on demande qu'il soit null lors de son appel plus haut
    printf("fin cp_dir\n");
}

int main(int argc, char *argv[])
{
    if (argc != 3) // on test si l'utilisateur rentre le bon nombre de mots
    {
        exit(EXIT_FAILURE);
    }
    if (stat(argv[1], &sb) == -1) // on test si le fichier s'ouvre
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    if (S_ISDIR(sb.st_mode)) // on test si c'est un dir
    {
        cp_dir(argv[1], argv[2]);
    }
    else
    {
        cp_file(argv[1], argv[2]);
    }
    printf("il y a eu %i sous dossier de : %s\n", count, argv[1]);
    // il faut également libérer les variables globales
    exit(EXIT_SUCCESS);
}
