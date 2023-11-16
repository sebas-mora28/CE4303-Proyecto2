CC=gcc 
CFLAGS= -Wall
SRCDIR= src
BINDIR= bin
SRCEXT= c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(SOURCES:.$(SRCEXT)=.o)
LIBS= -lsndfile

all: exec 

build: $(OBJECTS) 
		@mkdir -p $(BINDIR)
		$(CC) $(CFLAGS) $^  -o $(BINDIR)/client $(LIBS)
		@$(RM) -r $(OBJECTS)

%.o: %.$(SRCEXT)
	$(CC) $(CFLAGS) -c $< -o $@

exec: build
	./$(BINDIR)/client $(file)


debug: build
	gdb $(BINDIR)/client