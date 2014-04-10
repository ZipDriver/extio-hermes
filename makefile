SRC=.
TGT=.
INCLUDES = -I.
CXXFLAGS = -DNDEBUG -DFLOG -std=gnu++0x -DWINVER=0x502 -fpermissive $(INCLUDES)
LDFLAGS = -static -static-libgcc -static-libstdc++ -Wl,-Bstatic -L. -lpthreadGC2 -lwsock32  -liphlpapi  -s -shared -Wl,--add-stdcall-alias,--subsystem,windows
DLL = Extio_hpsdr_mgw.dll

SOURCES = $(wildcard $(SRC)/*.cpp)
OBJS = $(addprefix $(TGT)/, $(notdir $(SOURCES:.cpp=.o))) 
RESOURCES = gui.rc logw.rc
RESOURCES_OBJ = gui_rc.o logw_rc.o

$(TGT)/$(DLL): $(OBJS) $(RESOURCES_OBJ)
	$(CXX) $(OBJS) $(RESOURCES_OBJ) $(LDFLAGS) -o $@
	-cp Extio_hpsdr_mgw.dll "/c/Program Files (x86)/HDSDR"
	-cp Extio_hpsdr_mgw.dll "/c/Users/andrew/Studio_1/ExtIO/Hermes"
	-cp Extio_hpsdr_mgw.dll "/c/Users/andrew/Studio1_105e"
	-cp Extio_hpsdr_mgw.dll "/c/Users/andrew/HDSDR_270"
	
$(TGT)/gui_rc.o:	$(SRC)/gui.rc
	windres -i gui.rc -o gui_rc.o

$(TGT)/logw_rc.o:	$(SRC)/logw.rc
	windres -i logw.rc -o logw_rc.o

$(TGT)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

	
clean:
	-rm -f *.o $(TGT)/$(DLL)
	  
	 