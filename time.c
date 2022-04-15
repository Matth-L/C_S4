#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*-------------------------------------------------------------------------------------------
*******************************************************************************************
Enoncé
*******************************************************************************************
-------------------------------------------------------------------------------------------*/
typedef enum
{
    MON = 1,
    TUE = 2,
    WED = 3,
    THU = 4,
    FRI = 5,
    SAT = 6,
    SUN = 7
} dayname;

typedef enum
{
    JAN = 1,
    FEB = 2,
    MAR = 3,
    APR = 4,
    MAY = 5,
    JUN = 6,
    JUL = 7,
    AUG = 8,
    SEP = 9,
    OCT = 10,
    NOV = 11,
    DEC = 12
} monthname;

typedef struct
{
    dayname weekday : 3;
    unsigned int day : 5;
    monthname month : 4;
    int year : 20;
} date;

/*-------------------------------------------------------------------------------------------
*******************************************************************************************
1
*******************************************************************************************
-------------------------------------------------------------------------------------------*/
int oneYear = 60 * 60 * 24 * 365.25;
int oneDay = 60 * 60 * 24;

char *dayname_str(dayname day)
{
    switch (day)
    {
    case MON:
        return "Monday";
    case TUE:
        return "Tuesday";
    case WED:
        return "Wednesday";
    case THU:
        return "Thursday";
    case FRI:
        return "Friday";
    case SAT:
        return "Saturday";
    case SUN:
        return "Sunday";
    default:
        return "error";
    }
}

/*-------------------------------------------------------------------------------------------
*******************************************************************************************
2
*******************************************************************************************
-------------------------------------------------------------------------------------------*/
char *monthname_str(monthname month)
{
    switch (month)
    {
    case JAN:
        return "January";
    case FEB:
        return "February";
    case MAR:
        return "March";
    case APR:
        return "April";
    case MAY:
        return "May";
    case JUN:
        return "June";
    case JUL:
        return "July";
    case AUG:
        return "August";
    case SEP:
        return "September";
    case OCT:
        return "October";
    case NOV:
        return "November";
    case DEC:
        return "December";
    default:
        return "January"; // permet de gérer le cas temporaire de changement d'année
    }
}
/*-------------------------------------------------------------------------------------------
*******************************************************************************************
3
La fonction donne le jour de la semaine depuis l'epoch (donc que de 1 a 7)
l'epoch commence un jeudi


*******************************************************************************************
-------------------------------------------------------------------------------------------*/
dayname weekday(time_t when)
{
    unsigned long days = 4 + when / oneDay;

    return (((3 % 7) + (days % 7)) % 7) + 1;
}

/*-------------------------------------------------------------------------------------------
*******************************************************************************************
4
*******************************************************************************************
-------------------------------------------------------------------------------------------*/
int leapyear(unsigned int year) // 1 si bissextile 0 sinon
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

/*-------------------------------------------------------------------------------------------
            cl  bx
JANVIER	    31	31
FÉVRIER	    28	29
MARS	    31	31
AVRIL	    30	30
MAI	        31	31
JUIN	    30	30
JUILLET	    31	31
AOÛT	    31	31
SEPTEMBRE	30	30
OCTOBRE	    31	31
NOVEMBRE	30	30
DÉCEMBRE	31	31
-------------------------------------------------------------------------------------------*/
int day365(time_t sec)
{
    int j = 1; // on commence au jour 1 de l'année

    while (sec >= oneDay)
    {
        sec -= oneDay;
        j++;
    }
    return j;
}

void findMonthDay(int day, int year, int *j, int *m)
{
    int mois[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int moisBissextile[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int i; // on aura besoin de i+1 par la suite donc on l'initialise avant pour qu'il continue d'exister

    /*
    Fonctionnement: (elle explique également la condition)
    Ex: si day = 324 et on est en janvier (case 1) on enleve 31 il reste encore 293 jours a traiter
        si day = 17 et on est en mars (case 3) on enleve rien car 17<31 on est donc le 17 mars
        si day = 33 et on est en janvier (case 1) on enleve 31 et on est le 2 février car au prochain tour 2< 29 ou 28
    */
    if (leapyear(year)) // si l'année est bissextile
    {
        for (i = 0; i < 12 && moisBissextile[i] < day; i++)
        {
            day -= moisBissextile[i];
        }
    }
    else // sinon
    {
        for (i = 0; i < 12 && mois[i] < day; i++)
        {
            day -= mois[i];
        }
    }
    // permet avec une fonction de rendre 2 variables

    *m = i + 1; // on rajoute +1 à la fin car le tab va que de 0 a 11

    *j = day; // on rend également le jour
}

/*-------------------------------------------------------------------------------------------
*******************************************************************************************
5
un jour = 145 440 sec
une année = 31 536 000 sec
*******************************************************************************************
-------------------------------------------------------------------------------------------*/
date from_time(time_t when)
{
    time_t temps = when;
    int trueYear = 1970; // l'epoch commence en 1970
    int monthNum;        // va contenir le mois

    if ((temps / oneYear) > 1) // si le nombre de seconde dépasse une année
    {
        trueYear += temps / oneYear;          // on rajoute le nombre d'année
        temps -= oneYear * (temps / oneYear); // temps vaut maintenant un nombre inférieur à une année en sec
    }

    // nombre de jour sur 365
    int dayNum = day365(temps);

    findMonthDay(dayNum, trueYear, &dayNum, &monthNum); // la fonction donne le bon jour a dayNum et le bon mois à monthNum

    dayname d = weekday(when);
    monthname m = monthNum;

    date resultat = {
        .weekday = d,
        .day = dayNum,
        .month = m,
        .year = trueYear};

    return resultat;
}

/*-------------------------------------------------------------------------------------------
*******************************************************************************************
6
*******************************************************************************************
-------------------------------------------------------------------------------------------*/
int main(void)
{

    date b = from_time(1609459300); // Friday 1 January 2021 00:00:01
    printf("%s %i %s %i\n", dayname_str(b.weekday), b.day, monthname_str(b.month), b.year);
}