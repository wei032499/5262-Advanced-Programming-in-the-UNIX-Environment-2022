CC := g++ -g
exe := hw1
OBJS := file_info.o proc_info.o hw1.o

all: $(OBJS)
	$(CC) $^ -o $(exe)

%.o: %.cpp
	$(CC) -c $? -I . -o $@

.PHONY: clean
clean:
	rm -f $(OBJS)