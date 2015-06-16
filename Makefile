CC = gcc
SRCS = main.c houghc.c canny.c sobel.c img_utils.c img.c vec.c 
OBJS = $(SRCS:.c=.o)

CFLAGS += $(shell pkg-config --cflags gtk+-2.0 glib-2.0)
CFLAGS += -Wall -g -O3

LDLIBS += $(shell pkg-config --libs gtk+-2.0 glib-2.0)
LDLIBS += -lm 

EXE = img-circle

.PHONY: clean all

all: $(EXE)

$(EXE): $(OBJS)
	@$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@ $(LDLIBS)
	@echo "Compilation is complited: $@"

%.o:%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Building $< --> $@"

-include $(subst .c,.d,$(SRCS))

%.d:%.c
	@$(CC) -M $(CPPFLAGS) $< > $@.$$$$ 2>/dev/null;		\
	sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	$(RM) $@.$$$$

clean:
	$(RM) *.o *.d *.c~ *.h~ $(EXE)


