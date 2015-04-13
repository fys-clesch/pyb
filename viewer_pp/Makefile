CC = g++
CFLAGS = -O0 -g -Wall -Wextra -pedantic -std=c++11 -fopenmp

FILES = main.cpp draw.cpp viewer_class.cpp
EXTFILES = auxf.cpp threadhand_class.cpp
PROJECT = viewer

ifdef windir
 delcall = del
 binfix = .exe
 pathfix = $(subst /,\,$1)
 OSID = 1
 LFLAGS = -lm -lopengl32 -lglu32 -lfreeglut -fopenmp
else
 ifeq ($(shell uname), Linux)
  delcall = rm -f
  binfix =
  pathfix = $1
  OSID = 0
  LFLAGS = -lm -lGL -lGLU -lglut -fopenmp
 else
  ifneq ($(shell uname), Linux)
   $(error OS not identified)
  endif
 endif
endif

ODIR = obj
SDIR = src
EXTSDIR = ../src

_OBJECTS = $(patsubst %.cpp, %.o, $(FILES)) $(patsubst %.cpp, %.o, $(EXTFILES))

OBJECTS = $(patsubst %, $(ODIR)/%, $(_OBJECTS)) $(patsubst %, $(ODIR)/%, $(_EXTOBJECTS))

ifeq ($(OSID),1)
 DELOBJECTS := $(patsubst %, $(ODIR)\\%, $(_OBJECTS))
else
 DELOBJECTS := $(OBJECTS)
endif

$(PROJECT): $(OBJECTS)
	$(CC) -o $@ $^ $(LFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(SDIR)/$(PROJECT)_class.h Makefile
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

$(ODIR)/%.o: $(EXTSDIR)/%.cpp $(EXTSDIR)/header.hpp Makefile
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

$(OBJECTS): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

clean:
	$(delcall) $(PROJECT)$(binfix) $(DELOBJECTS)
