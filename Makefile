.PHONY: relink

#
# This can be used as a template Makefile for analysis programs
# which uses the RootSorter library
#
#
# This is where you have installed the RootSorter 

BASEDIR = $(ROOTSORTER)

#
# TARGET is the basename of the file containing main()
# This should be a separate file 

TARGET   = jediwasa

#
# MODULES are all .cc/.hh file combinations containing your
# own classes except the ones which have to put in a
# shared library
# The ROOT linkage has to be specified in 'LinkDef.hh'

MODULES = JediPolAnalysis TreeTrack

#
# All classes which have to available later as a shared
# library have to be listed separately. Give here the final
# library names. The basename of the libraries have to be identical to
# the classname in the corresponding .cc/.hh files. That means
# one class per file.
# Do not specify these classes in 'LinkDef.hh'

SOLIBS   =

# 
# Starting from here no changes should be necessary
# 
CPPFLAGS := `$(BASEDIR)/bin/sorter-config -cpp`
CXXFLAGS := `$(BASEDIR)/bin/sorter-config -cxx` -fno-strict-aliasing
LDFLAGS  := `$(BASEDIR)/bin/sorter-config -ld -libs-wasa` -g


LDFLAGS += -lWasaRecFD -lWasaCalib -lWasaParameter  -lWasaRecFPC -lMLP  -lTreePlayer
LDFLAGS += -lRSClient

all: $(TARGET)

relink: $(TARGET).o $(TARGET).so $(addsuffix .so,$(SOLIBS))
	$(CXX) $(LDFLAGS) $^ -o $(TARGET)

$(TARGET): $(TARGET).o $(TARGET).so $(addsuffix .so,$(SOLIBS))
	$(CXX) $(LDFLAGS) $^ -o $@

$(TARGET).so: $(addsuffix .o,$(MODULES)) $(TARGET)Dict.o
	$(CXX) -fPIC -O2 -shared $^ -o $@

$(TARGET)Dict.cc: $(addsuffix .hh,$(MODULES)) LinkDef.hh
	rootcint -f $@ -c -p $(CPPFLAGS) $^

%.o: %.cc 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.so: %.cc
	rootcint -f $(subst .cc,Dict.cc,$<) -c -p $(CPPFLAGS) -g $(subst .cc,.hh,$<)+
	$(CXX) -fPIC -O2 -shared $(CPPFLAGS) $< $(subst .cc,Dict.cc,$<) -o $@

clean:
	-rm $(TARGET) $(TARGET).o
	-rm $(addsuffix .o,$(MODULES))
	-rm *Dict.*
	-rm *.so
	-rm *\~
