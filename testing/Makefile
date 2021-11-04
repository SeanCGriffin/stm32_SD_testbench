#
# In order to execute this "Makefile" just type "make"
#	A. Delis (ad@di.uoa.gr)
#

SRCDIR = ../Core/Src
OBJDIR = obj/
INCDIR = ../Core/Inc
INCLUDES = -I $(INCDIR) -I inc

OBJS	= $(OBJDIR)/main.o\
		  $(OBJDIR)/hs_streamer.o\
		  $(OBJDIR)/ff_proxy.o\
		  $(OBJDIR)/hs_readback.o\
		  $(OBJDIR)/hs_unit_tests.o
SOURCE	= main.c\
		  $(SRCDIR)/streamer.c
#HEADER	= $(INCDIR)/streamer.h
OUT	= test_streamer
CC	 = g++
FLAGS	 = -g -c -Wall $(INCLUDES) -DPLATFORM_STANDALONE
LFLAGS	 = 
# -g option enables debugging mode 
# -c flag generates object code for separate files


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

$(OBJS) : | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

test_streamer: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

# create/compile the individual files >>separately<<
$(OBJDIR)/main.o: main.c
	@echo $(FLAGS)
	$(CC) $(FLAGS) main.c -o $@

$(OBJDIR)/hs_streamer.o: $(SRCDIR)/hs_streamer.c
	@echo $(FLAGS)
	$(CC) $(FLAGS) $(SRCDIR)/hs_streamer.c -o $@

$(OBJDIR)/ff_proxy.o: ff_proxy.c
	@echo $(FLAGS)
	$(CC) $(FLAGS) ff_proxy.c -o $@

$(OBJDIR)/hs_readback.o: $(SRCDIR)/hs_readback.c
	@echo $(FLAGS)
	$(CC) $(FLAGS) $(SRCDIR)/hs_readback.c -o $@	

$(OBJDIR)/hs_unit_tests.o: $(SRCDIR)/hs_unit_tests.c
	@echo $(FLAGS)
	$(CC) $(FLAGS) $(SRCDIR)/hs_unit_tests.c -o $@	


# clean house
clean:
	rm -f $(OBJS) $(OUT)

# run the program
run: $(OUT)
	./$(OUT)