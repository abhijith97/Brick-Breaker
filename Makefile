all: play

play: Sample_GL3_2D.cpp glad.c 
	g++ -o play Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl -lao -ldl -lm `pkg-config --libs ftgl` `pkg-config --cflags ftgl` -pthread -std=c++11

clean:
	rm play
