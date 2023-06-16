# logger Makefile
CXX      = g++
CXXFLAGS = -Wall -std=c++17

TARGET	 = ./build
SRC	 = Logger.cpp log_settings.cpp logthread.cpp Helpers.cpp

ifeq ($(OS),Windows_NT)
  TARGET = ./build/logger_win32
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
     TARGET = ./build/logger_linux
  endif
  ifeq ($(UNAME_S),Darwin)
     TARGET = ./build/logger_osx
  endif
endif

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET)
