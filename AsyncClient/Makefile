PROGNAME := TomyAsyncClient
SRCDIR := src
SUBDIR := src/lib

SRCS := $(SRCDIR)/Application.cpp \
$(SUBDIR)/GwProxy.cpp \
$(SUBDIR)/MqttsnClient.cpp \
$(SUBDIR)/NetworkUdp.cpp \
$(SUBDIR)/NetworkXBee.cpp \
$(SUBDIR)/Payload.cpp \
$(SUBDIR)/PublishManager.cpp \
$(SUBDIR)/RegisterManager.cpp \
$(SUBDIR)/SubscribeManager.cpp \
$(SUBDIR)/TaskManager.cpp \
$(SUBDIR)/test.cpp \
$(SUBDIR)/Timer.cpp \
$(SUBDIR)/TopicTable.cpp \
$(SUBDIR)/Util.cpp 

CXX := g++
CPPFLAGS += 
DEFS :=
LDFLAGS += 
LIBS +=

CXXFLAGS := -Wall -O3

OUTDIR := Build

PROG := $(OUTDIR)/$(PROGNAME)
OBJS := $(SRCS:%.cpp=$(OUTDIR)/%.o)
DEPS := $(SRCS:%.cpp=$(OUTDIR)/%.d)

.PHONY: install clean distclean

all: $(PROG)

-include $(DEPS)

$(PROG): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OUTDIR)/%.o:%.cpp
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFS) -o $@ -c -MMD -MP -MF $(@:%.o=%.d) $<

clean:
	rm -rf $(OUTDIR)

distclean:
	rm -rf Build
	
install:
	cp -pf $(PROG) ../../
	
