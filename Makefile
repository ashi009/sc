include Makefile.mk

executable = main
lib_dir = lib
include_dirs = $(lib_dir:%=-I%) -Iinclude
lib_sources = $(wildcard $(lib_dir)/*/*.cc)
lib_objects = $(lib_sources:.cc=.o)
shared_libs = -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml \
    -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect \
    -lopencv_contrib -lopencv_legacy -lopencv_flann -lGL -lGLU -lGLEW -lglfw \
    -lpthread

all: $(executable)

$(executable): $(lib_dir) $(lib_objects) $(objects)
	$(CXX) -o $@ $(objects) $(lib_objects) $(shared_libs)

$(lib_dir): force_check
	$(MAKE) -C $@ all

clean:
	$(MAKE) -C $(lib_dir) clean
	$(RM) $(executable)
	$(RM) *.o

.PHONY: all
