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
CFLAGS  =  -I$(INCDIR) -I$(eINCDIR) -L$(eLIBDIR)
CFLAGS  += -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lgmssl -lm
CFLAGS  += -D_DEBUG
# 检查操作系统类型
ifeq ($(OS),Windows_NT) # Windows环境
	RM = del
	MKDIR = mkdir
	TARGET_EXT = .exe

    CFLAGS += -lmingw32 -lWs2_32
else # Linux环境
	RM = rm -f
	MKDIR = mkdir -p
	TARGET_EXT =
	
	CFLAGS  += -D_LINUX
	CFLAGS  += `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`
endif
TARGET  = encFace$(TARGET_EXT)

$(TARGET): $(OBJECT)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HANDLE) | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

.PHONY: clean
clean:
	if [ -d "$(OBJDIR)" ]; then $(RM) $(OBJDIR)/*.o; fi
	if [ -f "$(TARGET)" ]; then $(RM) $(TARGET); fi