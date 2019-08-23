CXX = g++

ifndef DEBUG
 DEBUG = 1
endif

ifndef STATIC
 STATIC = 0
endif

ifeq ($(DEBUG), 1)
 CFLAGS = -O0 -g -Wall -Wextra -pedantic -std=c++11 -fopenmp
else
 ifeq ($(STATIC), 1)
  CFLAGS = -O3 -Wall -Wextra -pedantic -std=c++11 -fopenmp -DIGYBA_NDEBUG -DNDEBUG -DFREEGLUT_STATIC
 else
  CFLAGS = -O3 -Wall -Wextra -pedantic -std=c++11 -fopenmp -DIGYBA_NDEBUG -DNDEBUG
 endif
endif

FILES = main.cpp
EXTFILES = auxf.cpp threadhand_class.cpp
FILES_MINIME = minime_profile_class.cpp minime_propagation_class.cpp fifo_class.cpp fit_fun_profile.cpp funcs.cpp gabe_class.cpp minimizer_s.cpp random.cpp
FILES_VIEWER = draw.cpp viewer_class.cpp
FILES_GRABBER = grabber_class.cpp
FILES_TC = thorlabs_cam.cpp
RESOURCES = igyba_res.rc

PROJECT = igyba_thorlabs

ifdef windir
 delcall = del
 binfix = .exe
 pathfix = $(subst /,\,$1)
 OSID = 1
 CAMLIB = -luc480_64
 ifeq ($(STATIC), 1)
  CDIR = -IC:\MinGW\mingw64\opencv_static\x64\mingw\bin -IC:\MinGW\mingw64\opencv_static\include
  LDIR = -LC:\MinGW\mingw64\opencv_static\x64\mingw\staticlib
  # -static
  LFLAGS = -s -fopenmp -lm -static-libgcc -static-libstdc++ -L libgcc_s.a \
  -lopencv_highgui2411 -lopencv_imgproc2411 -lopencv_core2411 \
  -lIlmImf -llibpng -llibjasper -llibjpeg -llibtiff -lzlib \
  -lfreeglut_static -lcomctl32 -lgdi32 -lglu32 -lopengl32 \
  -lole32 -loleaut32 -luuid -lVfw32 -L libwinpthread.a -L libgomp.a -lwinmm
 else
  CDIR = -IC:\MinGW\mingw64\opencv\x64\mingw\bin -IC:\MinGW\mingw64\opencv\include
  LDIR = -LC:\MinGW\mingw64\opencv\x64\mingw\lib
  LFLAGS = -lm -lopencv_core2411 -lopencv_highgui2411 -lopencv_imgproc2411 -lopengl32 -lglu32 -lfreeglut -fopenmp
 endif
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

# define the different paths of objects and sources
ODIR = obj
SDIR = .
EXT_ODIR = obj/src
EXT_SDIR = ../src
VIEWER_SDIR = ../viewer_pp/src
VIEWER_ODIR = obj/viewer_pp
MINIME_SDIR = ../minime_pp/src
MINIME_ODIR = obj/minime_pp
GRABBER_SDIR = ../grabber/src
GRABBER_ODIR = obj/grabber
TC_SDIR = ../thorlabs_cam
TC_ODIR = obj/thorlabs_cam

# create templates for the objects
_OBJECTS = $(patsubst %.cpp, %.o, $(FILES))
_OBJECTS_EXT = $(patsubst %.cpp, %.o, $(EXTFILES))
_OBJECTS_VIEWER = $(patsubst %.cpp, %.o, $(FILES_VIEWER))
_OBJECTS_MINIME = $(patsubst %.cpp, %.o, $(FILES_MINIME))
_OBJECTS_GRABBER = $(patsubst %.cpp, %.o, $(FILES_GRABBER))
_OBJECTS_TC = $(patsubst %.cpp, %.o, $(FILES_TC))

# combine the source- and the file names
SOURCE_EXT = $(addprefix $(EXT_SDIR)/, $(EXTFILES))
SOURCE_VIEWER = $(addprefix $(VIEWER_SDIR)/, $(FILES_VIEWER))
SOURCE_MINIME = $(addprefix $(MINIME_SDIR)/, $(FILES_MINIME))
SOURCE_GRABBER = $(addprefix $(GRABBER_SDIR)/, $(FILES_GRABBER))
SOURCE_TC = $(addprefix $(TC_SDIR)/, $(FILES_TC))

# combine the object- and the file names
OBJECTS = $(addprefix $(ODIR)/, $(_OBJECTS))
OBJECTS_EXT = $(addprefix $(EXT_ODIR)/, $(_OBJECTS_EXT))
OBJECTS_VIEWER = $(addprefix $(VIEWER_ODIR)/, $(_OBJECTS_VIEWER))
OBJECTS_MINIME = $(addprefix $(MINIME_ODIR)/, $(_OBJECTS_MINIME))
OBJECTS_GRABBER = $(addprefix $(GRABBER_ODIR)/, $(_OBJECTS_GRABBER))
OBJECTS_TC = $(addprefix $(TC_ODIR)/, $(_OBJECTS_TC))

# add the objects all up
OBJECTS_ALL = $(OBJECTS) \
$(OBJECTS_TC) \
$(OBJECTS_GRABBER) \
$(OBJECTS_VIEWER) \
$(OBJECTS_MINIME) \
$(OBJECTS_EXT)

# define the resources
RES = $(patsubst %.rc, %.res, $(RESOURCES))

# define the objects to be deleted
ifeq ($(OSID), 1)
 DELOBJECTS := $(OBJECTS_ALL) $(RES)
else
 DELOBJECTS := $(OBJECTS_ALL)
endif

# define the dependencies
ifeq ($(OSID), 1)
 DEPENDS = $(OBJECTS_ALL) $(RES)
else
 DEPENDS = $(OBJECTS_ALL)
endif

$(PROJECT): $(DEPENDS)
	$(CXX) $(LDIR) -o $@$(binfix) $^ $(CAMLIB) $(LFLAGS)

$(RES): $(RESOURCES)
	windres $^ -O coff -o $@

$(ODIR)/%.o: $(SDIR)/%.cpp $(RESOURCES) Makefile
	$(CXX) $(CFLAGS) $(CDIR) -c $< -o $@

$(EXT_ODIR)/%.o: $(EXT_SDIR)/%.cpp $(EXT_SDIR)/header.hpp
	$(CXX) $(CFLAGS) $(CDIR) -c $< -o $@

$(VIEWER_ODIR)/%.o: $(VIEWER_SDIR)/%.cpp $(EXT_SDIR)/header.hpp
	$(CXX) $(CFLAGS) $(CDIR) -c $< -o $@

$(MINIME_ODIR)/%.o: $(MINIME_SDIR)/%.cpp $(EXT_SDIR)/header.hpp
	$(CXX) $(CFLAGS) $(CDIR) -c $< -o $@

$(GRABBER_ODIR)/%.o: $(GRABBER_SDIR)/%.cpp $(EXT_SDIR)/header.hpp
	$(CXX) $(CFLAGS) $(CDIR) -c $< -o $@

$(TC_ODIR)/%.o: $(TC_SDIR)/%.cpp $(EXT_SDIR)/header.hpp
	$(CXX) $(CFLAGS) $(CDIR) -c $< -o $@

# create the folders
$(OBJECTS): | $(ODIR)

$(OBJECTS_EXT): | $(EXT_ODIR)

$(OBJECTS_VIEWER): | $(VIEWER_ODIR)

$(OBJECTS_MINIME): | $(MINIME_ODIR)

$(OBJECTS_GRABBER): | $(GRABBER_ODIR)

$(OBJECTS_TC): | $(TC_ODIR)

$(ODIR):
	mkdir $(call pathfix, $@)

$(EXT_ODIR):
	mkdir $(call pathfix, $@)

$(VIEWER_ODIR):
	mkdir $(call pathfix, $@)

$(MINIME_ODIR):
	mkdir $(call pathfix, $@)

$(GRABBER_ODIR):
	mkdir $(call pathfix, $@)

$(TC_ODIR):
	mkdir $(call pathfix, $@)

# clean it all up
clean:
	$(delcall) $(PROJECT)$(binfix) $(call pathfix, $(DELOBJECTS))
