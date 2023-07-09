CXX = g++
CXXFLAGS = -I$(PWD) -O3 -g -std=c++17 -I"./third_party"

SRCS =  main.cpp ./third_party/FractionClass/Fraction.cpp cycle/Cycle.cpp tests/tst_cycle.cpp tests/tst_renderNode.cpp
OBJS = ${SRCS:.cpp=.o}
HEADERS =

BUILDDIR=$(PWD)/build

MAIN = main.exe

all: ${MAIN}
	@echo Done

${MAIN}: ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} -o ${MAIN}

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	${RM} ${PROGS} ${OBJS} *.o *~.
