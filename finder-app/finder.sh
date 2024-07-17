#!/bin/sh
#
# Rick Mesta
# 07/17/2024
#
# University of Colorado at Boulder
# ECEN 5713: Advanced Embedded Linux Development
# Assignment 3 Part 2
#

usage ()
{
    echo "$0 filesdir searchstr"
}

not_dir ()
{
    echo "$1 is not a valid directory"
}

[[ $# -lt 2 ]] && usage && exit 1

filesdir=$1
[[ ! -d ${filesdir} ]] && not_dir ${filesdir} && exit 1

searchstr=$2
FILES=$(find ${filesdir} -type f)

fcnt=0
lmp=0
for f in ${FILES}; do
    fcnt=$((fcnt+1))
    cnt=$(grep -c ${searchstr} ${f})
    if [[ ${cnt} -gt 0 ]]; then
        lmp=$((lmp + cnt))
        cnt=0
    fi
done

echo "The number of files are ${fcnt} and the number of matching lines are ${lmp}"
