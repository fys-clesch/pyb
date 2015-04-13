CC = g++

CFLAGS = -O0 -g -Wall -Wextra -pedantic -std=c++11 -fopenmp -D__FIFO_NO_OPENCV__
LFLAGS = -lm

FILES = main.cpp fifo_class.cpp \
fit_fun.cpp funcs.cpp gabe_class.cpp \
minime_class.cpp minimizer_s.cpp random.cpp

EXTFILES = auxf.cpp
PROJECT = minime

ifdef windir
 delcall = del
 binfix = .exe
 pathfix = $(subst /,\,$1)
 OSID = 1
else
 ifeq ($(shell uname), Linux)
  delcall = rm -f
  binfix =
  pathfix = $1
  OSID = 0
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
	$(CC) -fopenmp -o $@ $^ $(LFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(SDIR)/$(PROJECT)_class.h Makefile
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

$(ODIR)/%.o: $(EXTSDIR)/%.cpp $(EXTSDIR)/header.hpp Makefile
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

$(OBJECTS): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

clean:
	$(delcall) $(PROJECT)$(binfix) $(DELOBJECTS)
