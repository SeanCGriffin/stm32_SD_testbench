TARGET_EXEC ?= test_streamer

BUILD_DIR ?= ./build
SRC_DIRS ?= .../Core/Src

INCDIR = ../Core/Inc

SRCS := main.cpp\
		ff_proxy.cpp\
		printer.c\
		streamer.cpp\
		packet.cpp\
		hs_time.cpp

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := inc\
			$(INCDIR)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CC	 = gcc
CXX	 = g++

FLAGS ?= -DPLATFORM_STANDALONE 
# CPPFLAGS ?= $(INC_FLAGS) -DPLATFORM_STANDALONE
CXXFLAGS ?= $(INC_FLAGS) $(FLAGS) 
CFLAGS ?= $(INC_FLAGS) $(FLAGS) -lstdc++

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@echo $(CXX) $(OBJS) -o $@ $(FLAGS)
	$(CXX) $(OBJS) -o $@ $(FLAGS)

# # assembly
# $(BUILD_DIR)/%.s.o: %.s
# 	$(MKDIR_P) $(dir $@)
# 	@echo $(AS) $(ASFLAGS) -c $< -o $@
# 	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	@echo $(CC) $(CFLAGS) -c $< -o $@ 
	$(CC) $(CFLAGS) -c $< -o $@ 

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	@echo $(CXX) $(CXXFLAGS) -c $< -o $@
	$(CXX) $(CXXFLAGS) -c $< -o $@ 


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

# run the program
.PHONY: run
run: $(BUILD_DIR)/$(TARGET_EXEC)
	./$(BUILD_DIR)/$(TARGET_EXEC) $(PWD)
	@echo "\n\n"
	# @echo "File contents:"
	# @cat hitspool/*/*.spool
	@echo "File sizes:"
	@ls -lh hitspool/*/*.spool


# SRCDIR = ../Core/Src
# OBJDIR = obj/
# INCDIR = ../Core/Inc
# INCLUDES = -I $(INCDIR) -I inc

# OBJS	= $(OBJDIR)/main.o\
# 		  $(OBJDIR)/ff_proxy.o

# SOURCE	= main.cpp\
# 		  $(SRCDIR)/streamer.cpp
# #HEADER	= $(INCDIR)/streamer.h
# OUT	= test_streamer
# CC	 = g++
# FLAGS	 = -g -c -Wall $(INCLUDES) -DPLATFORM_STANDALONE
# LFLAGS	 = 
# # -g option enables debugging mode 
# # -c flag generates object code for separate files


# all: $(OBJS)
# 	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

# test_streamer: $(OBJS)
# 	$(CC) -g $(OBJS) -o $(OUT)

# # create/compile the individual files >>separately<<
# $(OBJDIR)/main.o: main.cpp
# 	@echo $(FLAGS)
# 	$(CC) $(FLAGS) main.cpp -o $@

# # $(OBJDIR)/hs_streamer.o: $(SRCDIR)/hs_streamer.c
# # 	@echo $(FLAGS)
# # 	$(CC) $(FLAGS) $(SRCDIR)/hs_streamer.c -o $@

# $(OBJDIR)/ff_proxy.o: ff_proxy.c
# 	@echo $(FLAGS)
# 	$(CC) $(FLAGS) ff_proxy.c -o $@

# # $(OBJDIR)/hs_readback.o: $(SRCDIR)/hs_readback.c
# # 	@echo $(FLAGS)
# # 	$(CC) $(FLAGS) $(SRCDIR)/hs_readback.c -o $@	

# # clean house
# clean:
# 	rm -f $(OBJS) $(OUT)

# # run the program
# run: $(OUT)
# 	./$(OUT)