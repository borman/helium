VPATH = .:../../xml:../../util

INCLUDES = -I../..
CFLAGS = $(INCLUDES) -g -Wall --std=gnu99

OBJECTS = xml_test.o xml_parser.o xml_memory.o string_util.o 
TARGET = xml_test

#---

all: $(TARGET)

clean: 
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean

#---

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

xml_test.o: xml_parser.h xml_common.h

xml_gen.o: xml_common.h xml_gen.h string_util.h

xml_parser.o:  xml_parser.h xml_common.h string_util.h xml_memory.h

string_util.o: string_util.h

