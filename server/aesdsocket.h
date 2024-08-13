/*
 * Rick Mesta
 * 08/13/2024
 *
 * University of Colorado at Boulder
 * ECEN 5713: Advanced Embedded Linux Development
 * Assignment 5 (Part 1)
 */
#ifndef __AESDSOCKET_H__
#define __AESDSOCKET_H__

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <syslog.h>
#include <assert.h>
#include <signal.h>
#include <netdb.h>


#define SERV_PORT   "9000"
#define LISTEN_QL   10
#define MAX_BUFSZ   1024*1024

#define TMP_FILE    "/var/tmp/aesdsocketdata"

#endif /* __AESDSOCKET_H__ */
