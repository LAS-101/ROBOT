# Makefile for Robot Simulation with raylib

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11

TARGET = robot_simulation
SOURCE = robot_simulation.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) journal_robot.txt

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run