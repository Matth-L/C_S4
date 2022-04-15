// ldd: -lcrypto
#include <openssl/md5.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int zeros(char *s, int n)
{
    int tailleMot = strlen(s);
    if (tailleMot < n)
    {
        return 0;
    }
    for (int i = 0; i < n; i++)
    {
        if (s[i] != '0')
        {
            return 0;
        }
    }
    return 1;
}

// = {0} => all the array is reset to zero (only works for zero!)
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

void bruteforce(int first, int step, int zero)
{
    int count = first;
    pid_t found;
    // il faut convertir count en string dans md5 hash
    char bufferCount[15];
    sprintf(bufferCount, "%d", count);
    char *motHach = md5hash(bufferCount);
    while (1)
    {
        if (zeros(motHach, zero))
        {
            found = getpid();
            break;
        }
        count += step;
        sprintf(bufferCount, "%d", count);
        motHach = md5hash(bufferCount);
    }
    sprintf(bufferCount, "found.%d", found);
    int idFile;
    if ((idFile = open(bufferCount, O_WRONLY | O_CREAT, 0700)) == -1) // a vérifier les permissions
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    char buf[6];
    sprintf(buf, "%d", count);
    if (write(idFile, buf, strlen(buf)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    if (close(idFile) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
int main(int argc, char *argv[])
{
    int i;
    pid_t pid;
    if (pid = fork())
    {
        i = 1;
    }
    else if (pid = fork())
    {
        i = 2;
    }
    else
    {
        pid = getpid();
        i = 3;
    }
    bruteforce(i, 10, 2);
    kill(pid, SIGKILL);
    exit(EXIT_SUCCESS);
}