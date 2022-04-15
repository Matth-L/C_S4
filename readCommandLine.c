#include <stdlib.h>
#include <stdio.h>
// argc -> nb d'élement
// argv -> mot découpé en tab les espace ne sont pas compté
// arge -> variable d'environnement

int test250(void)
{
    char *input = getenv("BASE"); // on prend la valeur de la variable d'environnement
    if (input != NULL)            // si la base est rentré
    {
        int base = strtol(input, NULL, 10); // on la convertit en entier
        if (base < 2 || base > 16)
        {
            printf("exit250\n");
            exit(250);
        }
        return base; // pas besoin de else si c'est oui ça quitte
    }
    else
    {
        return 10;
    }
}
void test252(int dividende)
{
    if (dividende == 0)
    {
        printf("exit252\n");
        exit(252);
    }
}
// traite le cas 5+6
void test253(int length)
{
    // on regarde si moins de 2 valeurs on été rentré avant un signe
    // si oui alors pas assez d'élément dans le tableau
    if ((length - 2) < 0)
    {
        printf("exit253\n");
        exit(253);
    }
}
// traite le cas ou plus de 128 caractères sont entrés
void test254(int len)
{
    if (len >= 129)
    {
        printf("exit254");
        exit(254);
    }
}
// traite le cas 54 6 + 5 (ou il y a trop de nombre par rapport au nombre de signe)
void test255(long *tab, int len)
{
    // on parcours le tableau de la case 1 à la derniere case et on regarde si c'est que des 0
    // on aurait pu faire en sorte que ça commence directement i= 1 mais car on print la pile c'est mieux
    for (int i = 0; i < len; i++)
    {

        printf("pile :%ld\n", tab[i]);
        if (tab[i] != 0 && i > 0) // si non alors le tableau possède encore des valeurs dedans donc exit
        {
            printf("exit255\n");
            exit(255);
        }
    }
    /*
        for (int i = 1; i < len; i++)
    {
        if (tab[i] != 0 ) // si non alors le tableau possède encore des valeurs dedans donc exit
        {
            exit(255);
        }
    }
    */
}

void depile(long *tab, char signe, int index)
{
    switch (signe)
    {
    case '+':
        tab[index - 2] += tab[index - 1];
        break;
    case '-':
        tab[index - 2] -= tab[index - 1];
        break;
    case '.': // le point est plus facile a gerer l'étoile est considérer comme "er"
        tab[index - 2] *= tab[index - 1];
        break;
    case '/':
        test252(tab[index - 1]); // quand on divise on vérifie que le dividende n'est pas 0
        tab[index - 2] /= tab[index - 1];
        break;
    }
    tab[index - 1] = 0; // on met a 0 la valeur qu'on a ajouté dans la case qui cumule les résultats
}

void fillTab(long *tab, char *mot[], int len, int base)
{
    char *endPtr;
    int i, k;
    for (i = 1, k = 0; i < len; i++, k++)
    {
        // remplit le tableau
        tab[k] = strtol(mot[i], &endPtr, base);
        if (endPtr[0] == '+' || endPtr[0] == '-' || endPtr[0] == '.' || endPtr[0] == '/') // si le pointeur est un signe
        {
            test253(k); // on vérifie qu'il y a assez d'élément pour dépiler
            depile(tab, endPtr[0], k);
            k -= 2; // on recule le tableau de 2 cases car on a piler ex : [5,6,+]->[11,0,0]
        }
        else if (endPtr[0] != 0) // si le pointeur existe et ce n'est pas 0 (donc autre chose qu'un entier ou un signe)
        {
            printf("exit251");
            exit(251);
        }
    }
}
int main(int argc, char *argv[])
{
    int base = test250(); // donne la base si elle existe
    long *pile = (long *)calloc(128, sizeof(long));
    fillTab(pile, argv, argc, base); // remplit le tableau
    test255(pile, argc);
    printf("res:%li\n", pile[0]);
    free(pile);
    exit(EXIT_SUCCESS);
}
