
OPENCV = `pkg-config opencv4 --cflags --libs`
SFML   = `pkg-config sfml-graphics --cflags --libs`
FINDER = -I$(IRIS_FINDER)/include -L$(IRIS_FINDER)/lib -lirisFinder
CXX    = clang++ -fPIC -std=c++11 -I../include -I$(IRIS)/builds/libbiomeval/src/include \
                 -isysroot `xcrun --show-sdk-path` -L/usr/local/lib

all: $(IRIS_FINDER)/lib/libIrisFinder.so ../bin/overlay ../bin/localize

$(IRIS_FINDER)/lib/libIrisFinder.so: irisFinder.cpp irisBoundary.cpp ../include/*.h
	$(CXX) $(OPENCV) -shared irisBoundary.cpp irisFinder.cpp -o $@

../bin/overlay: overlay.cpp
	$(CXX) $< $(SFML) $(OPENCV) $(FINDER) -o $@

../bin/localize: localize.cpp
	$(CXX) $(OPENCV) $(FINDER) -lbiomeval $< -o $@

clean:
	rm -f $(IRIS_FINDER)/lib/libIrisFinder.so ../bin/overlay ../bin/localize


