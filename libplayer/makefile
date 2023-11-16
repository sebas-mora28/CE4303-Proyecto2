CC=gcc 
IDIR=include
CFLAGS=-I$(IDIR) -Wall -Wextra -Werror -static
SRCDIR=src
ODIR=bin
SRCEXT=c
HDREXT=h


LIBS=-lpthread

# Headers dentro de include/
DEPS = $(shell find $(IDIR) -type f -name "*.$(HDREXT)")

# Object files (.o) que haya que compilar 
OBJ = $(shell find $(SRCDIR) -type f -name "*.c" \
	   | sed "s/\.c/.o/g" \
	   | sed "s/$(SRCDIR)/$(ODIR)/g")

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

libplayer.a: $(OBJ)
	ar rcs $(ODIR)/$@ $^ $(LIBS)

.PHONY: clean

clean:
	rm -rf $(ODIR)
	mkdir -p $(ODIR)
