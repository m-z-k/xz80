SRCS= xz80.cpp
BIN=a.out


#CXXFLAGS += -O3 -g3 -Wall
CXXFLAGS += -fno-operator-names -O0 -g3 -Wall
OBJS=$(SRCS:.cpp=.o)

.PHONY: all run doc test diff clean

all : $(BIN)

run : all
	./$(BIN)

doc : all
	doxygen

test : all
	./$(BIN) | tee a.asm
	z80asm a.asm
	-cmp exp.bin a.bin
	@$(MAKE) -s diff
	z80dasm -z -a exp.bin > exp.asm

diff :
	@od -tx1 a.bin > a.dump
	@od -tx1 exp.bin > exp.dump
	@git diff --no-index --word-diff --color -- exp.dump a.dump

clean :
	-rm $(BIN) $(OBJS)

$(BIN) : $(OBJS)
	g++ $< -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $<

$(foreach SRC,$(SRCS),$(eval $(subst \,,$(shell $(CXX) -MM $(SRC) $(CXXFLAGS)))))

