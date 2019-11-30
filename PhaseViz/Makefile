CXX=g++
CXXFLAGS=-I.
LDFLAGS=-L/usr/lib64 -lGL -lGLEW -lglut -lGLU
DEPS =
OBJ = main.o RenderGL.o ThreeBodySolver.o 

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

PhaseViz: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

