while true; do
	./gen
	./mul2
	mpirun -n 16 ./mul > log.txt
	if diff -b -B mul.out mul2.out ; then
		printf "ac "
	else
		printf "wa "
		exit
	fi
done
