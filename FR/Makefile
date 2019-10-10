
CPPC:= aarch64-linux-gnu-g++ -std=c++11 -O2  -fopenmp
CC:= aarch64-linux-gnu-g++ -std=c++11 -O2   -fopenmp

TEST_SRC:=Samplecode/testSdkCompile.cpp

APP_NAME:=wfFRTestSample

# define source path
VPATH:=Samplecode

#create object names for all source

TEST_OBJS:=$(patsubst %.cpp, Samplecode/%.o, $(notdir $(TEST_SRC)))

print-%  : ; @echo $* = $($*)		#make print-STASM_OBJ

.PHONY: clean install

all: $(TEST_SRC) $(APP_NAME) 

Samplecode/%.o:  %.cpp
	 $(CC) -Iinclude/  $(CPPFLAGS) -c -o $@ $<

Samplecode/%.o:  %.c
	 $(CC) -Iinclude/  $(CPPFLAGS) -c -o $@ $<


$(APP_NAME): $(TEST_OBJS) 
	 $(CC) $(TEST_OBJS)  -o $@ -L./  -lwfFR  -Wl,-rpath=../drivers/ -L../drivers/ -L../ -lrknn_runtime


clean:
	rm Samplecode/*.o
	rm $(APP_NAME)
