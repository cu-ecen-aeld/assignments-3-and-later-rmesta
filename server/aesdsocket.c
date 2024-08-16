/*
 * Rick Mesta
 * 08/15/2024
 *
 * University of Colorado at Boulder
 * ECEN 5713: Advanced Embedded Linux Development
 * Assignment 5 (Part 2)
 */
#include "aesdsocket.h"


void
die(const char *func, char *msg, int ev)
{
    fprintf(stderr, "%s: %s\n", func, msg);
    exit(ev);
}


void
pkt_recv(int csd, int fd)
{
    char    buf[MAX_BUFSZ];
    ssize_t rb = 0;
    ssize_t wb = 0;
    bool    done = false;

    do {
        bzero(buf, sizeof(buf));

        // read contents off the wire
        errno = 0;
        if ((rb = recv(csd, buf, MAX_BUFSZ, 0)) < 0)
            die(__func__, strerror(errno), -1);

        if (buf[rb - 1] == '\n') {
#ifdef  DEBUG
            fprintf(stdout, "\033[0;92m Read %ld bytes\033[0m\n", rb);
#endif  // DEBUG
            done = true;
        }
#ifdef  DEBUG
        else
            fprintf(stderr, "\033[1;91m Incomplete msg: %ld bytes read\033[0m\n", rb);
#endif  // DEBUG

        // append pkt contents to file
        errno = 0;
        if ((wb = write(fd, buf, rb)) < 0)
            die(__func__, strerror(errno), -10);

    } while (!done);
}


void
pkt_send(int csd, int fd)
{
    char        *data = NULL;
    ssize_t      rb = 0;
    ssize_t      wb = 0;
    struct stat  sbuf;

    // exists ?
    errno = 0;
    if (fstat(fd, &sbuf) < 0)
        die(__func__, strerror(errno), -11);

    // rewind
    errno = 0;
    if (((off_t) lseek(fd, (off_t)0, SEEK_SET)) < 0)
        die(__func__, strerror(errno), -12);

    // get heap space
    errno = 0;
    if ((data = (char *) malloc(sbuf.st_size)) == NULL)
        die(__func__, strerror(errno), -13);

    // read contents
    errno = 0;
    if ((rb = read(fd, data, sbuf.st_size)) < 0)
        die(__func__, strerror(errno), -14);

    else if (rb == 0)
        die(__func__, "Failed to read from file", -15);

    else if (rb != sbuf.st_size)
        die(__func__, "Edge case: need to re-read from file", -16);

    assert(rb == sbuf.st_size);

    errno = 0;
    if ((wb = send(csd, data, rb, 0)) < 0)
        die(__func__, strerror(errno), -16);

    assert(wb == rb);
    free(data);
}


int     lsd;    // listen


static void
signal_handler(int signo)
{
    char    *fname = (char *)TMP_FILE;

    syslog(LOG_DEBUG, "Caught signal %d, exiting\n", signo);

    errno = 0;
    if (unlink(fname) < 0)
        fprintf(stderr, "unlink(): %s\n", strerror(errno));

    errno = 0;
    if (shutdown(lsd, SHUT_RDWR) < 0)
        fprintf(stderr, "shutdown(): %s\n", strerror(errno));
    syslog(LOG_DEBUG, "Socket shutdown, exiting\n");

    closelog();
}


int
init()
{
    struct sigaction     sa;
    int                  fd = 0;
    char                *fname = (char *)TMP_FILE;

    (void) sigemptyset(&sa.sa_mask);
    (void) sigaddset(&sa.sa_mask, SIGINT);
    (void) sigaddset(&sa.sa_mask, SIGTERM);

    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;

    errno = 0;
    (void) sigaction(SIGINT, &sa, NULL);
    (void) sigaction(SIGTERM, &sa, NULL);

    errno = 0;
    if ((fd = open(fname, O_CREAT|O_RDWR|O_TRUNC|O_APPEND, S_IRWXU|S_IRWXG)) < 0)
        die(__func__, strerror(errno), -2);

    openlog("AELD Socket Server", LOG_PID, LOG_USER);

    return fd;
}


int
main(int argc, char **argv)
{
    int         csd;    // connected
    int         c;
    const int   on = 1;
    int         dflag = false;
    int         fd = init();

    /*
     * cmdline flags
     */
    while ((c = getopt(argc, argv, "d")) != EOF) {
        switch (c) {
        case 'd':
            dflag = true;
            break;

        default:
            fprintf(stdout, "\nUsage: %s [-d]\n\n", argv[0]);
            exit(0);
        }
    }

    /*
     * Prepare server for wildcard address
     */
    struct addrinfo      hints;
    struct addrinfo     *res;
    struct addrinfo     *saveptr;
    int                  en = 0;

	bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((en = getaddrinfo(NULL, SERV_PORT, &hints, &res)) != 0)
        die(__func__, (char *)gai_strerror(en), -1);
    saveptr = res;

    do {
        errno = 0;
        if ((lsd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
            continue;   // try next addr

        errno = 0;
        if (setsockopt(lsd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
            die("setsockopt()", strerror(errno), -1);

        errno = 0;
        if (bind(lsd, res->ai_addr, res->ai_addrlen) == 0)
            break;      // found one

        fprintf(stderr, "%s: bind failure (%s)\n", __func__, strerror(errno));
        close(lsd);

    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        fprintf(stderr, "\033[1;91mFatal error - No suitable address found\033[0m\n");
        die(__func__, strerror(errno), -1);
    }

    freeaddrinfo(saveptr);

    /*
     * daemonizing
     */
    if (dflag) {
        fprintf(stdout, "\033[1;93mStarting aesdsocket\033[0m\n");

        errno = 0;
        if (daemon(0, 0) < 0)
            die("daemon", strerror(errno), -1);
     }

    errno = 0;
    if (listen(lsd, LISTEN_QL) < 0)
        die(__func__, strerror(errno), -1);

    for (;;) {
        socklen_t            c_len;  // client
        struct sockaddr_in   c_addr;
        char                 c_ipstr[INET_ADDRSTRLEN];
        const char          *cip;   // 4 octect ip addr

        c_len = sizeof(c_addr);
        syslog(LOG_DEBUG, "Accepting connection at port %s\n", SERV_PORT);

        errno = 0;
        if ((csd = accept(lsd, (struct sockaddr *)&c_addr, &c_len)) < 0)
            die(__func__, strerror(errno), -1);

        bzero(c_ipstr, sizeof(c_ipstr));
        cip = inet_ntop(AF_INET, &c_addr.sin_addr, c_ipstr, INET_ADDRSTRLEN);

        syslog(LOG_DEBUG, "Accepted connection from %s\n", cip ? cip : "Unkown");

        pkt_recv(csd, fd);
        pkt_send(csd, fd);

        syslog(LOG_DEBUG, "Closed connection from %s\n", cip ? cip : "Unkown");
	    close(csd);
    }
    closelog();
}
