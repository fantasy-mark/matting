# Option for development
CFLAGS     := -g -Wall
CFLAGS     += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
CFLAGS     += -mcpu=cortex-a9 -mfloat-abi=softfp -mfpu=neon -mno-unaligned-access -fno-aggressive-loop-optimizations -O2

# Option for release
#CFLAGS= -O -Wall

CXXFLAGS = $(CFLAGS)

CC         = arm-hisiv400-linux-gcc
CXX        = arm-hisiv400-linux-g++
AR         = arm-hisiv400-linux-ar

PROGRAM   := director
SRCDIRS   := .
SRCEXTS   := .c .cpp

#QT_DIR   = /opt/qt4.8.6-Hi3531A-v400

INCLUDE = -I. -I../log \
        -I../mpp/include -I../mpp/extdrv/tlv320aic31 \
        -I$(QT_DIR)/include/QtCore \
        -I$(QT_DIR)/include/QtNetwork \
        -I$(QT_DIR)/include/QtGui \
        -I$(QT_DIR)/include

LIBS += -L../log -llog \
        -L../mpp/lib -lhdmi -live -lmd -lmpi -ltde -lupvqe -lVoiceEngine -ljpeg -ldnvqe -laacdec  -laacenc \
        -lpthread -ldl \
        -L$(QT_DIR)/lib -lQtGui -lQtNetwork -lQtCore

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
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

#----------------------------------------------
clean :
	$(RM) $(OBJS) $(PROGRAM)

rebuild: clean all
