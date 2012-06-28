include Makefile.mk

executable = sc
lib_dir = lib
include_dirs = $(lib_dir:%=-I%) -Iinclude
lib_sources = $(wildcard $(lib_dir)/*/*.cc)
lib_objects = $(lib_sources:.cc=.o)
pbo_objects = $(lib_objects) main.pbo.o controller.pbo.o
proxy_objects = $(lib_objects) main.o controller.o
cpu_objects = $(lib_objects) main.cpu.o controller.cpu.o

ifeq ($(TARGET),linux)
shared_libs = -lopencv_core -lopencv_imgproc -lopencv_highgui \
    -lGL -lGLU -lGLEW -lglfw -lpthread
all: $(executable).proxy $(executable).pbo $(executable).cpu
$(executable).proxy: $(proxy_objects)
	$(CXX) -o $@ $(proxy_objects) $(shared_libs)
$(executable).pbo: $(pbo_objects)
	$(CXX) -o $@ $(pbo_objects) $(shared_libs)
$(executable).cpu: $(cpu_objects)
	$(CXX) -o $@ $(cpu_objects) $(shared_libs)
else
shared_libs = -s -lopencv_core240 -lopencv_imgproc240 -lopencv_highgui240 \
    -luser32 -lkernel32 -lGLEW32 -lglfw -lglu32 -lopengl32
all: $(executable).proxy.exe $(executable).pbo.exe $(executable).cpu.exe
$(executable).proxy.exe: $(proxy_objects)
	$(CXX) -o $@ $(proxy_objects) $(shared_libs)
$(executable).pbo.exe: $(pbo_objects)
	$(CXX) -o $@ $(pbo_objects) $(shared_libs)
$(executable).cpu.exe: $(cpu_objects)
	$(CXX) -o $@ $(cpu_objects) $(shared_libs)
endif

$(lib_dir): FORCE
	$(MAKE) -C $@ all

clean:
	$(MAKE) -C $(lib_dir) clean
	$(RM) *.o
	$(RM) $(executable).*

FORCE:

.PHONY: FORCE all
.DEFAULT: all
