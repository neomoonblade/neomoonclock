CC = gcc
CFLAGS	 = -Wall `pkg-config --cflags gtk+-3.0` -MMD -MP
LIBS		 = `pkg-config --libs gtk+-3.0`
INCLUDES	 = -I "src"
#C_FILES = $(wildcard src/*.c)
#O_FILES = $(addprefix build/,$(notdir $(patsubst %.c,%.o,$(C_FILES))))
C_FILES = $(shell find . -type f -name '*.c')
O_FILES = $(patsubst %.c,%.o,$(C_FILES))
PROGRAM = Program

test:
	@echo $(shell find . -type f -name '*.c')
	@echo $(O_FILES)

run: $(PROGRAM)
	./$(PROGRAM)

compile: $(PROGRAM)

$(PROGRAM): $(O_FILES)
	$(CC) $(CFLAGS) $(INCLUDES) $(O_FILES) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIBS)

clean:
	rm $(O_FILES)
	rm $(patsubst %.o,%.d,$(O_FILES))
