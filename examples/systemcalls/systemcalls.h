#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

/*
 * Rick Mesta
 * 06/29/2024
 *
 * University of Colorado at Boulder
 * ECEN 5713: Advanced Embedded Linux Development
 * Assignment 3 (Part 1)
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

bool do_system(const char *command);

bool do_exec(int count, ...);

bool do_exec_redirect(const char *outputfile, int count, ...);
