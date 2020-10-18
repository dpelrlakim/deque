CXX = g++
CXXFLAGS = -std=c++14 -Wall -MMD -g
EXEC = main-deque
OBJECTS = dequeMain.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${DEPENDS} ${EXEC}
