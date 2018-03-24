/**
 * Name:    Francesco
 * Surname: Longo
 * ID:      223428
 * Lab:     1
 * Ex:      3
 *
 * Implement the solution for Exercise 2 replacing system call execlp with system call execv.
 *
 * Exercise 2:
 * Write a C program that takes as arguments a number C and a directory name dir.
 * The main program, using the system call system, outputs in a file list.txt the list of files in directory dir.
 * Then it reads the content of the file list.txt, and for each read line (a filename) forks a child process,
 * which must sort the file by executing (through the execlp system call) the Unix sort program with the appropriate
 * arguments.
 * Notice that the command sort –n –o fname fname sorts in ascending order the content of fname, and by means of
 * the –o option rewrites the content of file fname with the sorted numbers. Option –n indicates numeric rather
 * than alphabetic ordering.
 * The main process can create a maximum of C children that sort different files in concurrency, to avoid overloading
 * the system. Then, it has to wait the termination of these children before reading the next filename from
 * file list.txt.
 * After all files listed in list.txt have been sorted, the main process must produce a single file all_sorted.txt,
 * where all the numbers appearing in all the sorted files are sorted in ascending order. Do this by using again
 * system call system with the appropriate command.
 * Take care of dealing with a number of files that is not a multiple of C, i.e., remember to wait for the last files
 * of the list.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <unistd.h>

#define N 50

void eraseString(char *string, int size);
char *Malloc(char *str);

int main(int argc, char **argv) {
    fprintf(stdout, "> Start\n");

    int C;
    char dirname[N];
    char *filename = "list.txt";

    FILE *fp, *fp1, *fp2;
    char line[N], line1[N];

    if (argc != 3) {
        fprintf(stdout, "Expected 3 argument: <prog_name> <C> <dirname>\n");
        exit(-1);
    }

    // take C and dirname from command line
    C = atoi(argv[1]);
    strcpy(dirname, argv[2]);
    fprintf(stdout, "> Input args: %d, %s\n", C, dirname);

    // initialize a string for command
    char cmd[N];
    strcpy(cmd, "ls ./");
    strcat(cmd, dirname);
    strcat(cmd, " > ");
    strcat(cmd, filename);

    if (system(cmd) == -1) {
        fprintf(stdout, "Error executing system\n");
        exit(-2);
    }
    fprintf(stdout, "> File %s created!\n", filename);

    if ((fp = fopen(filename, "rt")) == NULL) {
        fprintf(stdout, "Error creating file\n");
        exit(-3);
    }

    // count total files
    int totalFiles = 0;
    while (fgets(line, N, fp) != NULL) {
        totalFiles++;
    }
    fprintf(stdout, "> Total files: %d\n", totalFiles);

    // fp point to start of the file
    rewind(fp);

    char temp[50];
    int status, numChild = 0;

    // while a new file exists
    while (fgets(line, N, fp) != NULL) {
        // terminate read string line
        line[strlen(line)-1] = '\0';

        fprintf(stdout, "\n> File: %s\n", line);

        eraseString(temp, 50);

        sprintf(temp, "./%s/%s", dirname, line);

        fprintf(stdout, "%s", temp);

        // if I have more than max thread running
        if (numChild >= C) {
            // wait for a child termination
            wait(&status);
            numChild--;

            if (status != 0) {
                fprintf(stdout, "Error, child error 1\n");
                exit(-4);
            }
        }

        switch (fork()) {
            case 0:
                // child

                eraseString(temp, 50);

                sprintf(temp, "./%s/%s", dirname, line);

                fprintf(stdout, "\n > Child(%d): %s\n", getpid(), temp);

                char *arr[] = {"mySort", "-n", "-o", temp, temp, (char *)0};

                if (execv("/usr/bin/sort", arr) < 0) {
                    fprintf(stdout, "Error, execv error\n");
                    exit(-6);
                }
                break;

            case -1:
                // error

                fprintf(stdout, "Error, fork error\n");
                exit(-5);

            default:
                // father

                numChild++;
                break;
        }
    }

    while (numChild > 0) {
        // wait for all remaining childrens' termination
        wait(&status);
        numChild--;

        if (status != 0) {
            fprintf(stdout, "Error, child error 2\n");
            exit(-4);
        }
    }

    // fp point to start of the file
    rewind(fp);

    if ((fp2 = fopen("all_sorted.txt", "w+")) == NULL) {
        fprintf(stdout, "Error creating file\n");
        exit(-3);
    }

    // main create unique file
    while (fgets(line, N, fp) != NULL) {
        eraseString(temp, 50);

        // terminate read string line
        line[strlen(line)-1] = '\0';

        sprintf(temp, "./%s/%s", dirname, line);

        if ((fp1 = fopen(temp, "rt")) == NULL) {
            fprintf(stdout, "Error creating file\n");
            exit(-3);
        }

        while (fgets(line1, N, fp1) != NULL) {
            // terminate read string line
            line1[strlen(line1)-1] = '\0';
            fprintf(fp2, "%s\n", line1);
        }

        fclose(fp1);
    }

    fclose(fp2);
    fclose(fp);

    system("sort -n -o all_sorted.txt all_sorted.txt");

    return 0;
}

void eraseString(char *string, int size) {
    for (int i=0; i<size; i++) {
        string[i] = (char) '\0';
    }
}
