#!/bin/bash

netType="Dijkstra"
#netType="T"

testDir="./TestSystems/$netType"
OUTFILE="$testDir/testerOutput.csv"
MultiDir="$testDir/MultiUbic"
UbicDir="$testDir/UBIC2"

Ubic="./bin/ubic2"
MultiUbic="./bin/MultiUbic"

ready=1

echo "Ubic2..."
if [ -e "$Ubic" ]
then
	echo "check."
else
	echo "Not found. Did you place the binary in MultiUbic/bin?"
	ready=0
fi

echo "MultiUbic..."
if [ -e "$MultiUbic" ]
then
	echo "check."
else
	echo "Not found. Did you run make?"
	ready=0
fi

if (( $ready < 1 ))
then
	echo "Something is missing. Run make to install MultiUbic and copy the Ubic2 binary into MultiUbic/bin"
else

	echo "Net Name ; MultiUbic (ms); Ubic (ms)" > $OUTFILE
	while read NET; do
		echo "net: $NET"
		if [ $netType == "Dijkstra" ]
		then 
			POL="LH"
			MSTART=$(($(date +%s%N)/1000000))
			./bin/MultiUbic $MultiDir/$NET.ll_net $MultiDir/$POL.msd
			MEND=$(($(date +%s%N)/1000000))
			USTART=$(($(date +%s%N)/1000000))
			./bin/ubic2 $UbicDir/$NET.ll_net
			UEND=$(($(date +%s%N)/1000000))
			echo "$NET ; $((MEND-MSTART)) ; $((UEND-USTART))" >> $OUTFILE
		else
			POL=$NET
			MSTART=$(($(date +%s%N)/1000000))
			./bin/MultiUbic $MultiDir/$NET.ll_net $MultiDir/$POL.msd
			MEND=$(($(date +%s%N)/1000000))
			USTART=$(($(date +%s%N)/1000000))

			for N in $UbicDir/$NET/*; do
				./bin/ubic2 $N
			done	

			UEND=$(($(date +%s%N)/1000000))
			echo "$NET ; $((MEND-MSTART)) ; $((UEND-USTART))" >> $OUTFILE
	fi
		
	done < netList.txt

fi