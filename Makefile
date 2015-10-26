CXX=clang++

SRCS=$(shell ls *.cpp)
APPS=$(subst .cpp,,$(SRCS))

CXXFLAGS= --std=c++11 -g

all: $(APPS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -rf $(APPS)

