
OPENCV = `pkg-config opencv4 --cflags --libs`
FINDER = -Iinclude -Llib -lirisFinder
CXX    = g++ -std=c++11 -Iinclude -L/usr/local/lib

all: lib/libIrisFinder.so bin/localize

lib/libIrisFinder.so: src/irisFinder.cpp src/irisBoundary.cpp include/*.h
	$(CXX) $(OPENCV) -shared src/irisBoundary.cpp src/irisFinder.cpp -o $@

bin/localize: src/localize.cpp
	$(CXX) $(OPENCV) $(FINDER) -lbiomeval $< -o $@

clean:
	rm -f lib/libIrisFinder.so bin/localize

