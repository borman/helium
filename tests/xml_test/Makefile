VPATH = .:../../xml:../../util

INCLUDES = -I../..
CFLAGS = $(INCLUDES) -g -Wall

OBJECTS = xml_test.o xml_parser.o xml_gen.o string_util.o 
TARGET = xml_test

#---

all: $(TARGET)

.PHONY: all

#---

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

xml_test.o: xml_parser.h xml_common.h

xml_gen.o: xml_common.h xml_gen.h string_util.h

xml_parser.o: xml_parser.h xml_common.h string_util.h

string_util.o: string_util.h

