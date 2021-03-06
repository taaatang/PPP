#!/usr/bin/env bash

# env=env.sh
# set environment
cwd=$(pwd)
# cd ~
# source ./${env}
# cd ${cwd}

molname=Ada
JobDir=Job/${molname}
keyword1=RUNID
keyword2=MOLNAME
app=main.out
script=run_sherlock.sh
input=input.txt

for i in $(seq 0 1 6)
    do
        appDir=${JobDir}/run_${i}
        mkdir -p ${appDir}
        inputDir=${appDir}
        mkdir -p ${inputDir}

        sed -e "s/${keyword1}/${i}/g;s/${keyword2}/${molname}/g" ${input} > ${inputDir}/${input}
        sed -e "s/${keyword1}/${i}/g;s/${keyword2}/${molname}/g" ${script} > ${appDir}/${script}
        cd ${appDir}
        rm -rf job.out
        rm -rf job.err
        sbatch ${script}
        cd ${cwd}
    done