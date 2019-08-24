CXX = g++

ifndef DEBUG
 DEBUG = 1
endif

ifndef STATIC
 STATIC = 0
endif

ifeq ($(DEBUG), 1)
    CFLAGS := -O0 -g -Wall -Wextra -pedantic -std=c++11 -fopenmp -DUSE_DEPRECATED_FUNCTIONS # Whitespace
else
    CFLAGS := -O3 -Wall -Wextra -pedantic -std=c++11 -fopenmp -DIGYBA_NDEBUG -DNDEBUG -DUSE_DEPRECATED_FUNCTIONS # Whitespace
endif

FILES_MAIN = igyba_wxApp.cpp igyba_wxMain.cpp src/led.cpp
FILES_EXT = auxf.cpp threadhand_class.cpp
FILES_MINIME = minime_profile_class.cpp minime_propagation_class.cpp fifo_class.cpp fit_fun_profile.cpp funcs.cpp gabe_class.cpp minimizer_s.cpp random.cpp
FILES_VIEWER = draw.cpp viewer_class.cpp
FILES_GRABBER = grabber_class.cpp
FILES_TC = thorlabs_cam.cpp
FILES_IC = ids_cam.cpp
RESOURCES = resource.rc

PROJECT = igyba_wx

ifdef windir
    delcall = del
    binfix = .exe
    pathfix = $(subst /,\,$1)
    OSID = 1
    CAMLIB = "C:\Program Files\IDS\uEye\Develop\Lib\uEye_api_64.lib"
    WXFLAGS := -D__WXMSW__ -DWXUSINGDLL -DwxUSE_UNICODE # Whitespace
    CFLAGS := $(CFLAGS)$(WXFLAGS)
    RESFLAGS = -IC:\Libraries\wxWidgets\lib\gcc_x64_dll\mswu -IC:\Libraries\wxWidgets\include
    ifeq ($(STATIC), 1)
        STATIC_FLAGS := -DFREEGLUT_STATIC # Whitespace
        CFLAGS := $(CFLAGS)$(STATIC_FLAGS)
        CDIR = -IC:\Libraries\OpenCV_static\include \
               -IC:\Libraries\freeglut\x86_64-w64-mingw32\include \
               -IC:\Libraries\wxWidgets\include \
               -IC:\Libraries\wxWidgets\lib\gcc_x64_dll\mswu
        LDIR = -LC:\Libraries\OpenCV_static\x64\mingw\staticlib \
               -LC:\Libraries\freeglut\x86_64-w64-mingw32\lib \
               -LC:\Libraries\wxWidgets\lib\gcc_x64_dll
        LFLAGS = -s -lm -fopenmp -mthreads -static-libgcc -static-libstdc++ -L libgcc_s.a \
                 -lopencv_highgui411 -lopencv_imgcodecs411 -lopencv_imgproc411 -lopencv_videoio411 -lopencv_core411 \
                 -lIlmImf -llibpng -llibjasper -llibjpeg-turbo -llibtiff -lzlib -llibwebp \
                 -lfreeglut_static -lglu32 -lopengl32 \
                 -lwxmsw31u_richtext -lwxmsw31u_xrc -lwxmsw31u_aui -lwxmsw31u_media -lwxbase31u_net -lwxmsw31u_gl -lwxbase31u_xml -lwxmsw31u_adv -lwxmsw31u_html -lwxmsw31u_core -lwxbase31u \
                 -lcomdlg32  -lgdi32 -lwinmm
    else
        CDIR = -IC:\Libraries\OpenCV\include \
               -IC:\Libraries\freeglut\x86_64-w64-mingw32\include \
               -IC:\Libraries\wxWidgets\include \
               -IC:\Libraries\wxWidgets\lib\gcc_x64_dll\mswu
        LDIR = -LC:\Libraries\OpenCV\x64\mingw\lib \
               -LC:\Libraries\freeglut\x86_64-w64-mingw32\lib \
               -LC:\Libraries\wxWidgets\lib\gcc_x64_dll
        LFLAGS = -lm -fopenmp -mthreads \
                 -lopencv_highgui411 -lopencv_imgcodecs411 -lopencv_imgproc411 -lopencv_videoio411 -lopencv_core411 \
                 -lwxmsw31u_richtext -lwxmsw31u_xrc -lwxmsw31u_aui -lwxmsw31u_media -lwxbase31u_net -lwxmsw31u_gl -lwxbase31u_xml -lwxmsw31u_adv -lwxmsw31u_html -lwxmsw31u_core -lwxbase31u \
                 -lfreeglut -lglu32 -lopengl32
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
        CAMLIB = -luEye_api_64
    else
        ifneq ($(shell uname), Linux)
            $(error OS not identified)
        endif
    endif
endif

# Define the different paths of objects and sources
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
IC_SDIR = ../ids_cam
IC_ODIR = obj/ids_cam

# Create templates for the objects
_OBJECTS_MAIN = $(patsubst %.cpp, %.o, $(FILES_MAIN))
_OBJECTS_EXT = $(patsubst %.cpp, %.o, $(FILES_EXT))
_OBJECTS_VIEWER = $(patsubst %.cpp, %.o, $(FILES_VIEWER))
_OBJECTS_MINIME = $(patsubst %.cpp, %.o, $(FILES_MINIME))
_OBJECTS_GRABBER = $(patsubst %.cpp, %.o, $(FILES_GRABBER))
_OBJECTS_TC = $(patsubst %.cpp, %.o, $(FILES_TC))
_OBJECTS_IC = $(patsubst %.cpp, %.o, $(FILES_IC))

# Combine the source- and the file names
SOURCE_EXT = $(addprefix $(EXT_SDIR)/, $(FILES_EXT))
SOURCE_VIEWER = $(addprefix $(VIEWER_SDIR)/, $(FILES_VIEWER))
SOURCE_MINIME = $(addprefix $(MINIME_SDIR)/, $(FILES_MINIME))
SOURCE_GRABBER = $(addprefix $(GRABBER_SDIR)/, $(FILES_GRABBER))
SOURCE_TC = $(addprefix $(TC_SDIR)/, $(FILES_TC))
SOURCE_IC = $(addprefix $(IC_SDIR)/, $(FILES_IC))

# Combine the object- and the file names
OBJECTS_MAIN = $(addprefix $(ODIR)/, $(_OBJECTS_MAIN))
OBJECTS_EXT = $(addprefix $(EXT_ODIR)/, $(_OBJECTS_EXT))
OBJECTS_VIEWER = $(addprefix $(VIEWER_ODIR)/, $(_OBJECTS_VIEWER))
OBJECTS_MINIME = $(addprefix $(MINIME_ODIR)/, $(_OBJECTS_MINIME))
OBJECTS_GRABBER = $(addprefix $(GRABBER_ODIR)/, $(_OBJECTS_GRABBER))
OBJECTS_TC = $(addprefix $(TC_ODIR)/, $(_OBJECTS_TC))
OBJECTS_IC = $(addprefix $(IC_ODIR)/, $(_OBJECTS_IC))

# Add the objects all up
OBJECTS_ALL = $(OBJECTS_MAIN) \
              $(OBJECTS_TC) \
              $(OBJECTS_IC) \
              $(OBJECTS_GRABBER) \
              $(OBJECTS_VIEWER) \
              $(OBJECTS_MINIME) \
              $(OBJECTS_EXT)

# Define the resources
RES = $(patsubst %.rc, %.res, $(RESOURCES))

# Define the objects to be deleted
ifeq ($(OSID), 1)
    DELOBJECTS := $(OBJECTS_ALL) $(RES)
else
    DELOBJECTS := $(OBJECTS_ALL)
endif

# Define the dependencies
ifeq ($(OSID), 1)
    DEPENDS = $(OBJECTS_ALL) $(RES)
else
    DEPENDS = $(OBJECTS_ALL)
endif

$(PROJECT): $(DEPENDS)
	$(CXX) $(LDIR) -o $@$(binfix) $^ $(LFLAGS) $(CAMLIB)

$(RES): $(RESOURCES)
	windres $(RESFLAGS) $^ -O coff -o $@

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

$(IC_ODIR)/%.o: $(IC_SDIR)/%.cpp $(EXT_SDIR)/header.hpp
	$(CXX) $(CFLAGS) $(CDIR) -c $< -o $@

# Create the folders
$(OBJECTS_MAIN): | $(ODIR)

$(OBJECTS_EXT): | $(EXT_ODIR)

$(OBJECTS_VIEWER): | $(VIEWER_ODIR)

$(OBJECTS_MINIME): | $(MINIME_ODIR)

$(OBJECTS_GRABBER): | $(GRABBER_ODIR)

$(OBJECTS_TC): | $(TC_ODIR)

$(OBJECTS_IC): | $(IC_ODIR)

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

$(IC_ODIR):
	mkdir $(call pathfix, $@)

# Clean it all up
clean:
	$(delcall) $(PROJECT)$(binfix) $(call pathfix, $(DELOBJECTS))
