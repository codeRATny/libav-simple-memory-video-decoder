.Phony: all clean clean_frames

CXX = g++
LIBS = -lavcodec -lavformat -lavfilter -lavdevice -lswresample -lswscale -lavutil

INCLUDES += -I ./src/Utils
INCLUDES += -I ./src/Player

BIN_PATH = bin
BUILD_PATH = build
SRC_PATH = src

CFLAGS = -Wall
LDFLAGS = -Wall

all: $(BIN_PATH)/main

$(BIN_PATH)/main: $(BUILD_PATH)/main.o $(BUILD_PATH)/player.o $(BUILD_PATH)/player_utils.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LIBS) $(INCLUDES)

$(BUILD_PATH)/main.o: $(SRC_PATH)/main.cpp
	$(CXX) $(LDFLAGS) $< -c -o $@ $(INCLUDES)

$(BUILD_PATH)/player.o: $(SRC_PATH)/Player/player.cpp
	$(CXX) $(LDFLAGS) $< -c -o $@ $(INCLUDES)

$(BUILD_PATH)/player_utils.o: $(SRC_PATH)/Player/player_utils.cpp
	$(CXX) $(LDFLAGS) $< -c -o $@ $(INCLUDES)

clean:
	rm build/*.o bin/main

clean_frames:
	rm bin/*.yuv