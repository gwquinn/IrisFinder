
OPENCV = `pkg-config opencv4 --cflags --libs`
# -GW SFML   = `pkg-config sfml-graphics --cflags --libs`
FINDER = -I../include -L../lib -lirisFinder
CXX    = clang++ -fPIC -std=c++11 -I../include -I$(IRIS)/builds/libbiomeval/src/include \
                 -isysroot `xcrun --show-sdk-path` -L/usr/local/lib

all: ../lib/libIrisFinder.so ../bin/localize

../lib/libIrisFinder.so: irisFinder.cpp irisBoundary.cpp ../include/*.h
	$(CXX) $(OPENCV) -DNDEBUG -shared irisBoundary.cpp irisFinder.cpp -o $@

../bin/localize: localize.cpp
	$(CXX) $(OPENCV) $(FINDER) -lbiomeval $< -o $@

clean:
	rm -f ../lib/libIrisFinder.so ../bin/localize


