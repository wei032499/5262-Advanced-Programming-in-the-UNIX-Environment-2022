CC := g++
EXE := logger
EXE_OBJS := hw2.o

all: $(EXE) logger.so

$(EXE): $(EXE_OBJS)
	$(CC) $? -o $(EXE)

logger.so: logger.cpp
	$(CC) $? -o $@ -ldl -shared -fPIC

%.o: %.cpp
	$(CC) -c $? -I . -o $@

.PHONY: clean clean_all

clean:
	rm -f $(EXE_OBJS)

clean_all:
	rm -f $(EXE_OBJS) $(EXE) logger.so