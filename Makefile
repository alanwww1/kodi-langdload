DEBUG          := YES

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

DEBUG_CXXFLAGS   := -Wall -Wno-format -g -DDEBUG
RELEASE_CXXFLAGS := -Wall -Wno-unknown-pragmas -Wno-format -O3

LIBS		 := -lcurl

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

INCS := -Ilib/Json-cpp/include

OUTPUT := kodi-langdload

all: ${OUTPUT}

SRCS := lib/TinyXML/tinyxml.cpp lib/TinyXML/tinyxmlparser.cpp lib/TinyXML/tinystr.cpp lib/TinyXML/tinyxmlerror.cpp \
lib/XMLHandler.cpp \
lib/HTTPUtils.cpp \
lib/CharsetUtils.cpp \
lib/LCodeHandler.cpp \
lib/LCode.cpp \
lib/Fileversioning.cpp \
lib/FileUtils.cpp \
lib/Log.cpp \
lib/ConfigHandler.cpp \
lib/ResourceHandler.cpp \
lib/JSONHandler.cpp \
lib/Json-cpp/src/lib_json/json_reader.cpp \
lib/Json-cpp/src/lib_json/json_value.cpp \
lib/Json-cpp/src/lib_json/json_writer.cpp \
$(OUTPUT)

OBJS := $(addsuffix .o,$(basename ${SRCS}))

${OUTPUT}: ${OBJS}
	${LD} -o $@ ${OBJS} ${LIBS} ${LDFLAGS}

%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJS} ${OUTPUT}

tinyxml.o: tinyxml.h tinyxml.cpp tinystr.o tinyparser.o tinyxmlerror.o
tinyxmlparser.o: tinyxmlparser.cpp tinyxmlparser.h
tinyxmlerror.o: tinyxmlerror.cpp tinyxmlerror.h
tinystr.o: tinystr.cpp tinystr.h
FileUtils.o: FileUtils.h FileUtils.cpp Log.cpp Log.h
XMLHandler.o: XMLHandler.h Log.cpp Log.h tinyxml.o CharsetUtils.h CharsetUtils.cpp Types.h ConfigHandler.h ConfigHandler.cpp
HTTPUtils.o: HTTPUtils.h Log.h Log.cpp Types.h
JSONHandler.o: JSONHandler.h JSONHandler.cpp
ResourceHandler.o: ResourceHandler.h Log.cpp Log.h JSONHandler.o JSONHandler.h JSONHandler.cpp
CharsetUtils.o: CharsetUtils.cpp CharsetUtils.h
Fileversioning.o: Fileversioning.h Fileversioning.cpp
LCodeHandler.o: LCodeHandler.h LCodeHandler.cpp
LCode.o: LCodeHandler.h LCodeHandler.cpp LCode.h LCode.cpp

install:
	install -m 755 kodi-langdload /usr/local/bin/
uninstall:
	rm -rf /usr/local/bin/kodi-langdload
