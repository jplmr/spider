CXX = g++
CFLAGS = `pkg-config --cflags opencv x11`
LIBS = `pkg-config --libs opencv x11`
SRC = src
BIN = bin

SRCS := $(wildcard $(SRC)/*.cpp)
OBJS := $(addprefix $(BIN)/,$(notdir $(SRCS:.cpp=.o)))

.PHONY: all

all: $(BIN)/spider $(BIN)/trainer

$(BIN)/spider: $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm bin/*

$(BIN)/%.o : $(SRC)/%.cpp
	$(CXX) $(CFLAGS) -Iinclude/ -c -o $@ $<

$(BIN)/trainer: $(SRC)/trainer/trainer.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

$(BIN)/control: $(SRC)/control.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)
