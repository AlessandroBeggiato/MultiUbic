#!/bin/bash
generator="./bin/testNetGen"
partitioner="./bin/partitioner"
destDir="./TestSystems/T/MultiUbic"
ubicDir="./TestSystems/T/UBIC2"



ready=1
echo "Net generator..."

if [ -e "$generator" ]
then
	echo "check."
else
	echo "Not found. Did you run make?"
	ready=0
fi

echo "Partitioner..."
if [ -e "$partitioner" ]
then
	echo "check."
else
	echo "Not found. Did you run make?"
	ready=0
fi

if (( $ready < 1 ))
then
	echo "Some tools are missing. Run make tools or make all to build them"
else

	for N in {100..300..10}
	do
		echo "*******************************"
		echo "Generating MultiUbic test nets: size $N / 300"
		echo "*******************************"
		L=1
		E=0
		./bin/testNetGen -n $N -l $L -e $E $destDir
		net=TestNet-$N-$L-$E
		./bin/partitioner $destDir/$net.ll_net $destDir/$net.msd
		mv $destDir/$net $ubicDir
	
		L=2
		E=0
		./bin/testNetGen -n $N -l $L -e $E $destDir
		net=TestNet-$N-$L-$E
		./bin/partitioner $destDir/$net.ll_net $destDir/$net.msd
		mv $destDir/$net $ubicDir
		E=2
		./bin/testNetGen -n $N -l $L -e $E $destDir
		net=TestNet-$N-$L-$E
		./bin/partitioner $destDir/$net.ll_net $destDir/$net.msd
		mv $destDir/$net $ubicDir
		
		L=3
		for E in 0 3 6
		do
			./bin/testNetGen -n $N -l $L -e $E $destDir
			net=TestNet-$N-$L-$E
			./bin/partitioner $destDir/$net.ll_net $destDir/$net.msd
			mv $destDir/$net $ubicDir
		done	
	done
	
fi