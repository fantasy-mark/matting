CFLAGS     := -g -Wall
CFLAGS     += -mcpu=cortex-a9 -mfloat-abi=softfp -mfpu=neon -mno-unaligned-access -fno-aggressive-loop-optimizations

CC         = arm-hisiv400-linux-gcc
AR         = arm-hisiv400-linux-ar

PROGRAM   := liblog.a
SRCDIRS   := .
SRCEXTS   := .c

INCLUDE    +=-I.
	
## Stable Section: usually no need to be changed. But you can add more.
##=============================================================================
SHELL   = /bin/sh
SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
OBJS    = $(foreach x,$(SRCEXTS), \
      $(patsubst %$(x),%.o,$(filter %$(x),$(SOURCES))))
DEPS    = $(patsubst %.o,%.d,$(OBJS))

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $<
	
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) $(OUTPUT_OPTION) $<

.PHONY : all clean rebuild

all : $(PROGRAM)

##=============================================================================
# Rules for producing the executable.
#----------------------------------------------
$(PROGRAM) : $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

#----------------------------------------------
clean :
	$(RM) $(OBJS) $(PROGRAM)
	
rebuild: clean all

