#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

    /*
     * Rick Mesta
     * 06/29/2024
     *
     * University of Colorado at Boulder
     * ECEN 5713: Advanced Embedded Linux Development
     * Assignment 3 (Part 1)
     */
    int rv = 0;

    errno = 0;
    if ((rv = system(cmd)) < 0) {
        fprintf(stderr, "%s: %s\n", __func__, strerror(errno));
        return false;
    }
    else if (rv == 127) {
        fprintf(stderr, "%s: Shell could not be executed in child\n", __func__);
        return false;
    }
#ifdef  DEBUG
    fprintf(stdout, "%s: \"%s\" returned with termination status = %i\n", __func__, cmd, rv);
#endif
    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    /*
     * Rick Mesta
     * 06/29/2024
     *
     * University of Colorado at Boulder
     * ECEN 5713: Advanced Embedded Linux Development
     * Assignment 3 (Part 1)
     */

    /*
     * Check all args to execv(). Any
     * non-flags MUST be absolute paths
     */
#ifdef DEBUG
    fprintf(stderr, "%s: Verifying arguments...\n", __func__);
#endif
    for (i = 0; i < count; i++) {
        char *p = NULL;

#ifdef DEBUG
        fprintf(stderr, "\t%s... ", command[i]);
#endif
        if ((p = strstr(command[i], "-"))) {
#ifdef DEBUG
            fprintf(stderr, " skipped\n");
#endif
            continue;
        }
        else if ((p = strstr(command[i], "/")) == NULL) {
#ifdef DEBUG
            fprintf(stderr, "is not an absolute path <FALSE>\n");
#endif
            return false;
        }
#ifdef DEBUG
        fprintf(stderr, "<OK>\n");
#endif
    }

    /*
     * If we're here, we know we have
     * secure (full path) arguments.
     */
    pid_t   pid;

    errno = 0;
    if ((pid = fork()) < 0) {
        fprintf(stderr, "%s: %s\n", __func__, strerror(errno));
        return false;
    }
    else if (pid == 0) {
        // child
#ifdef DEBUG
        int          k;

        fprintf(stderr, "%s: [Child] Arguments to execv()\n", __func__);
        for (k = 0; k < count; k++)
            fprintf(stderr, "\t%s\n", command[k]);
#endif
        execv(command[0], command+1);

        fprintf(stderr, "%s: [Child] Shouldn't have gotten here\n", __func__);
        exit(-1);
    }

    // parent
    int status;

    errno = 0;
    if (waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "%s: [Parent] %s\n", __func__, strerror(errno));
        return -1;
    }
#ifdef  DEBUG
    fprintf(stderr, "%s: [Parent] notified of child (exit)\n", __func__);
#endif

    va_end(args);
    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);

    return true;
}
