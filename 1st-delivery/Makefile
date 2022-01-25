# Makefile
# Sistemas Operativos, DEI/IST/ULisboa 2017-18

heatSim: main.o matrix2d.o leQueue.o mplib3.o
	gcc -pthread -o heatSim main.o matrix2d.o leQueue.o mplib3.o

main.o: main.c matrix2d.h
	gcc -g -c  main.c

matrix2d.o: matrix2d.c matrix2d.h
	gcc -g -c matrix2d.c

leQueue.o: leQueue.c leQueue.h
	gcc -g -c leQueue.c

mplib3.o: mplib3.c mplib3.h leQueue.h
	gcc -g -c mplib3.c


clean:
	rm -f *.o main