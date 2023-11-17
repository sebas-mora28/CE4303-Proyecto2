CC=mpicc 
CFLAGS= -Wall -Wextra -g -I../libplayer/include -Iinclude
SRCDIR= src
BINDIR= bin
SRCEXT= c
HDREXT= h
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(SOURCES:.$(SRCEXT)=.o)
LIBS= -lsndfile -lm -lfftw3 -L../libplayer/bin -lplayer


all: exec 

build: $(OBJECTS)
		@mkdir -p $(BINDIR)
		$(CC) $(CFLAGS) $^ -o $(BINDIR)/server $(LIBS)
		@$(RM) -r $(OBJECTS).

%.o: %.$(SRCEXT)
	$(CC) $(CFLAGS) -c $< -o $@

exec: build
	mpirun -np 3 ./$(BINDIR)/server


debug: build
	gdb $(BINDIR)/client