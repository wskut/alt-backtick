CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -IInclude
LDFLAGS := -static-libgcc -static-libstdc++
LDLIBS := -lkernel32 -luser32 -lgdi32 -lshell32 -ladvapi32 -lcomctl32 -lole32 -lpsapi

SRCDIR := Source
OBJDIR := Obj
BINDIR := bin

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
RES_OBJ := $(OBJDIR)/Resource.o

TARGET := $(BINDIR)/alt-backtick.exe

.PHONY: all clean release debug

all: release

release: CXXFLAGS += -O2 -DNDEBUG -mwindows
release: $(TARGET)

debug: CXXFLAGS += -O0 -g -DDEBUG
debug: $(TARGET)

$(TARGET): $(OBJECTS) $(RES_OBJ) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(RES_OBJ): resources/Resource.rc Include/Resource.h resources/icon.ico alt-backtick.exe.manifest | $(OBJDIR)
	windres resources/Resource.rc -I resources -I Include -I . -o $@

$(OBJDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)
