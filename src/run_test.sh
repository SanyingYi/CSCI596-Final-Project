for VARIABLE in 1 2 4 8 16 32 64
do
	echo $VARIABLE
	./lsh_opt_openmp_single $VARIABLE
done
