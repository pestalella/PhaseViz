CXX=g++
CXXFLAGS=-Iinclude -std=c++1y -O3 -fopenmp
LDFLAGS=-L/usr/lib64 -lGL -lGLEW -lglut -lGLU
#DEPS =
#OBJ = main.o RenderGL.o ThreeBodySolver.o

.PHONY: all clean

SDIR = src
BUILDDIR = build
SOURCES = $(wildcard $(SDIR)/*.cpp)
_OBJ = $(patsubst %.cpp,%.o,$(SOURCES))
OBJ = $(patsubst $(SDIR)/%,$(BUILDDIR)/%,$(_OBJ))

$(info OBJ=$(OBJ))

all: $(BUILDDIR) PhaseViz

$(BUILDDIR)/%.o: $(SDIR)/%.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

PhaseViz: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
