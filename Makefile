PPUCC	= ppu-gcc
EMB		= ppu-embedspu
SPUCC	= spu-gcc
CFLAGS	= -lspe2 -O3
SCFLAGS	= -O3

SPUPROG	= spu_matrix_add
PROGRAM	= matrix_add
SPUOBJ	= $(SPUPROG).o
PPUOBJ	= $(PROGRAM).o

all: $(PROGRAM)

$(PROGRAM): $(SPUOBJ)
	$(PPUCC) -o $(PROGRAM) $(PROGRAM).c $(SPUOBJ) $(CFLAGS)

$(SPUOBJ):
	$(SPUCC) $(SPUPROG).c $(SCFLAGS) -o $(SPUPROG)
	$(EMB) -m64 $(SPUPROG) $(SPUPROG) $(SPUOBJ)

clean:
	rm $(PROGRAM) $(SPUPROG) *.o
