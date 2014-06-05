CPP=g++ -g
EXE=hdram
OBJS=core.o controller.o dram.o hdram.o request.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CPP) $^ -o $@

%.o: %.cpp
	$(CPP) -c $< -o $@

clean:
	rm -rf $(EXE) $(OBJS)
