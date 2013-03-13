
include ../config.mk

CPPFLAGS = $(XML2_CPPFLAGS)
LDFLAGS = $(XML2_LDFLAGS)

OBJS = \
	ght_attribute.o \
	ght_hash.o \
	ght_mem.o \
	ght_node.o \
	ght_schema.o \
	ght_util.o \
	stringbuffer.o

all: $(LIBGHT_A)

$(LIBGHT_A): $(OBJS)
	ar rs $@ $^

clean:
	@rm -f $(OBJS) $(LIBGHT_A)


