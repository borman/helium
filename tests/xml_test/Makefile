VPATH = .:../../xml:../../util

INCLUDES = -I../..
CFLAGS = $(INCLUDES) -g -Wall -pedantic --std=gnu99

OBJECTS = xml_test.o xml_parser.o xml_memory.o strbuf.o
TARGET = xml_test

#---

all: $(TARGET)

clean: 
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean

#---

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< 


