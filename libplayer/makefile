CC=gcc 
IDIR=include
CFLAGS=-g -I../server/include -I$(IDIR) -Wall -Wextra -Werror
SRCDIR=src
ODIR=bin
SRCEXT=c
HDREXT=h

LIBS=-lpthread,m

# Headers dentro de include/
DEPS= $(shell find $(IDIR) -type f -name "*.$(HDREXT)")
EXT_DEPS= $(shell find ../server/include -type f -name "*.$(HDREXT)")

# Object files (.o) que haya que compilar 
OBJ= $(shell find $(SRCDIR) -type f -name "*.c" \
	   | sed "s/\.c/.o/g" \
	   | sed "s/$(SRCDIR)/$(ODIR)/g")

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS) $(EXT_DEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

libplayer.a: $(OBJ)
	ar rcs $(ODIR)/$@ $^ $(LIBS)

test: libplayer.a
	$(CC) test/test.c -o bin/test -lplayer -Lbin -lpthread -lm $(CFLAGS)

.PHONY: clean

clean:
	rm -rf $(ODIR)
	mkdir -p $(ODIR)
