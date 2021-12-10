/*
    texture.c
    Nate Robins, 1997

    Tool for teaching about OpenGL texture.
	
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "sgi.h"
#include "glm.h"
#include "ex.h"


typedef struct _cell {
    int id;
    int x, y;
    float min, max;
    float value;
    float step;
    char* info;
    char* format;
} cell;

void
printm(double m[], int size)
{
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			printf(" %f ", m[(j * size) + i]);
		}
		printf("\n");
	}
}

void
printmf(float m[], int size)
{
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			printf(" %f ", m[(j * size) + i]);
		}
		printf("\n");
	}
}

void
prnmatrix()
{
	double m[16] = {0};
	exgetparamd(GL_PROJECTION_MATRIX, m);
	printf("Projection\n");
	printm(m, 4);
	exgetparamd(GL_MODELVIEW_MATRIX, m);
	printf("Modelview\n");
	printm(m, 4);
}


cell texcoords[2*4] = {
    { 1, 140, 370, -2.0, 2.0, 0.0, 0.01,
      "Specifies S value of 1st texture coordinate", "%.1f" },
    { 2, 190, 370, -2.0, 2.0, 0.0, 0.01,
      "Specifies T value of 1st texture coordinate", "%.1f" },
    { 3, 140, 400, -2.0, 2.0, 1.0, 0.01,
      "Specifies S value of 2nd texture coordinate", "%.1f" },
    { 4, 190, 400, -2.0, 2.0, 0.0, 0.01,
      "Specifies T value of 2nd texture coordinate", "%.1f" },
    { 5, 140, 430, -2.0, 2.0, 1.0, 0.01,
      "Specifies S value of 3rd texture coordinate", "%.1f" },
    { 6, 190, 430, -2.0, 2.0, 1.0, 0.01,
      "Specifies T value of 3rd texture coordinate", "%.1f" },
    { 7, 140, 460, -2.0, 2.0, 0.0, 0.01,
      "Specifies S value of 4th texture coordinate", "%.1f" },
    { 8, 190, 460, -2.0, 2.0, 1.0, 0.01,
      "Specifies T value of 4th texture coordinate", "%.1f" },
};

cell vertices[3*4] = {
    { 9, 350, 370, -2.0, 2.0, -1.0, 0.01,
      "Specifies X coordinate of 1st vertex", "%.1f" },
    { 10, 400, 370, -2.0, 2.0, -1.0, 0.01,
      "Specifies Y coordinate of 1st vertex", "%.1f" },
    { 11, 450, 370, -2.0, 2.0, 0.0, 0.01,
      "Specifies Z coordinate of 1st vertex", "%.1f" },
    { 12, 350, 400, -2.0, 2.0, 1.0, 0.01,
      "Specifies X coordinate of 2nd vertex", "%.1f" },
    { 13, 400, 400, -2.0, 2.0, -1.0, 0.01,
      "Specifies Y coordinate of 2nd vertex", "%.1f" },
    { 14, 450, 400, -2.0, 2.0, 0.0, 0.01,
      "Specifies Z coordinate of 2nd vertex", "%.1f" },
    { 15, 350, 430, -2.0, 2.0, 1.0, 0.01,
      "Specifies X coordinate of 3rd vertex", "%.1f" },
    { 16, 400, 430, -2.0, 2.0, 1.0, 0.01,
      "Specifies Y coordinate of 3rd vertex", "%.1f" },
    { 17, 450, 430, -2.0, 2.0, 0.0, 0.01,
      "Specifies Z coordinate of 3rd vertex", "%.1f" },
    { 18, 350, 460, -2.0, 2.0, -1.0, 0.01,
      "Specifies X coordinate of 4th vertex", "%.1f" },
    { 19, 400, 460, -2.0, 2.0, 1.0, 0.01,
      "Specifies Y coordinate of 4th vertex", "%.1f" },
    { 20, 450, 460, -2.0, 2.0, 0.0, 0.01,
      "Specifies Z coordinate of 4th vertex", "%.1f" },
};

GLenum minfilter = GL_NEAREST;
GLenum magfilter = GL_NEAREST;
GLenum env = GL_MODULATE;

GLenum wraps = GL_REPEAT;
GLenum wrapt = GL_REPEAT;

cell translation[3] = {
    { 21, 120, 90, -5.0, 5.0, 0.0, 0.01,
      "Specifies X coordinate of translation vector.", "%.2f" },
    { 22, 180, 90, -5.0, 5.0, 0.0, 0.01,
      "Specifies Y coordinate of translation vector.", "%.2f" },
    { 23, 240, 90, -5.0, 5.0, 0.0, 0.01,
      "Specifies Z coordinate of translation vector.", "%.2f" },
};

cell rotation[4] = {
    { 24, 120, 120, -360.0, 360.0, 0.0, 1.0,
      "Specifies angle of rotation, in degrees.", "%.1f" },
    { 25, 180, 120, -1.0, 1.0, 0.0, 0.01,
      "Specifies X coordinate of vector to rotate about.", "%.2f" },
    { 26, 240, 120, -1.0, 1.0, 0.0, 0.01,
      "Specifies Y coordinate of vector to rotate about.", "%.2f" },
    { 27, 300, 120, -1.0, 1.0, 1.0, 0.01,
      "Specifies Z coordinate of vector to rotate about.", "%.2f" },
};

cell scale[3] = {
    { 28, 120, 150, -5.0, 5.0, 1.0, 0.01,
      "Specifies scale factor along X axis.", "%.2f" },
    { 29, 180, 150, -5.0, 5.0, 1.0, 0.01,
      "Specifies scale factor along Y axis.", "%.2f" },
    { 30, 240, 150, -5.0, 5.0, 1.0, 0.01,
      "Specifies scale factor along Z axis.", "%.2f" },
};

cell pcolor[4] = {
    { 31, 120, 310, 0.0, 1.0, 0.6, 0.01,
      "Specifies red component of polygon color.", "%.2f" },
    { 32, 180, 310, 0.0, 1.0, 0.6, 0.01,
      "Specifies green component of polygon color.", "%.2f" },
    { 33, 240, 310, 0.0, 1.0, 0.6, 0.01,
      "Specifies blue component of polygon color.", "%.2f" },
    { 34, 300, 310, 0.0, 1.0, 1.0, 0.01,
      "Specifies alpha component of polygon color.", "%.2f" },
};

cell bcolor[4] = {
    { 39, 240, 30, 0.0, 1.0, 1.0, 0.01,
      "Specifies red component of texture border color.", "%.2f" },
    { 40, 300, 30, 0.0, 1.0, 0.0, 0.01,
      "Specifies green component of texture border color.", "%.2f" },
    { 41, 360, 30, 0.0, 1.0, 0.0, 0.01,
      "Specifies blue component of texture border color.", "%.2f" },
    { 42, 420, 30, 0.0, 1.0, 1.0, 0.01,
      "Specifies alpha component of texture border color.", "%.2f" },
};

cell ecolor[4] = {
    { 35, 240, 60, 0.0, 1.0, 0.0, 0.01,
      "Specifies red component of texture environment color.", "%.2f" },
    { 36, 300, 60, 0.0, 1.0, 1.0, 0.01,
      "Specifies green component of texture environment color.", "%.2f" },
    { 37, 360, 60, 0.0, 1.0, 0.0, 0.01,
      "Specifies blue component of texture environment color.", "%.2f" },
    { 38, 420, 60, 0.0, 1.0, 1.0, 0.01,
      "Specifies alpha component of texture environment color.", "%.2f" },
};

GLfloat eye[3] = { 0.0, 0.0, 3.0 };
GLfloat at[3]  = { 0.0, 0.0, 0.0 };
GLfloat up[3]  = { 0.0, 1.0, 0.0 };


void redisplay_all(void);
GLuint window, world, screen, command;
GLuint sub_width = 256, sub_height = 256;
GLint selection = 0;

GLfloat spin_x = 0.0;
GLfloat spin_y = 0.0;

int iheight, iwidth, idepth;
unsigned char* image = NULL;
int twidth, theight;
int stipple;


#if defined(GL_VERSION_1_1)
#define GL_REPLACE_EXT GL_REPLACE
#endif


GLvoid *font_style = GLUT_BITMAP_TIMES_ROMAN_10;

void
setfont(char* name, int size)
{
    font_style = GLUT_BITMAP_HELVETICA_10;
    if (strcmp(name, "helvetica") == 0) {
	if (size == 12) 
	    font_style = GLUT_BITMAP_HELVETICA_12;
	else if (size == 18)
	    font_style = GLUT_BITMAP_HELVETICA_18;
    } else if (strcmp(name, "times roman") == 0) {
	font_style = GLUT_BITMAP_TIMES_ROMAN_10;
	if (size == 24)
	    font_style = GLUT_BITMAP_TIMES_ROMAN_24;
    } else if (strcmp(name, "8x13") == 0) {
	font_style = GLUT_BITMAP_8_BY_13;
    } else if (strcmp(name, "9x15") == 0) {
	font_style = GLUT_BITMAP_9_BY_15;
    }
}

void 
drawstr(GLuint x, GLuint y, char* format, ...)
{
    va_list args;
    char buffer[255], *s;
    
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    exrasterpos2i(x, y);
    for (s = buffer; *s; s++)
	exbitmapcharacter(font_style, *s);
}

void
cell_draw(cell* cell)
{
    excolor3ub(0, 255, 128);
    if (selection == cell->id) {
	excolor3ub(255, 255, 0);
	drawstr(10, 525, cell->info);
	excolor3ub(255, 0, 0);
    }

    drawstr(cell->x, cell->y, cell->format, cell->value);
}

int
cell_hit(cell* cell, int x, int y)
{
    if (x > cell->x && x < cell->x+50 &&
	y > cell->y-20 && y < cell->y+10)
	return cell->id;
    return 0;
}

void
cell_update(cell* cell, int update)
{
    if (selection != cell->id)
	return;

    cell->value += update * cell->step;

    if (cell->value < cell->min)
	cell->value = cell->min;
    else if (cell->value > cell->max) 
	cell->value = cell->max;

}

void
cell_vector(float* dst, cell* cell, int num)
{
    while (--num >= 0)
	dst[num] = cell[num].value;
}

void
drawaxes(void)
{
    excolor3ub(255, 0, 0);
    exbeginmode(GL_LINE_STRIP);
    exvertex3f(0.0, 0.0, 0.0);
    exvertex3f(1.0, 0.0, 0.0);
    exvertex3f(0.75, 0.25, 0.0);
    exvertex3f(0.75, -0.25, 0.0);
    exvertex3f(1.0, 0.0, 0.0);
    exvertex3f(0.75, 0.0, 0.25);
    exvertex3f(0.75, 0.0, -0.25);
    exvertex3f(1.0, 0.0, 0.0);
    exendmode();
    exbeginmode(GL_LINE_STRIP);
    exvertex3f(0.0, 0.0, 0.0);
    exvertex3f(0.0, 1.0, 0.0);
    exvertex3f(0.0, 0.75, 0.25);
    exvertex3f(0.0, 0.75, -0.25);
    exvertex3f(0.0, 1.0, 0.0);
    exvertex3f(0.25, 0.75, 0.0);
    exvertex3f(-0.25, 0.75, 0.0);
    exvertex3f(0.0, 1.0, 0.0);
    exendmode();
    exbeginmode(GL_LINE_STRIP);
    exvertex3f(0.0, 0.0, 0.0);
    exvertex3f(0.0, 0.0, 1.0);
    exvertex3f(0.25, 0.0, 0.75);
    exvertex3f(-0.25, 0.0, 0.75);
    exvertex3f(0.0, 0.0, 1.0);
    exvertex3f(0.0, 0.25, 0.75);
    exvertex3f(0.0, -0.25, 0.75);
    exvertex3f(0.0, 0.0, 1.0);
    exendmode();

    excolor3ub(255, 255, 0);
    exrasterpos3f(1.1, 0.0, 0.0);
    exbitmapcharacter(GLUT_BITMAP_HELVETICA_12, 'x');
    exrasterpos3f(0.0, 1.1, 0.0);
    exbitmapcharacter(GLUT_BITMAP_HELVETICA_12, 'y');
    exrasterpos3f(0.0, 0.0, 1.1);
    exbitmapcharacter(GLUT_BITMAP_HELVETICA_12, 'z');
}

void
texenv(void)
{
    GLfloat env_color[4], border_color[4];

    cell_vector(env_color, ecolor, 4);
    cell_vector(border_color, bcolor, 4);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapt);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, env);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, env_color);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
}

void
texture(void)
{
	exuseprogram(glutGetWindow());
    //texenv();
   	exbuildtex2d(GL_TEXTURE_2D, idepth == 4 ? GL_RGBA : GL_RGB, iwidth, iheight, 
		      idepth == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image, texenv);
}

void
main_reshape(int width,  int height) 
{
	exuseprogram(glutGetWindow());

    glViewport(0, 0, width, height);
    exsetmatrixmode(GL_PROJECTION, TRUE);
    
    exortho2d(0, width, height, 0);
    exsetmatrixmode(GL_MODELVIEW, TRUE);
    

#define GAP  25				/* gap between subwindows */
    sub_width = (width-GAP*3)/3;
    sub_height = (height-GAP*3)/2;

    glutSetWindow(screen);
    glutPositionWindow(GAP, GAP);
    glutReshapeWindow(sub_width, sub_height);
    glutSetWindow(world);
    glutPositionWindow(GAP, GAP+sub_height+GAP);
    glutReshapeWindow(sub_width, sub_height);
    glutSetWindow(command);
    glutPositionWindow(GAP+sub_width+GAP, GAP);
    glutReshapeWindow(sub_width*2, sub_height*2+GAP);
}

void
main_display(void)
{
	exuseprogram(glutGetWindow());

    glClearColor(0.8, 0.8, 0.8, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    excolor3ub(0, 0, 0);
    setfont("helvetica", 12);
    drawstr(GAP, GAP-5, "Screen-space view");
    drawstr(GAP+sub_width+GAP, GAP-5, "Command manipulation window");
    drawstr(GAP, GAP+sub_height+GAP-5, "Texture-space view");
    glutSwapBuffers();
}

void
world_reshape(int width, int height)
{
	exuseprogram(glutGetWindow());

    twidth = width;
    theight = height;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapt);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glViewport(0, 0, width, height);
    exsetmatrixmode(GL_PROJECTION, TRUE);
    exortho(0, width, 0, height, -1, 1);

    exsetmatrixmode(GL_MODELVIEW, TRUE);
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);
    exenable(GL_LIGHT0);
}

void
world_display(void)
{
	exuseprogram(glutGetWindow());

    exsetmatrixmode(GL_PROJECTION, TRUE);
    exortho(0, twidth, 0, theight, -1, 1);

    exsetmatrixmode(GL_MODELVIEW, TRUE);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    excolor3ub(255, 255, 255);

    exenable(GL_TEXTURE_2D);
    exenable(GL_LIGHTING);

    exbeginmode(GL_POLYGON);
    extexcoord2f(-0.5, -0.5);
    exvertex2i(0, 0);
    extexcoord2f(1.5, -0.5);
    exvertex2i(twidth, 0);
    extexcoord2f(1.5, 1.5);
    exvertex2i(twidth, theight);
    extexcoord2f(-0.5, 1.5);
    exvertex2i(0, theight);
    exendmode();

    exbeginmode(GL_POLYGON);
    extexcoord2f(-0.5, -0.5);
    exvertex2i(-1, -1);
    extexcoord2f(1.5, -0.5);
    exvertex2i(1, -1);
    extexcoord2f(1.5, 1.5);
    exvertex2i(1, 1);
    extexcoord2f(-0.5, 1.5);
    exvertex2i(-1, 1);
    exendmode();

    exdisable(GL_TEXTURE_2D);
    exdisable(GL_LIGHTING);

    exscalef(twidth/2, theight/2, 1.0);
    extranslatef(0.5, 0.5, 0.0);
    extranslatef(translation[0].value, translation[1].value,
		 translation[2].value);
    exrotatef(rotation[0].value, rotation[1].value, 
	      rotation[2].value, rotation[3].value);
    exscalef(scale[0].value, scale[1].value, scale[2].value);

    /* axes */
    expushmatrix();
    extranslatef(-0.001, -0.001, 0.0);
    excolor3ub(0, 255, 128);
    exbeginmode(GL_LINE_STRIP);
    exvertex2f(0.1, 0.5);
    exvertex2f(0.0, 0.6);
    exvertex2f(-0.1, 0.5);
    exendmode();
    exbeginmode(GL_LINE_STRIP);
    exvertex2f(0.5, 0.1);
    exvertex2f(0.6, 0.0);
    exvertex2f(0.5, -0.1);
    exendmode();
    exbeginmode(GL_LINE_STRIP);
    exvertex2f(0.6, 0.0);
    exvertex2f(0.0, 0.0);
    exvertex2f(0.0, 0.6);
    exendmode();
    excolor3ub(255, 255, 0);
    exrasterpos2f(0.6, -0.1);
    exbitmapcharacter(GLUT_BITMAP_HELVETICA_12, 's');
    exrasterpos2f(-0.1, 0.6);
    exbitmapcharacter(GLUT_BITMAP_HELVETICA_12, 't');
    expopmatrix();

    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, (GLushort)stipple);
    excolor3ub(255, 0, 0);
    exbeginmode(GL_LINE_LOOP);
    exvertex2f(texcoords[0].value, texcoords[1].value);
    exvertex2f(texcoords[2].value, texcoords[3].value);
    exvertex2f(texcoords[4].value, texcoords[5].value);
    exvertex2f(texcoords[6].value, texcoords[7].value);
    exendmode();
    glLineStipple(1, (GLushort)(stipple^0xffff));
    excolor3ub(255, 255, 255);
    exbeginmode(GL_LINE_LOOP);
    exvertex2f(texcoords[0].value, texcoords[1].value);
    exvertex2f(texcoords[2].value, texcoords[3].value);
    exvertex2f(texcoords[4].value, texcoords[5].value);
    exvertex2f(texcoords[6].value, texcoords[7].value);
    exendmode();
    glDisable(GL_LINE_STIPPLE);

    glutSwapBuffers();
}

void
world_menu(int value)
{
    char* name = 0;

    switch (value) {
    case 'f':
	name = "data/fishermen.sgi";
	break;
    case 'o':
	name = "data/opengl.sgi";
	break;
    case 'c':
	name = "data/checker.sgi";
	break;
    case 'm':
	name = "data/marble.sgi";
	break;
    case 't':
	name = "data/train.sgi";
	break;
    }

    if (name) {
	free(image);
	image = read_sgi(name, &iwidth, &iheight, &idepth);
	if (!image)
	    image = read_sgi("data/fishermen.sgi", &iwidth, &iheight, &idepth);
    }

    glutSetWindow(screen);
    texture();
    glutSetWindow(world);
    texture();

    redisplay_all();
}

void
screen_reshape(int width, int height)
{
	exuseprogram(glutGetWindow());

    glViewport(0, 0, width, height);
    exsetmatrixmode(GL_PROJECTION, TRUE);
    
    experspective(60.0, (GLfloat)width/height, 0.5, 8.0);
    exsetmatrixmode(GL_MODELVIEW, TRUE);
    
    exlookat(eye[0], eye[1], eye[2], at[0], at[1], at[2], up[0], up[1],up[2]);
    glClearColor(0.2, 0.2, 0.2, 1.0);
    exenable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    exenable(GL_LIGHTING);
    exenable(GL_LIGHT0);
    exenable(GL_TEXTURE_2D);

	exsetmatrixmode(GL_TEXTURE, TRUE);
    extranslatef(translation[0].value, translation[1].value,
		 translation[2].value);
    exrotatef(rotation[0].value, rotation[1].value, 
	      rotation[2].value, rotation[3].value);
    exscalef(scale[0].value, scale[1].value, scale[2].value);
    exsetmatrixmode(GL_MODELVIEW, FALSE);
}

void
screen_display(void)
{
	exuseprogram(glutGetWindow());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    expushmatrix();
    exrotatef(spin_y, 1.0, 0.0, 0.0);
    exrotatef(spin_x, 0.0, 1.0, 0.0);

    excolor4f(pcolor[0].value, pcolor[1].value, pcolor[2].value, pcolor[3].value);
    exbeginmode(GL_POLYGON);
    exnormal3f(0.0, 0.0, 1.0);
    extexcoord2f(texcoords[0].value, texcoords[1].value);
    exvertex3f(vertices[0].value, vertices[1].value, vertices[2].value);
    extexcoord2f(texcoords[2].value, texcoords[3].value);
    exvertex3f(vertices[3].value, vertices[4].value, vertices[5].value);
    extexcoord2f(texcoords[4].value, texcoords[5].value);
    exvertex3f(vertices[6].value, vertices[7].value, vertices[8].value);
    extexcoord2f(texcoords[6].value, texcoords[7].value);
    exvertex3f(vertices[9].value, vertices[10].value, vertices[11].value);
    exendmode();

    expopmatrix();
    glutSwapBuffers();
}

int old_x, old_y;

void
screen_mouse(int button, int state, int x, int y)
{
    old_x = x;
    old_y = y;
    
    redisplay_all();
}

void
screen_motion(int x, int y)
{
    spin_x = x - old_x;
    spin_y = y - old_y;
    
    redisplay_all();
}

void
command_reshape(int width, int height)
{
	exuseprogram(glutGetWindow());

    glViewport(0, 0, width, height);
    exsetmatrixmode(GL_PROJECTION, TRUE);
    
    exortho2d(0, width, height, 0);
    exsetmatrixmode(GL_MODELVIEW, TRUE);
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glDisable(GL_DEPTH_TEST);
}

void
polygon_display(void)
{
    setfont("helvetica", 12);

    drawstr(10, pcolor[0].y-50, "glEnable(GL_TEXTURE_2D);");
    drawstr(10, pcolor[0].y-30, "gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, "
	    "GL_RGB, GL_UNSIGNED_BYTE, image);");

    setfont("helvetica", 18);

    drawstr(10, pcolor[0].y, "excolor4f(");
    drawstr(pcolor[0].x+50, pcolor[0].y, ",");
    drawstr(pcolor[1].x+50, pcolor[1].y, ","); 
    drawstr(pcolor[2].x+50, pcolor[2].y, ","); 
    drawstr(pcolor[3].x+50, pcolor[2].y, ");"); 

    drawstr(10, texcoords[0].y-30, "exbeginmode(GL_POLYGON);");
    drawstr(10, texcoords[0].y, "extexcoord2f(");
    drawstr(texcoords[0].x+40, texcoords[0].y, ",");
    drawstr(texcoords[1].x+40, texcoords[1].y, ");"); 
    drawstr(250, vertices[0].y, "exvertex3f(");
    drawstr(vertices[0].x+40, vertices[0].y, ",");
    drawstr(vertices[1].x+40, vertices[1].y, ","); 
    drawstr(vertices[2].x+40, vertices[1].y, ");"); 

    drawstr(10, texcoords[2].y, "extexcoord2f(");
    drawstr(texcoords[2].x+40, texcoords[2].y, ",");
    drawstr(texcoords[3].x+40, texcoords[3].y, ");"); 
    drawstr(250, vertices[3].y, "exvertex3f(");
    drawstr(vertices[3].x+40, vertices[3].y, ",");
    drawstr(vertices[4].x+40, vertices[4].y, ","); 
    drawstr(vertices[5].x+40, vertices[5].y, ");"); 

    drawstr(10, texcoords[4].y, "extexcoord2f(");
    drawstr(texcoords[4].x+40, texcoords[4].y, ",");
    drawstr(texcoords[5].x+40, texcoords[5].y, ");"); 
    drawstr(250, vertices[6].y, "exvertex3f(");
    drawstr(vertices[6].x+40, vertices[6].y, ",");
    drawstr(vertices[7].x+40, vertices[7].y, ","); 
    drawstr(vertices[8].x+40, vertices[8].y, ");"); 

    drawstr(10, texcoords[6].y, "extexcoord2f(");
    drawstr(texcoords[6].x+40, texcoords[6].y, ",");
    drawstr(texcoords[7].x+40, texcoords[7].y, ");"); 
    drawstr(250, vertices[9].y, "exvertex3f(");
    drawstr(vertices[9].x+40, vertices[9].y, ",");
    drawstr(vertices[10].x+40, vertices[10].y, ","); 
    drawstr(vertices[11].x+40, vertices[11].y, ");"); 

    drawstr(10, vertices[11].y+30, "exendmode();");

    cell_draw(&texcoords[0]); 
    cell_draw(&texcoords[1]); 
    cell_draw(&texcoords[2]); 
    cell_draw(&texcoords[3]); 
    cell_draw(&texcoords[4]); 
    cell_draw(&texcoords[5]); 
    cell_draw(&texcoords[6]); 
    cell_draw(&texcoords[7]); 

    cell_draw(&vertices[0]);
    cell_draw(&vertices[1]);
    cell_draw(&vertices[2]);
    cell_draw(&vertices[3]);
    cell_draw(&vertices[4]);
    cell_draw(&vertices[5]);
    cell_draw(&vertices[6]);
    cell_draw(&vertices[7]);
    cell_draw(&vertices[8]);
    cell_draw(&vertices[9]);
    cell_draw(&vertices[10]);
    cell_draw(&vertices[11]);

    cell_draw(&pcolor[0]);
    cell_draw(&pcolor[1]);
    cell_draw(&pcolor[2]);
    cell_draw(&pcolor[3]);

    excolor3ub(255, 255, 255);
}

void
matrix_display(void)
{
	exuseprogram(glutGetWindow());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    excolor3ub(255, 255, 255);

    polygon_display();

    setfont("helvetica", 18);

    drawstr(10, translation[0].y-30, "glMatrixMode(GL_TEXTURE);");
    drawstr(10, translation[0].y, "extranslatef(");
    drawstr(170, translation[0].y, ",");
    drawstr(230, translation[0].y, ","); 
    drawstr(290, translation[0].y, ");");
    drawstr(30, rotation[0].y, "exrotatef(");
    drawstr(170, rotation[0].y, ",");
    drawstr(230, rotation[0].y, ","); 
    drawstr(290, rotation[0].y, ",");
    drawstr(350, rotation[0].y, ");");
    drawstr(38, scale[0].y, "exscalef(");
    drawstr(170, scale[0].y, ",");
    drawstr(230, scale[0].y, ","); 
    drawstr(290, scale[0].y, ");");
    drawstr(10, scale[0].y+30, "exsetmatrixmode(GL_MODELVIEW, TRUE);");

    cell_draw(&translation[0]);
    cell_draw(&translation[1]);
    cell_draw(&translation[2]);

    cell_draw(&rotation[0]);
    cell_draw(&rotation[1]);
    cell_draw(&rotation[2]);
    cell_draw(&rotation[3]);

    cell_draw(&scale[0]);
    cell_draw(&scale[1]);
    cell_draw(&scale[2]);

    if (!selection) {
	excolor3ub(255, 255, 0);
	drawstr(10, 525,
	     "Click on the arguments and move the mouse to modify values.");
    }   
    
    glutSwapBuffers();
}

void
parameters_display(void)
{
    float pos[4];

	exuseprogram(glutGetWindow());
	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    excolor3ub(255, 255, 255);

    polygon_display();

    setfont("helvetica", 18);
    drawstr(10, bcolor[0].y, "GLfloat border_color[ ] = { ");
    drawstr(bcolor[0].x+40, bcolor[0].y, ",");
    drawstr(bcolor[1].x+40, bcolor[0].y, ",");
    drawstr(bcolor[2].x+40, bcolor[0].y, ",");
    drawstr(bcolor[3].x+40, bcolor[0].y, "};");
    drawstr(10, ecolor[0].y, "GLfloat env_color[ ] = { ");
    drawstr(ecolor[0].x+40, ecolor[0].y, ",");
    drawstr(ecolor[1].x+40, ecolor[0].y, ",");
    drawstr(ecolor[2].x+40, ecolor[0].y, ",");
    drawstr(ecolor[3].x+40, ecolor[0].y, "};");

    setfont("helvetica", 12);
    drawstr(10, 90, "glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR,"
	    " border_color);");
    drawstr(10, 110, "glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, "
	    "env_color);");

    drawstr(10, 140, "glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,");
    drawstr(10, 160, "glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,");
    drawstr(10, 180, "glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,");
    drawstr(10, 200, "glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,");
    drawstr(10, 220, "glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,");

    excolor3ub(0, 255, 128);
    if (minfilter == GL_LINEAR_MIPMAP_LINEAR) {
	drawstr(380, 140, "GL_LINEAR_MIPMAP_LINEAR");
    } else if (minfilter == GL_LINEAR_MIPMAP_NEAREST) {
	drawstr(380, 140, "GL_LINEAR_MIPMAP_NEAREST");
    } else if (minfilter == GL_NEAREST_MIPMAP_LINEAR) {
	drawstr(380, 140, "GL_NEAREST_MIPMAP_LINEAR");
    } else if (minfilter == GL_NEAREST_MIPMAP_NEAREST) {
	drawstr(380, 140, "GL_NEAREST_MIPMAP_NEAREST");
    } else if (minfilter == GL_LINEAR) {
	drawstr(380, 140, "GL_LINEAR");
    } else {
	drawstr(380, 140, "GL_NEAREST");
    }
	
    excolor3ub(255, 255, 255);
    glGetFloatv(GL_CURRENT_RASTER_POSITION, pos); 
    drawstr(pos[0]+2, glutGet(GLUT_WINDOW_HEIGHT)-pos[1]-1, ");");
    excolor3ub(0, 255, 128);

    if (magfilter == GL_LINEAR) {
	drawstr(390, 160, "GL_LINEAR");
    } else {
	drawstr(390, 160, "GL_NEAREST");
    }

    excolor3ub(255, 255, 255);
    glGetFloatv(GL_CURRENT_RASTER_POSITION, pos); 
    drawstr(pos[0]+2, glutGet(GLUT_WINDOW_HEIGHT)-pos[1]-1, ");");
    excolor3ub(0, 255, 128);

    if (wraps == GL_REPEAT) {
	drawstr(360, 180, "GL_REPEAT");
    } else {
	drawstr(360, 180, "GL_CLAMP");
    }

    excolor3ub(255, 255, 255);
    glGetFloatv(GL_CURRENT_RASTER_POSITION, pos); 
    drawstr(pos[0]+2, glutGet(GLUT_WINDOW_HEIGHT)-pos[1]-1, ");");
    excolor3ub(0, 255, 128);

    if (wrapt == GL_REPEAT) {
	drawstr(360, 200, "GL_REPEAT");
    } else {
	drawstr(360, 200, "GL_CLAMP");
    }

    excolor3ub(255, 255, 255);
    glGetFloatv(GL_CURRENT_RASTER_POSITION, pos); 
    drawstr(pos[0]+2, glutGet(GLUT_WINDOW_HEIGHT)-pos[1]-1, ");");
    excolor3ub(0, 255, 128);

    if (env == GL_MODULATE) {
	drawstr(360, 220, "GL_MODULATE");
    } else if (env == GL_DECAL) {
	drawstr(360, 220, "GL_DECAL");
    } else if (env == GL_BLEND) {
	drawstr(360, 220, "GL_BLEND");
    } else {
	drawstr(360, 220, "GL_REPLACE");
    }

    excolor3ub(255, 255, 255);
    glGetFloatv(GL_CURRENT_RASTER_POSITION, pos); 
    drawstr(pos[0]+2, glutGet(GLUT_WINDOW_HEIGHT)-pos[1]-1, ");");

    setfont("helvetica", 18);

    cell_draw(&bcolor[0]);
    cell_draw(&bcolor[1]);
    cell_draw(&bcolor[2]);
    cell_draw(&bcolor[3]);

    cell_draw(&ecolor[0]);
    cell_draw(&ecolor[1]);
    cell_draw(&ecolor[2]);
    cell_draw(&ecolor[3]);
	
    if (!selection) {
	excolor3ub(255, 255, 0);
	drawstr(10, 525,
	     "Click on the arguments and move the mouse to modify values.");
    }   
    
    glutSwapBuffers();
}

void
polygon_mouse(int x, int y)
{
    /* mouse should only hit _one_ of the cells, so adding up all
       the hits just propagates a single hit. */
    selection += cell_hit(&texcoords[0], x, y);
    selection += cell_hit(&texcoords[1], x, y);
    selection += cell_hit(&texcoords[2], x, y);
    selection += cell_hit(&texcoords[3], x, y);
    selection += cell_hit(&texcoords[4], x, y);
    selection += cell_hit(&texcoords[5], x, y);
    selection += cell_hit(&texcoords[6], x, y);
    selection += cell_hit(&texcoords[7], x, y);
    selection += cell_hit(&vertices[0], x, y);
    selection += cell_hit(&vertices[1], x, y);
    selection += cell_hit(&vertices[2], x, y);
    selection += cell_hit(&vertices[3], x, y);
    selection += cell_hit(&vertices[4], x, y);
    selection += cell_hit(&vertices[5], x, y);
    selection += cell_hit(&vertices[6], x, y);
    selection += cell_hit(&vertices[7], x, y);
    selection += cell_hit(&vertices[8], x, y);
    selection += cell_hit(&vertices[9], x, y);
    selection += cell_hit(&vertices[10], x, y);
    selection += cell_hit(&vertices[11], x, y);
    selection += cell_hit(&pcolor[0], x, y);
    selection += cell_hit(&pcolor[1], x, y);
    selection += cell_hit(&pcolor[2], x, y);
    selection += cell_hit(&pcolor[3], x, y);
}

void
matrix_mouse(int button, int state, int x, int y)
{
    selection = 0;

    if (state == GLUT_DOWN) {
	polygon_mouse(x, y);
	/* mouse should only hit _one_ of the cells, so adding up all
           the hits just propagates a single hit. */
	selection += cell_hit(&translation[0], x, y);
	selection += cell_hit(&translation[1], x, y);
	selection += cell_hit(&translation[2], x, y);
	selection += cell_hit(&rotation[0], x, y);
	selection += cell_hit(&rotation[1], x, y);
	selection += cell_hit(&rotation[2], x, y);
	selection += cell_hit(&rotation[3], x, y);
	selection += cell_hit(&scale[0], x, y);
	selection += cell_hit(&scale[1], x, y);
	selection += cell_hit(&scale[2], x, y);
    }

    old_y = y;

    redisplay_all();
}

void
parameters_mouse(int button, int state, int x, int y)
{
    selection = 0;

    if (state == GLUT_DOWN) {
	polygon_mouse(x, y);
	selection += cell_hit(&bcolor[0], x, y);
	selection += cell_hit(&bcolor[1], x, y);
	selection += cell_hit(&bcolor[2], x, y);
	selection += cell_hit(&bcolor[3], x, y);
	selection += cell_hit(&ecolor[0], x, y);
	selection += cell_hit(&ecolor[1], x, y);
	selection += cell_hit(&ecolor[2], x, y);
	selection += cell_hit(&ecolor[3], x, y);
	if (!selection) {
	    if (y < 145 && y > 125) {
		if (minfilter == GL_NEAREST)
		    minfilter = GL_LINEAR;
		else if (minfilter == GL_LINEAR)
		    minfilter = GL_NEAREST_MIPMAP_NEAREST;
		else if (minfilter == GL_NEAREST_MIPMAP_NEAREST)
		    minfilter = GL_NEAREST_MIPMAP_LINEAR;
		else if (minfilter == GL_NEAREST_MIPMAP_LINEAR)
		    minfilter = GL_LINEAR_MIPMAP_NEAREST;
		else if (minfilter == GL_LINEAR_MIPMAP_NEAREST)
		    minfilter = GL_LINEAR_MIPMAP_LINEAR;
		else 
		    minfilter = GL_NEAREST;
	    } else if (y < 165 && y > 145) {
		if (magfilter == GL_NEAREST)
		    magfilter = GL_LINEAR;
		else
		    magfilter = GL_NEAREST;
	    } else if (y < 185 && y > 165) {
		if (wraps == GL_REPEAT)
		    wraps = GL_CLAMP;
		else
		    wraps = GL_REPEAT;
	    } else if (y < 205 && y > 185) {
		if (wrapt == GL_REPEAT)
		    wrapt = GL_CLAMP;
		else
		    wrapt = GL_REPEAT;
	    } else if (y < 225 && y > 205) {
		if (env == GL_REPLACE_EXT)
		    env = GL_DECAL;
		else if (env == GL_DECAL)
		    env = GL_BLEND;
		else if (env == GL_BLEND)
		    env = GL_MODULATE;
		else
		    env = GL_REPLACE_EXT;
	    }
	}
    }

    glutSetWindow(screen);
    texenv();
    glutSetWindow(world);
    texenv();

    old_y = y;

    redisplay_all();
}

void
command_motion(int x, int y)
{
    cell_update(&translation[0], old_y-y);
    cell_update(&translation[1], old_y-y);
    cell_update(&translation[2], old_y-y);
    cell_update(&rotation[0], old_y-y);
    cell_update(&rotation[1], old_y-y);
    cell_update(&rotation[2], old_y-y);
    cell_update(&rotation[3], old_y-y);
    cell_update(&scale[0], old_y-y);
    cell_update(&scale[1], old_y-y);
    cell_update(&scale[2], old_y-y);
    cell_update(&texcoords[0], old_y-y);
    cell_update(&texcoords[1], old_y-y);
    cell_update(&texcoords[2], old_y-y);
    cell_update(&texcoords[3], old_y-y);
    cell_update(&texcoords[4], old_y-y);
    cell_update(&texcoords[5], old_y-y);
    cell_update(&texcoords[6], old_y-y);
    cell_update(&texcoords[7], old_y-y);
    cell_update(&vertices[0], old_y-y);
    cell_update(&vertices[1], old_y-y);
    cell_update(&vertices[2], old_y-y);
    cell_update(&vertices[3], old_y-y);
    cell_update(&vertices[4], old_y-y);
    cell_update(&vertices[5], old_y-y);
    cell_update(&vertices[6], old_y-y);
    cell_update(&vertices[7], old_y-y);
    cell_update(&vertices[8], old_y-y);
    cell_update(&vertices[9], old_y-y);
    cell_update(&vertices[10], old_y-y);
    cell_update(&vertices[11], old_y-y);
    cell_update(&pcolor[0], old_y-y);
    cell_update(&pcolor[1], old_y-y);
    cell_update(&pcolor[2], old_y-y);
    cell_update(&pcolor[3], old_y-y);
    cell_update(&bcolor[0], old_y-y);
    cell_update(&bcolor[1], old_y-y);
    cell_update(&bcolor[2], old_y-y);
    cell_update(&bcolor[3], old_y-y);
    cell_update(&ecolor[0], old_y-y);
    cell_update(&ecolor[1], old_y-y);
    cell_update(&ecolor[2], old_y-y);
    cell_update(&ecolor[3], old_y-y);

    glutSetWindow(screen);
    texenv();
    glutSetWindow(world);
    texenv();

    old_y = y;

    redisplay_all();
}

void
timer(int value)
{
    stipple = 0x00ff << value;
    if (value > 8)
	stipple += 0x00ff >> (8-(value-8));

    value++;
    if (value >= 16)
	value = 0;

    glutSetWindow(world);
    glutPostRedisplay();

    glutTimerFunc(100, timer, value);
}


void
redisplay_all(void)
{
    glutSetWindow(command);
    glutPostRedisplay();
    glutSetWindow(world);
    world_reshape(sub_width, sub_height);
    glutPostRedisplay();
    glutSetWindow(screen);
    screen_reshape(sub_width, sub_height);
    glutPostRedisplay();
}

void
main_keyboard(char key, int x, int y)
{
    switch (key) {
    case 'm':
	glutSetWindow(command);
	glutMouseFunc(matrix_mouse);
	glutDisplayFunc(matrix_display);
	break;
    case 'p':
	glutSetWindow(command);
	glutMouseFunc(parameters_mouse);
	glutDisplayFunc(parameters_display);
	break;
    case 'r':
	translation[0].value = 0.0;
	translation[1].value = 0.0;
	translation[2].value = 0.0;
	rotation[0].value = 0.0;
	rotation[1].value = 0.0;
	rotation[2].value = 0.0;
	rotation[3].value = 1.0;
	scale[0].value = 1.0;
	scale[1].value = 1.0;
	scale[2].value = 1.0;
	texcoords[0].value = 0.0;
	texcoords[1].value = 0.0;
	texcoords[2].value = 1.0;
	texcoords[3].value = 0.0;
	texcoords[4].value = 1.0;
	texcoords[5].value = 1.0;
	texcoords[6].value = 0.0;
	texcoords[7].value = 1.0;
	vertices[0].value = -1.0;
	vertices[1].value = -1.0;
	vertices[2].value = 0.0;
	vertices[3].value = 1.0;
	vertices[4].value = -1.0;
	vertices[5].value = 0.0;
	vertices[6].value = 1.0;
	vertices[7].value = 1.0;
	vertices[8].value = 0.0;
	vertices[9].value = -1.0;
	vertices[10].value = 1.0;
	vertices[11].value = 0.0;
	pcolor[0].value = 0.6;
	pcolor[1].value = 0.6;
	pcolor[2].value = 0.6;
	pcolor[3].value = 0.6;
	bcolor[0].value = 1.0;
	bcolor[1].value = 0.0;
	bcolor[2].value = 0.0;
	bcolor[3].value = 1.0;
	ecolor[0].value = 0.0;
	ecolor[1].value = 1.0;
	ecolor[2].value = 0.0;
	ecolor[3].value = 1.0;
	minfilter = GL_NEAREST;
	magfilter = GL_NEAREST;
	env = GL_MODULATE;
	wraps = GL_REPEAT;
	wrapt = GL_REPEAT;
	break;
    case 27:
	exit(0);
    }

    glutSetWindow(screen);
    texenv();
    redisplay_all();
}

void
command_menu(int value)
{
    main_keyboard((unsigned char)value, 0, 0);
}

int
main(int argc, char** argv)
{
    image = read_sgi("data/fishermen.sgi", &iwidth, &iheight, &idepth);
    if (!image)
	exit(0);

	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize((512+GAP*3)*3/2, 512+GAP*3);
    glutInitWindowPosition(50, 50);
    glutInit(&argc, argv);

    window = glutCreateWindow("Texture");
	exinitialize(window);

    glutReshapeFunc(main_reshape);
    glutDisplayFunc(main_display);
    glutKeyboardFunc(main_keyboard);

    world = glutCreateSubWindow(window, GAP, GAP, 256, 256);
	exinitialize(world);

    glutReshapeFunc(world_reshape);
    glutDisplayFunc(world_display);
    glutKeyboardFunc(main_keyboard);
    glutCreateMenu(world_menu);
    glutAddMenuEntry("Textures", 0);
    glutAddMenuEntry("", 0);
    glutAddMenuEntry("Fishermen", 'f');
    glutAddMenuEntry("OpenGL Logo", 'o');
    glutAddMenuEntry("Checker", 'c');
    glutAddMenuEntry("Marble", 'm');
    glutAddMenuEntry("Train", 't');
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    texture();

    screen = glutCreateSubWindow(window, GAP+256+GAP, GAP, 256, 256);
	exinitialize(screen);

    glutReshapeFunc(screen_reshape);
    glutDisplayFunc(screen_display);
    glutKeyboardFunc(main_keyboard);
    glutMotionFunc(screen_motion);
    glutMouseFunc(screen_mouse);

    texture();

    command = glutCreateSubWindow(window, GAP+256+GAP, GAP+256+GAP, 256, 256);
	exinitialize(command);

    glutReshapeFunc(command_reshape);
    glutMotionFunc(command_motion);
    glutDisplayFunc(parameters_display);
    glutMouseFunc(parameters_mouse);
    glutKeyboardFunc(main_keyboard);
    glutCreateMenu(command_menu);
    glutAddMenuEntry("Texture", 0);
    glutAddMenuEntry("", 0);
    glutAddMenuEntry("Matrix", 'm');
    glutAddMenuEntry("Environment/Parameters", 'p');
    glutAddMenuEntry("Reset parameters (r)", 'r');
    glutAddMenuEntry("", 0);
    glutAddMenuEntry("Quit", 27);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    redisplay_all();

    glutTimerFunc(500, timer, 0);
    glutMainLoop();
    return 0;
}
