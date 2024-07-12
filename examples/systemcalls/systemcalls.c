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
    va_list     args;
    va_start(args, count);

    char        *command[count+1];
    int          i;

    for (i = 0; i < count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    /*
     * Rick Mesta
     * 07/12/2024
     *
     * University of Colorado at Boulder
     * ECEN 5713: Advanced Embedded Linux Development
     * Assignment 3 (Part 1)
     */
    pid_t   pid;

    errno = 0;
    switch ((pid = fork())) {
    case -1:
        fprintf(stderr, "%s: %s\n", __func__, strerror(errno));
        return false;

    case 0:
        {   // Child

            int         rv = 0;

            errno = 0;
            if ((rv = execv(command[0], command)) < 0) {
                fprintf(stderr, "*** ERROR: exec failed with return value %d\n", rv);

                switch (errno) {
                case ENOENT:
                    fprintf(stderr, "execv: echo error: %s\n", strerror(errno));
                    exit(1);

                default:
                    fprintf(stderr, "execv: [other error] %s\n", strerror(errno));
                    exit(2);
                }
            }
            fprintf(stdout, "[Child] Command %s returned non zero exit code %d\n",
                    command[0], rv);
            exit(0);
        }

    default:
        {   // Parent

            int     status;
            pid_t   kpid = 0;

            errno = 0;
            if ((kpid = waitpid(pid, &status, 0)) < 0) {
                fprintf(stderr, "%s: [Parent] %s\n", __func__, strerror(errno));
                return false;
            }
            else if (kpid == pid) {
                if (WIFEXITED(status)) {
                    int es = WEXITSTATUS(status);

                    switch (es) {
                    case 1:
                        fprintf(stdout,
                                "Command %s returned non zero exit code %d\n",
                                command[0], es);
                        return false;

                    default:
                        return true;
                    }
                }
                return false;
            }
        }
        break;
    }

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
    va_list      args;
    va_start(args, count);

    char        *command[count+1];
    int          i;

    for (i = 0; i < count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    command[count] = command[count];

/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
 */

    /*
     * Rick Mesta
     * 07/12/2024
     *
     * University of Colorado at Boulder
     * ECEN 5713: Advanced Embedded Linux Development
     * Assignment 3 (Part 1)
     */
    pid_t   pid;
    int     fd = 0;

    errno = 0;
    if ((fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644)) < 0) {
        fprintf(stderr, "%s: (open): %s\n", __func__, strerror(errno));
        return false;
    }

    errno = 0;
    switch ((pid = fork())) {
    case -1:
        fprintf(stderr, "%s: %s\n", __func__, strerror(errno));
        return false;

    case 0:
        {   // Child

            int     rv = 0;
            int     nfd = 0;

            errno = 0;
            if ((nfd = dup2(fd, 1)) < 0) {  // stdin = 0, stdout = 1, stderr = 2
                fprintf(stderr, "%s: (dup2): %s\n", __func__, strerror(errno));
                return false;
            }
            close(fd);

            errno = 0;
            if ((rv = execv(command[0], command)) < 0) {
                dprintf(nfd, "*** ERROR: exec failed with return value %d\n", rv);

                switch (errno) {
                case ENOENT:
                    dprintf(nfd, "execv: echo error: %s\n", strerror(errno));
                    exit(1);

                default:
                    dprintf(nfd, "execv: [other error] %s\n", strerror(errno));
                    exit(2);
                }
            }
            dprintf(nfd, "[Child] Command %s returned non zero exit code %d\n",
                    command[0], rv);
            exit(0);
        }

    default:
        {   // Parent

            int     status;
            pid_t   kpid = 0;

            close(fd);

            errno = 0;
            if ((kpid = waitpid(pid, &status, 0)) < 0) {
                fprintf(stderr, "%s: [Parent] %s\n", __func__, strerror(errno));
                return false;
            }
            else if (kpid == pid) {
                if (WIFEXITED(status)) {
                    int es = WEXITSTATUS(status);

                    switch (es) {
                    case 1:
                        fprintf(stdout,
                                "Command %s returned non zero exit code %d\n",
                                command[0], es);
                        return false;

                    default:
                        return true;
                    }
                }
                return false;
            }
        }
        break;
    }

    va_end(args);
    return true;
}
