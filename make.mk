all: graph.png

graph.png : dist.data
	python graph.py
	rm -f *.dat

dist.data: a.x
  ./a.x
  rm -f *.x

a.x : dist.c
	cc -Wall dist.c -o a.x -lm

clean :
rm -f *.x
rm -f *.data
