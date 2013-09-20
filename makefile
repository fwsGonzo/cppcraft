#######################
#  cppcraft makefile  #
#######################

# code folders
SOURCE_DIR  = source
SOURCE_DIRS = . tests
LIBRARY_DIRS = library library/bitmap library/compression library/math     \
				library/network library/noise library/opengl library/sound \
				library/threading library/timing library/voxels
ifeq ($(OS),Windows_NT)
RESOURCES = res/cppcraft.rc
endif

# build options
# -Ofast -msse4.1 -ffast-math -mfpmath=both
BUILDOPT = -g
# output file
OUTPUT   = ./Debug/cppcraft

##############################################################

# compiler
CC = g++ $(BUILDOPT) -std=c++11
# compiler flags
CCFLAGS = -c -Wall -Wno-write-strings -Iinc
# linker flags
ifeq ($(OS),Windows_NT)
	LFLAGS  = -Llib -static -lpthread -lbassdll -lglfw3 -lgdi32 -lopengl32 -llzo2 -lws2_32
else
	LFLAGS  = -Llib -lpthread -lbass -llzo2 -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lXi
endif
# resource builder
RES = windres
# resource builder flags
RFLAGS = -O coff

##############################################################

# make pipeline
DIRECTORIES = $(LIBRARY_DIRS) $(SOURCE_DIRS)
CCDIRS  = $(foreach dir, $(DIRECTORIES), $(SOURCE_DIR)/$(dir)/*.c)
CCMODS  = $(wildcard $(CCDIRS))
CXXDIRS = $(foreach dir, $(DIRECTORIES), $(SOURCE_DIR)/$(dir)/*.cpp)
CXXMODS = $(wildcard $(CXXDIRS))

# compile each .c to .o
.c.o:
	$(CC) $(CCFLAGS) $< -o $@

# compile each .cpp to .o
.cpp.o:
	$(CC) $(CCFLAGS) $< -o $@

# recipe for building .o from .rc files
%.o : %.rc
	$(RES) $(RFLAGS) $< -o $@

# convert .c to .o
CCOBJS  = $(CCMODS:.c=.o)
# convert .cpp to .o
CXXOBJS = $(CXXMODS:.cpp=.o)
# resource .rc to .o
CCRES   = $(RESOURCES:.rc=.o)

# link all OBJS using CC and link with LFLAGS, then output to OUTPUT
all: $(CXXOBJS) $(CCOBJS) $(CCRES)
	$(CC) $(CXXOBJS) $(CCOBJS) $(CCRES) $(LFLAGS) -o $(OUTPUT)

# remove each known .o file, and output
clean:
	$(RM) $(CXXOBJS) $(CCOBJS) $(CCRES) *~ $(OUTPUT).*
