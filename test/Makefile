
include ../config.mk

LIBGHT = ../src/$(LIBGHT_A)

CPPFLAGS = $(XML2_CPPFLAGS) $(CUNIT_CPPFLAGS) -I../src
LDFLAGS = $(XML2_LDFLAGS) $(CUNIT_LDFLAGS)

EXE = cu_tester

# ADD YOUR NEW TEST FILE HERE (1/1)
OBJS =	\
	cu_tester.o \
	cu_ght_core.o \
	cu_ght_schema.o

# Build the unit tester
all: $(EXE)

# Build and run the unit tester
check: $(EXE)
	@./$(EXE)

# Build the main unit test executable
$(EXE): $(OBJS) $(LIBGHT)
	$(CC) $(LDFLAGS) -o $@ $^

$(LIBGHT):
	$(MAKE) -C ../src $(LIBGHT_A)

# Clean target
clean:
	@rm -f $(OBJS)
	@rm -f $(EXE)
