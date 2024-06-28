#
# Rick Mesta
# 06/28/2024
#
# University of Colorado at Boulder
# ECEN 5713: Advanced Embedded Linux Development
# Assignment 2
#
GCC=gcc
XCC=$(CROSS_COMPILE)$(GCC)
RM=/usr/bin/rm
CFLAGS=-c
LDFLAGS=-g
DEFINES=

TGT=writer
SRC=$(TGT).c
OBJ=$(SRC:%.c=%.o)

.c.o:
	$(XCC) $(DEFINES) $(CFLAGS) -o $@ $<

$(TGT):	$(OBJ)
	$(XCC) $(LDFLAGS) -o $@ $?

clean:
	$(RM) -f $(TGT) $(OBJ)
