PROGNAME := TomyGateway
SRCDIR := src
SUBDIR := src/lib

SRCS := $(SRCDIR)/TomyGateway.cpp \
$(SRCDIR)/BrokerRecvTask.cpp \
$(SRCDIR)/BrokerSendTask.cpp \
$(SRCDIR)/ClientRecvTask.cpp \
$(SRCDIR)/ClientSendTask.cpp \
$(SRCDIR)/GatewayControlTask.cpp \
$(SRCDIR)/GatewayResourcesProvider.cpp \
$(SUBDIR)/ProcessFramework.cpp \
$(SUBDIR)/Messages.cpp \
$(SUBDIR)/TCPStack.cpp \
$(SUBDIR)/TLSStack.cpp \
$(SUBDIR)/Topics.cpp \
$(SUBDIR)/UDPStack.cpp \
$(SUBDIR)/XXXXXStack.cpp \
$(SUBDIR)/Test.cpp \
$(SUBDIR)/ZBStack.cpp

CXX := g++
CPPFLAGS += 
DEFS :=
LDFLAGS += 
LIBS +=
LDADD := -lpthread -lrt -lssl -lcrypto

CXXFLAGS := -Wall -O3
RPI := no
LDPI := -lwiringPi
OUTDIR := Build

PROG := $(OUTDIR)/$(PROGNAME)
OBJS := $(SRCS:%.cpp=$(OUTDIR)/%.o)
DEPS := $(SRCS:%.cpp=$(OUTDIR)/%.d)

.PHONY: install clean distclean start stop

all: $(PROG)                                                                                                                                                                                                                                                                                                                                                                                                                               

-include $(DEPS)

$(PROG): $(OBJS)
ifeq ($(RPI),no)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS) $(LDADD)
else
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS) $(LDADD) $(LDPI)
endif

$(OUTDIR)/%.o:%.cpp
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFS) -o $@ -c -MMD -MP -MF $(@:%.o=%.d) $<

clean:
	rm -rf $(OUTDIR)

install:
	cp -pf $(PROG) ../../
	
start:
	sudo systemctl start tomygateway.service
	
stop:
	sudo systemctl stop tomygateway.service
	
	
