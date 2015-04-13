CXX = g++
CFLAGS = -O0 -g -Wall -Wextra -pedantic -std=c++11 -fopenmp

FILES = main.cpp grabber_class.cpp test.cpp
EXTFILES = auxf.cpp threadhand_class.cpp
FILES_MINIME = minime_class.cpp fifo_class.cpp fit_fun.cpp funcs.cpp gabe_class.cpp minimizer_s.cpp random.cpp
FILES_VIEWER = draw.cpp viewer_class.cpp

PROJECT = grabber

ifdef windir
 delcall = del
 binfix = .exe
 pathfix = $(subst /,\,$1)
 OSID = 1
 CDIR = -IC:\MinGW\mingw64\opencv\x64\mingw\bin -IC:\MinGW\mingw64\opencv\include
 LDIR = -LC:\MinGW\mingw64\opencv\x64\mingw\lib
 LFLAGS = -lm -lopencv_core2410 -lopencv_highgui2410 -lopencv_imgproc2410 -lopengl32 -lglu32 -lfreeglut -fopenmp
 CAMLIB = -luc480_64
else
 ifeq ($(shell uname), Linux)
  delcall = rm -f
  binfix =
  pathfix = $1
  OSID = 0
  CDIR =
  LDIR =
  LFLAGS = -lm -lopencv_core -lopencv_highgui -lopencv_imgproc -lGL -lGLU -lglut -fopenmp
  CAMLIB = -lueye
 else
  ifneq ($(shell uname), Linux)
   $(error OS not identified)
  endif
 endif
endif

ODIR = obj
SDIR = src
EXTSDIR = ../src
VIEWERSDIR = ../viewer_pp/src
MINIMESDIR = ../minime_pp/src

_OBJECTS = $(patsubst %.cpp, %.o, $(FILES)) \
$(patsubst %.cpp, %.o, $(EXTFILES)) \
$(patsubst %.cpp, %.o, $(FILES_MINIME)) \
$(patsubst %.cpp, %.o, $(FILES_VIEWER))

OBJECTS = $(patsubst %, $(ODIR)/%, $(_OBJECTS))

ifeq ($(OSID),1)
 DELOBJECTS := $(patsubst %, $(ODIR)\\%, $(_OBJECTS))
else
 DELOBJECTS := $(OBJECTS)
endif

$(PROJECT): $(OBJECTS)
	$(CXX) $(LDIR) -o $@ $^ $(LFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp Makefile
	$(CXX) $(CFLAGS) $(CDIR) -c $< -o $@ $(LFLAGS)

$(ODIR)/%.o: $(EXTSDIR)/%.cpp $(EXTSDIR)/header.hpp
	$(CXX) $(CFLAGS) -c $< -o $@ $(LFLAGS)

$(ODIR)/%.o: $(VIEWERSDIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ $(LFLAGS)

$(ODIR)/%.o: $(MINIMESDIR)/%.cpp
	$(CXX) $(CFLAGS) $(CDIR) -c $< -o $@ $(LFLAGS)

$(OBJECTS): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

clean:
	$(delcall) $(PROJECT)$(binfix) $(DELOBJECTS)
