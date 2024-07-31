#!/bin/sh
# Tester script for assignment 1 and assignment 2
# Author: Siddhant Jajoo

set -e
set -u

NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/aeld-data

#
# Rick Mesta
# 07/31/2024
#
# University of Colorado at Boulder
# ECEN 5713: Advanced Embedded Linux Development
# Assignment 4 (Part 2)
#
WRITER=/usr/bin/writer
FINDER=/usr/bin/finder.sh
CFGDIR=/etc/finder-app/conf
SAVEFILE="/tmp/assignment4-result.txt"
SAVECMD="tee -a ${SAVEFILE} 2>&1"

username=$(cat ${CFGDIR}/username.txt)

if [ $# -lt 3 ]
then
	echo "Using default value ${WRITESTR} for string to write" | ${SAVECMD}
	if [ $# -lt 1 ]
	then
		echo "Using default value ${NUMFILES} for number of files to write" | ${SAVECMD}
	else
		NUMFILES=$1
	fi	
else
	NUMFILES=$1
	WRITESTR=$2
	WRITEDIR=/tmp/aeld-data/$3
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}" | ${SAVECMD}

rm -rf "${WRITEDIR}"

# create $WRITEDIR if not assignment1
assignment=`cat ${CFGDIR}/assignment.txt`

if [ $assignment != 'assignment1' ]
then
	mkdir -p "$WRITEDIR"

	#The WRITEDIR is in quotes because if the directory path consists of spaces, then variable substitution will consider it as multiple argument.
	#The quotes signify that the entire string in WRITEDIR is a single string.
	#This issue can also be resolved by using double square brackets i.e [[ ]] instead of using quotes.
	if [ -d "$WRITEDIR" ]
	then
		echo "$WRITEDIR created" | ${SAVECMD}
	else
		exit 1
	fi
fi

for i in $( seq 1 $NUMFILES)
do
	${WRITER} "$WRITEDIR/${username}$i.txt" "$WRITESTR"
done

OUTPUTSTRING=$(${FINDER} "$WRITEDIR" "$WRITESTR" | ${SAVECMD})

# remove temporary directories
rm -rf /tmp/aeld-data

set +e
echo ${OUTPUTSTRING} | grep "${MATCHSTR}"
if [ $? -eq 0 ]; then
	echo "success" | ${SAVECMD}
	exit 0
else
	echo "failed: expected  ${MATCHSTR} in ${OUTPUTSTRING} but instead found" | ${SAVECMD}
	exit 1
fi
