.Phony: all clean prepare

CXX = g++
LIBS += -lavcodec -lavformat -lavutil

INCLUDES += -I ./src/Utils
INCLUDES += -I ./src/Player
INCLUDES += -I ./src/FFmpegWrap

BIN_PATH = bin
BUILD_PATH = build
SRC_PATH = src

# CFLAGS = -Wall -g
# LDFLAGS = -Wall -g
CFLAGS = -Wall -g -fsanitize=leak
LDFLAGS = -Wall -g -fsanitize=leak

CBJS += $(shell find $(SRC_PATH) -name "*.cpp" -type f)
OBJS := $(patsubst %.cpp,%.o, $(CBJS))

define clean_objs
	rm $(OBJS)
endef

all: $(OBJS) $(BIN_PATH)/main

%.o : %.cpp
	$(CXX) $(CFLAGS) -c  $< -o $@ $(INCLUDES) $(LIBS)

$(BIN_PATH)/main: $(OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LIBS)

clean:
	$(call clean_objs)
	rm $(BIN_PATH)/main