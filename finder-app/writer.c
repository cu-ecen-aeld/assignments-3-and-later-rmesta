/*
 * Rick Mesta
 * 06/28/2024
 *
 * University of Colorado at Boulder
 * ECEN 5713: Advanced Embedded Linux Development
 * Assignment 2
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>


void
usage(const char *prog)
{
    fprintf(stderr, "usage:\n\t%s writefile writestr\n", prog);
    exit(1);
}


void
errlog(char *msg, int errnum, int es)
{
    syslog(LOG_ERR, "%s: %s\n", msg, strerror(errnum));
    exit(es);
}


char *
str_fixup(const char *msg, size_t *c_len)
{
    size_t   slen = strlen(msg);
    size_t   clen = slen + 1;
    char    *cmsg = NULL;

    errno = 0;
    if ((cmsg = (char *)malloc(clen)) == NULL)
        errlog("malloc", errno, 2);
    bzero(cmsg, clen);

    snprintf(cmsg, clen, "%s", msg);
    cmsg[slen] = '\n';

    *c_len = clen;
    return cmsg;
}


char *
dirname(const char *path)
{
    char    *dname = NULL;
    char    *p = NULL;

    if (strstr(path, "/") == NULL)
        return NULL;

    errno = 0;
    if ((dname = strdup(path)) == NULL)
        errlog("strdup", errno, 3);

    for (p = &dname[strlen(path)]; *p != '/'; --p)
        ;       // find last '/' separator

    *p = '\0';  // replace last '/' with '\0'

    return dname;
}


char *
basename(const char *path)
{
    char    *bname = NULL;
    char    *p = NULL;
    char    *n = NULL;

    errno = 0;
    if ((bname = strdup(path)) == NULL)
        errlog("strdup", errno, 4);

    for (p = &bname[strlen(path)]; *p != '/'; n = p, --p)
        ;       // find last '/' separator

    return n;
}


char *
filename(const char *path)
{
    char    *dname = NULL;  // dirname
    char    *bname = NULL;  // basename
    char    *fname = NULL;
    size_t   f_len = 0;

    if ((dname = dirname(path)) == NULL)
        return (char *)path;

    bname = basename(path);
    f_len = strlen(dname) + strlen(bname) + 2;
    fname = (char *)malloc(f_len);
    snprintf(fname, f_len, "%s/%s", dname, bname);

    free(dname);
    return fname;
}


int
main(int argc, char **argv)
{
    char    *fqfn = NULL;
    int      fd = 0;
    ssize_t  wb = 0;
    char    *o_msg = NULL;
    char    *c_msg = NULL;
    size_t   c_len = 0;

    if (argc < 3)
        usage(argv[0]);

    openlog("AELD writer", LOG_PID, LOG_USER);

    fqfn = filename(argv[1]);
    o_msg = argv[2];

    errno = 0;
    if ((fd = open(fqfn, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR)) < 0)
        errlog("open", errno, 1);

    c_msg = str_fixup(o_msg, &c_len);
    syslog(LOG_DEBUG, "Writing %s to %s\n", o_msg, fqfn);

    errno = 0;
    if ((wb = write(fd, c_msg, c_len)) < 0)
        errlog("write", errno, 1);

    free(fqfn);
    free(c_msg);

    exit(0);
}
