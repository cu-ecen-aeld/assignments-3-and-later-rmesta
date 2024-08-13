#
# Rick Mesta
# 08/13/2024
#
# University of Colorado at Boulder
# ECEN 5713: Advanced Embedded Linux Development
# Assignment 5 (Part 1)
#
RM=/usr/bin/rm

TGT=aesdsocket
SRC=$(TGT).c
OBJ=$(SRC:%.c=%.o)
CFLAGS+= -Wall -Werror

all: $(TGT)

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@ $(LDFLAGS)

clean:
	$(RM) -f $(TGT) $(OBJ)
