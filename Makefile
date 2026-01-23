.PHONY: all clean

all: main kasjer klient przewodnik straznik cleanup

main: main.cpp tools.cpp
	g++ -o main main.cpp tools.cpp

kasjer: kasjer.cpp tools.cpp
	g++ -o kasjer kasjer.cpp tools.cpp

klient: klient.cpp tools.cpp
	g++ -o klient klient.cpp tools.cpp

przewodnik: przewodnik.cpp tools.cpp
	g++ -o przewodnik przewodnik.cpp tools.cpp

straznik: straznik.cpp tools.cpp
	g++ -o straznik straznik.cpp tools.cpp

cleanup: cleanup.cpp tools.cpp
	g++ -o cleanup cleanup.cpp tools.cpp

clean:
	rm -f main kasjer klient przewodnik straznik cleanup *.txt