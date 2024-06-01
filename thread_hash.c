// ############################################################################
// #
// #    Author:      Lorenzo D. Moon
// #    Instructor:  R. Jesse Chaney
// #    Course:      CS-333
// #    Assignment:  Lab 4: Threads from the Crypt
// #    Description:
// #
// ############################################################################

/* thread_hash.h
# define OPTIONS "i:o:d:hvt:n"
# define MICROSECONDS_PER_SECOND 1000000.0
# ifndef NICE_VALUE : define NICE_VALUE 10

// This comes from a terrific Stackover flow posting.
https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
# define FOREACH_ALGORITHM(ALGORITHM) \
        ALGORITHM(DES)           \
        ALGORITHM(NT)            \
        ALGORITHM(MD5)           \
        ALGORITHM(SHA256)        \
        ALGORITHM(SHA512)        \
        ALGORITHM(YESCRYPT)      \
        ALGORITHM(GOST_YESCRYPT) \
        ALGORITHM(BCRYPT)        \
        ALGORITHM(ALGORITHM_MAX)

# define GENERATE_ENUM(ENUM) ENUM,
# define GENERATE_STRING(STRING) #STRING,

typedef enum hash_algorithm_e {
    FOREACH_ALGORITHM(GENERATE_ENUM)
} hash_algorithm_t;

static const char *algorithm_string[] = {
    FOREACH_ALGORITHM(GENERATE_STRING)
};
*/

#include <crypt.h>     // Crypt functions
#include <fcntl.h>     // open, O_RDONLY, O_WRONLY, O_TRUNC, O_CREAT
#include <pthread.h>   // Thread functions
#include <stdio.h>     // fprintf, printf, stderr
#include <stdlib.h>    // EXIT_FAILURE, EXIT_SUCCESS, malloc, free
#include <string.h>    // strcmp, strlen, strncpy
#include <sys/stat.h>  // stat, struct stat, S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH
#include <sys/types.h> // mode_t, off_t, uid_t, gid_t
#include <time.h>      // localtime, strftime, struct tm
#include <unistd.h>    // close, lseek, read, write, STDOUT_FILENO

#include "thread_hash.h"

void help(void);

int main(int argc, char *argv[]) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *dictionary = NULL;
    int threads = 1;
    int verbose = 0;

    {
        // Handle command line arguments. Exit if invalid.
        int opt = -1;

        while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
            switch (opt) {
            case 'i':
                input_file = strcpy(input_file, optarg);
                break;
            case 'o':
                output_file = strcpy(output_file, optarg);
                break;
            case 'd':
                dictionary = strcpy(dictionary, optarg);
                break;
            case 'h':
                help();
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                verbose += 1;
                fprintf(stderr, "Verbose mode enabled: Level %d\n", verbose);
                break;
            case 't':
                threads = atoi(optarg);
                break;
            case 'n':
                nice(NICE_VALUE);
                break;
            default:
                fprintf(stderr,
                        "oopsie - unrecognized command line option \"%s\"\n",
                        argv[optind]);
                exit(EXIT_FAILURE);
                break;
            }
        }

        if (!dictionary) {
            fprintf(stderr, "must give name for dictionary input file with -d\n");
            exit(EXIT_FAILURE);
        }

        if (!input_file) {
            fprintf(stderr, "must give name for hashed password input file with -i\n");
            exit(EXIT_FAILURE);
        }

        if (threads < 1) {
            fprintf(stderr, "invalid thread count %d\n", threads);
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}

void help(void) {
    fprintf(
        stderr,
        "help text\n"
        "       ./thread_hash ...\n"
        "       Options: i:o:d:hvt:n\n"
        "               -i file         input file name (required)\n"
        "               -o file         output file name (default stdout)\n"
        "               -d file         dictionary file name (default stdout)\n"
        "               -t #            number of threads to create (default "
        "1)\n"
        "               -v              enable verbose mode\n"
        "               -h              helpful text\n");
}
