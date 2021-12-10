
CC = gcc
TARGETS = transformation projection lightposition texture lightmaterial fog
ifeq ($(OS),Windows_NT)
ifeq ($(CC), clang)
CLANG = -target x86_64-w64-mingw64 #-D _USE_MATH_DEFINES
endif
CFLAGS =  $(CLANG) -g -I ../../include # path to glut and glew includes
LDFLAGS = $(CLANG) -l:libfreeglut.a -lopengl32 -lglu32 -L ../../Lib/cc/x64 -l:glew32.dll
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),FreeBSD)
CC = clang
CFLAGS = -g -I /usr/local/include
LDFLAGS = -L /usr/local/lib -lGL -lGLU -lglut -lGLEW -lm
else ifeq ($(UNAME_S),Darwin)
CFLAGS = -g  -D GL_SILENCE_DEPRECATION
LDFLAGS = -framework GLUT -framework OpenGL -framework Cocoa
else
CFLAGS = -g
LDFLAGS = -lGL -lGLU -lglut -lGLEW -lm
endif
endif

ODIR = bin

src = $(wildcard *.c)
# _OBJS	= $(patsubst src/%.c, %.o, $(SRC))
objs = $(addprefix $(ODIR)/, $(src:.c=.o))
rmexe = $(addprefix $(ODIR)/, $(TARGETS))

all		: $(TARGETS)

$(TARGETS)	: $(objs)
	$(CC) -o $(ODIR)/$@ $(ODIR)/$@.o $(ODIR)/glm.o $(ODIR)/sgi.o $(ODIR)/vsl.o $(ODIR)/glmex.o $(ODIR)/exfont.o $(LDFLAGS)

$(ODIR)/%.o: %.c ex.h | $(ODIR)
	$(CC) -c $(CFLAGS) $< -o $@

 $(ODIR):
	mkdir $(ODIR)

clean:
	rm $(rmexe) $(ODIR)/*.o $(ODIR)/*.exe