# 默认参数
CC		=	gcc
SRCDIR	=	src
OBJDIR  =	obj
INCDIR	=	include
eLIBDIR	=	external/lib
eINCDIR	=	external/include
SOURCE	=	$(wildcard $(SRCDIR)/*.c)
OBJECT	=	$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCE))
HANDLE	=	$(wildcard $(INCDIR)/*.h)
CFLAGS  =  -I$(INCDIR) -I$(eINCDIR) -L$(eLIBDIR) -g
CFLAGS  += -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lcomdlg32 -lGmSSL -lws2_32
TARGET	=	encFace

# 检查CFLAGS是否包含-g参数
ifneq (,$(findstring -g,$(CFLAGS)))
CFLAGS += -D_DEBUG
else
CFLAGS += -mwindows
endif

$(TARGET):$(OBJECT)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJDIR)/%.o:$(SRCDIR)/%.c $(HANDLE) | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: clean
clean:
	if [ -d "$(OBJDIR)" ]; then rm $(OBJDIR)/*.o; fi
	if [ -f "$(TARGET)" ]; then rm $(TARGET); fi