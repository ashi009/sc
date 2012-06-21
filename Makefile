include Makefile.mk

executable = main
lib_dir = lib
include_dirs = $(lib_dir:%=-I%) -Iinclude
lib_sources = $(wildcard $(lib_dir)/*/*.cc)
lib_objects = $(lib_sources:.cc=.o)
pbo_objects = $(lib_objects) main.o controller_pbo.o
nopbo_objects = $(lib_objects) main.o controller_nopbo.o

ifeq ($(TARGET),linux)
shared_libs = -lopencv_core -lopencv_imgproc -lopencv_highgui \
    -lGL -lGLU -lGLEW -lglfw -lpthread
all: $(executable)-nopbo $(executable)-pbo
$(executable)-nopbo: $(nopbo_objects)
	$(CXX) -o $@ $(nopbo_objects) $(shared_libs)
$(executable)-pbo: $(lib_dir) $(lib_objects) $(pbo_objects)
	$(CXX) -o $@ $(pbo_objects) $(shared_libs)
else
shared_libs = -s -lopencv_core240 -lopencv_imgproc240 -lopencv_highgui240 \
    -luser32 -lkernel32 -lGLEW32 -lglfw -lglu32 -lopengl32
all: $(executable)-nopbo.exe $(executable)-pbo.exe
$(executable)-nopbo.exe:$(nopbo_objects)
	$(CXX) -o $@ $(nopbo_objects) $(shared_libs)
$(executable)-pbo.exe: $(pbo_objects)
	$(CXX) -o $@ $(pbo_objects) $(shared_libs)
endif

controller_pbo.o: controller.cc controller.h
	$(CC) $(CPPFLAGS) $(other_flags) -DPBO $(include_dirs) -c $< -o $@

controller_nopbo.o: controller.cc controller.h
	$(CC) $(CPPFLAGS) $(other_flags) $(include_dirs) -c $< -o $@

$(lib_dir): FORCE
	$(MAKE) -C $@ all

clean:
	$(MAKE) -C $(lib_dir) clean
	$(RM) $(executable)-*
	$(RM) *.o

FORCE:

.PHONY: FORCE all
.DEFAULT: all
