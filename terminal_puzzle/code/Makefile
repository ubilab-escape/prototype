CXX = g++ -g -Wall -pedantic -std=c++11
MAIN_BINARIES = $(basename $(wildcard *Main.cpp))
TEST_BINARIES = $(basename $(wildcard *Test.cpp))
HEADERS = $(wildcard *.h)
OBJECTS = $(addsuffix .o, $(basename $(filter-out %Main.cpp %Test.cpp, $(wildcard *.cpp))))
LIBRARIES =

.PRECIOUS: %.o
.SUFFIXES:
.PHONY: all compile checkstyle

all: compile checkstyle

compile: $(MAIN_BINARIES) $(TEST_BINARIES)

valtest: $(TEST_BINARIES)
	for T in $(TEST_BINARIES); do valgrind --leak-check=full ./$$T; done

test: $(TEST_BINARIES)
	for T in $(TEST_BINARIES); do ./$$T; done

checkstyle:
	python3 ../cpplint.py --repository=. *.h *.cpp

clean:
	rm -f *.o
	rm -f $(MAIN_BINARIES)
	rm -f $(TEST_BINARIES)

svclean:
	svn rm --force *.o
	svn rm --force $(MAIN_BINARIES)
	svn rm --force $(TEST_BINARIES)


%Main: %Main.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBRARIES) -lncurses

%Test: %Test.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBRARIES) -lncurses -lgtest -lgtest_main -pthread

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<
