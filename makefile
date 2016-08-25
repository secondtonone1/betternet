CXXFLAGS = -I./netmodel  
TARGET = networklibtest
DIRS = ./netmodel ./Linux
OBJDIR = ./Linux
FILES = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS = $(patsubst %.cpp, %.o ,$(FILES))

$(TARGET):$(OBJS) 
	$(CXX) -g -o  $(OBJDIR)/$(TARGET) $^ $(CXXFLAGS) 

clean:
	-$(RM) $(TARGET)
	-$(RM) $(OBJS)
