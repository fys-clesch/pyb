CC = g++

CFLAGS = -O0 -g -Wall -Wextra -pedantic -std=c++14 -fopenmp
LFLAGS = -lm -static-libstdc++ -static-libgcc

FILES = main.cpp fifo_class.cpp \
        fit_fun_profile.cpp fit_fun_propagation.cpp funcs.cpp gabe_class.cpp \
        minime_profile_class.cpp minime_propagation_class.cpp minimizer_s.cpp \
        random.cpp

EXTFILES = auxf.cpp

PROJECT = minime

ifdef windir
 delcall = del
 binfix = .exe
 pathfix = $(subst /,\,$1)
 OSID = 1
 CDIR = -IC:\Libraries\OpenCV\include
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
LDIR = C:/Libraries/OpenCV/include

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
	$(CC) $(CFLAGS) $(CDIR) -c $< -o $@ $(LFLAGS)

$(ODIR)/%.o: $(EXTSDIR)/%.cpp $(EXTSDIR)/header.hpp Makefile
	$(CC) $(CFLAGS) $(CDIR) -c $< -o $@ $(LFLAGS)

$(OBJECTS): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

clean:
	$(delcall) $(PROJECT)$(binfix) $(DELOBJECTS)
