all: Slice

Slice: Slice.c
	g++ Slice.c -o Slice -framework GLUT -framework OpenGL -Wall

clean:
	rm -f *~ Slice
