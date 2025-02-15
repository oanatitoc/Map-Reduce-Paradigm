CC = gcc
CFLAGS = -lpthread
SOURCES = main.c utils.c reducer.c mapper.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = tema1

build: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)