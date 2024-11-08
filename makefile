all: simu cli_automatique

simu: simu.c
	gcc -o simu simu.c -pthread -lgomp

cli_automatique: cli_automatique.c
	gcc -o cli cli_automatique.c -lgomp

run: all
	./simu 8000 &
	sleep 0.6 && ./cli localhost 8000 &
	sleep 0.7 && ./cli localhost 8000 &
	sleep 0.8 && ./cli localhost 8000 &
	sleep 0.9 && ./cli localhost 8000 &
	sleep 1 && ./cli localhost 8000

clean:
	rm -f simu cli
