# Project: genewars-saveedit

CPP  = g++
CC   = gcc
WINDRES = windres
RM = rm -f
ECHO = @echo
INCS =
CXXINCS =
RES  = obj/gwsaved_private.res
CXXFLAGS = $(CXXINCS)
CFLAGS = -DDEBUG -g -c $(INCS)

OBJ_GWSAVED  = obj/gwsaved.o obj/bulcommn.o obj/lbfileio.o
OBJ_GWSAVED += $(RES)
LIBS_GWSAVED =
BIN_GWSAVED  = gwsaved.exe

BIN = $(BIN_GWSAVED)
OBJ = $(OBJ_GWSAVED)

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN_GWSAVED): $(OBJ_GWSAVED)
	$(CC) $(OBJ_GWSAVED) -o $(BIN_GWSAVED) $(LIBS_GWSAVED)

obj/%.o: src/%.c
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/%.res: res/%.rc 
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff 
