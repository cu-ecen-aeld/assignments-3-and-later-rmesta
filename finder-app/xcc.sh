#!/bin/zsh
#
# Rick Mesta
# 06/28/2024
#
# University of Colorado at Boulder
# ECEN 5713: Advanced Embedded Linux Development
# Assignment 2 (Cross compile output)
#
FILE="../assignments/assignment2/fileresult.txt"

uname -a >> ${FILE}
date >> ${FILE}
make clean >> ${FILE}
make CROSS_COMPILE=aarch64-none-linux-gnu- >> ${FILE}
file writer.o writer >> ${FILE}
date >> ${FILE}
