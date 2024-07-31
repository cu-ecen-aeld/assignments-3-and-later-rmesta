#
# Rick Mesta
# 07/31/2024
#
# University of Colorado at Boulder
# ECEN 5713: Advanced Embedded Linux Development
# Assignment 4 (Part 2)
#
RM=/usr/bin/rm

TGT=writer
SRC=$(TGT).c
OBJ=$(SRC:%.c=%.o)

all: $(TGT)

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@ $(LDFLAGS)

clean:
	$(RM) -f $(TGT) $(OBJ)
