CXX = g++
CFLAGS = `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`
SRC = src
BIN = bin

SRCS := $(wildcard $(SRC)/*.cpp)
OBJS := $(addprefix $(BIN)/,$(notdir $(SRCS:.cpp=.o)))

.PHONY: all

all: $(BIN)/spider

$(BIN)/spider: $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm bin/*

$(BIN)/%.o : $(SRC)/%.cpp
	$(CXX) $(CFLAGS) -Iinclude/ -c -o $@ $<

