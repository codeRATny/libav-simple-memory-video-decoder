.PHONY: all clean

CXX = g++
CC = gcc
CFLAGS = -Wall
PREFIX = .

CBJS += $(wildcard ./*.cpp)
OBJS := $(patsubst %.cpp,%.o, $(CBJS))

all: $(OBJS) $(PREFIX)/ffmpegwrap.a 

define clean_objs
	rm $(OBJS)
endef

%.o : %.cpp
	$(CXX) $(CFLAGS) -c  $< -o $@

$(PREFIX)/ffmpegwrap.a: $(OBJS)
	ar rsv $@ $^ && \
	$(call clean_objs)

clean:
	rm $(PREFIX)/ffmpegwrap.a