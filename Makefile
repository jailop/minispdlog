CXXFLAGS = -std=c++14 -Wall -Wextra -pedantic

all: example test_minispdlog

clean:
	rm -f example
