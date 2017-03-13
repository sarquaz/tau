#!/bin/bash

shopt -s nullglob
for f in test/*
do
    if [[ -x "$f" ]]
    then
        if [ "$f" != "test/run.sh" ]
        then
            if [[ -f "$f" ]]
            then
                echo "running $f"
                "$f"
            fi
         fi
    fi    
    
    if [ $? -ne 0 ]; then
        exit -1
    fi



done


 