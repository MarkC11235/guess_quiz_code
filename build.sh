clear

if [ $# -lt 1 ]; then
	echo "Please provide canvas api key\n"
	exit 1
fi 

gcc -o gqc gqc.c -lcurl
./gqc $1
