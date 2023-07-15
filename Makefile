.Phony: all clean prepare

CXX = g++
LIBS += -lavcodec -lavformat -lavutil

INCLUDES += -I ./src/Utils
INCLUDES += -I ./src/Player_v2
INCLUDES += -I ./src/FFmpegWrap

BIN_PATH = bin
BUILD_PATH = build
SRC_PATH = src


CFLAGS = -Wall -g
LDFLAGS = -Wall -g
# CFLAGS = -Wall -g -fsanitize=leak
# LDFLAGS = -Wall -g -fsanitize=leak

CBJS += $(shell find $(SRC_PATH) -name "*.cpp" -type f)
OBJS := $(patsubst %.cpp,%.o, $(CBJS))

define clean_objs
	rm $(OBJS)
endef

all: $(OBJS) $(BIN_PATH)/main2

%.o : %.cpp
	$(CXX) $(CFLAGS) -c  $< -o $@ $(INCLUDES) $(LIBS)

$(BIN_PATH)/main2: $(OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LIBS)

clean:
	$(call clean_objs)
	rm $(BIN_PATH)/main2
# all: prepare $(BIN_PATH)/main2

# $(BIN_PATH)/main2: $(BUILD_PATH)/main2.o $(BUILD_PATH)/player.o $(BUILD_PATH)/player_utils.o ./build/ffmpegwrap.a
# 	$(CXX) $(CFLAGS) $(LIBS) $^ -o $@  $(INCLUDES) 

# $(BUILD_PATH)/main2.o: $(SRC_PATH)/main2.cpp
# 	$(CXX) $(LDFLAGS) $< -c -o $@ $(INCLUDES)

# $(BUILD_PATH)/player.o: $(SRC_PATH)/Player_v2/player.cpp
# 	$(CXX) $(LDFLAGS) $< -c -o $@ $(INCLUDES)

# $(BUILD_PATH)/player_utils.o: $(SRC_PATH)/Player_v2/player_utils.cpp
# 	$(CXX) $(LDFLAGS) $< -c -o $@ $(INCLUDES)

# prepare:
# 	$(call install_wrap)