CXX = g++
CXXFLAGS = -g -O2 -Wall -std=c++11
LDFLAGS = -g
LIBS = -lm
HDRS=bmp_io.h  light_source.h  raytracer.h  scene_object.h  util.h

raytracer:  raytracer.o util.o light_source.o scene_object.o bmp_io.o util.o main.cpp
	$(CXX) $(CXXFLAGS) -o raytracer main.cpp \
	raytracer.o util.o light_source.o scene_object.o bmp_io.o $(LIBS)

%.o: %.cpp $(HDRS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	-rm -f core *.o
	-rm raytracer
	



