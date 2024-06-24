#!/bin/bash
#
# Rick Mesta
# 06/24/2024
#
# University of Colorado at Boulder
# ECEN 5713: Advanced Embedded Linux Development
# Assignment 1
#

usage ()
{
    echo "$0 writefile writestr"
}

ferror ()
{
    echo "File $1 could not be created"
}

[[ $# -lt 2 ]] && usage && exit 1

writefile=$1
writestr=$2

DIR=$(dirname ${writefile})
FILE=$(basename ${writefile})
[[ ! -d ${DIR} ]] && mkdir -p ${DIR}

echo "${writestr}" > ${DIR}/${FILE}
[[ $? -ne 0 ]] && ferror "${DIR}/${FILE}" && exit 1

exit 0
