VPATH = .:../../xml:../../util

INCLUDES = -I../..
CFLAGS = $(INCLUDES) -g -Wall -pedantic --std=gnu99

SOURCES = xml_test.c xml_parser.c xml_memory.c strbuf.c
TARGET = xml_test

OBJECTS = $(SOURCES:.c=.o)
DEPENDS = $(SOURCES:.c=.d)

#---

all: $(TARGET)

clean: 
	rm -f $(TARGET) $(OBJECTS) $(DEPENDS)

.PHONY: all clean

#---

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

%.d: %.c
	$(CC) $(CFLAGS) -MF $@ -MM $<
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@

-include $(DEPENDS)

