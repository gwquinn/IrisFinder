
OPENCV = `pkg-config opencv4 --cflags --libs`
FINDER = -I../include -L../lib -lirisFinder
CXX    = g++ -std=c++11 -I../include -L/usr/local/lib

all: ../lib/libIrisFinder.so ../bin/localize

../lib/libIrisFinder.so: irisFinder.cpp irisBoundary.cpp ../include/*.h
	$(CXX) $(OPENCV) -shared irisBoundary.cpp irisFinder.cpp -o $@

../bin/localize: localize.cpp
	$(CXX) $(OPENCV) $(FINDER) -lbiomeval $< -o $@

clean:
	rm -f ../lib/libIrisFinder.so ../bin/localize

