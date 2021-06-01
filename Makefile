CXX = g++
CFLAGS = -g -Wall -std=c++20
LIBS = -lpthread -lboost_program_options -lboost_thread
SRCS = lineCounter.cpp main.cpp
PROGRAM = lineCounter

$(PROGRAM): $(OBJS)
	$(CXX) $(CFLAGS) $(LIBS) $(SRCS) -o $(PROGRAM)
	@echo "LineCounter is compiled"

clean:
	rm -f $(PROGRAM)

