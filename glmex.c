

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#if __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/Opengl.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif
#include "ex.h"

#define VSRC	"vertex.vert"
#define FSRC	"frag.frag"

#define MAXLIGHTS 2
#define MAXTEX 2
#define MAXPARAMS 13
// parameters u_params
#define LIGHT0		0
#define LIGHT1		1

#define LIGHTING	0
#define LOCALVIEWER	1
#define TWOSIDED	2
#define MATAMBIENT	3
#define MATDIFFUSE	4
#define MATSPECULAR	5
#define MATEMISSIVE	6
#define MATTRACKCOL	7
#define MATFRONT	8
#define MATBACK		9
#define TXTRENDER	10
#define FOG	11
#define TEXTURE	12

#define FOG_LINEAR 0
#define FOG_EXP 1
#define FOG_EXP2 2

enum { BUFINIT = 100,
    BUFGROW = 2,
    MAXSTACK = 12,
    FRONT = 0,
    BACK = 1,
    MAXCONTEXTS = 12
};
typedef struct Array Array;
struct Array {
	int max, nval, dim;
	float *buffer;
};

typedef struct Stack Stack;

struct Stack {
	float *stack[MAXSTACK];
	int nval, mode, dim;
};


typedef struct Ulights  Ulights;
struct Ulights {
	int ambient, diffuse, specular;
	int kaklkq;
	int position;
	int spdirc;
	int spexp, spangle;
	float famb[4], fdif[4], fspec[4], fk[3], fpos[4], fsdirc[3], fsexp, fsang;
};

typedef struct Umaterial Umaterial;
struct Umaterial {
	int ambient, diffuse, specular, emissive, specexp;
	float famb[4], fdif[4], fspec[4], femis[4], fexp;
};
typedef struct UFog UFog;
struct UFog {
	int mode, start, end, density, color;
	float fstart, fend, fdensity, fcolor[4];
	int imode;
};

typedef struct Glcontext Glcontext;
struct Glcontext {
	Stack stprojmat, stviewmat, sttexmat, stnormat;
	Stack *curstack;
	float *curmatrix;

	Array arrcolor, arrvertex, arrnormal, arrtex;
	int a_position, a_normal, a_miter, a_texture, a_color, u_texunit1, u_texttex,
	    glpgm11;
	int u_modelview, u_projection, u_normalmatrix, u_viewport, u_sceneambient,
	    u_texturematrix;
	int u_shader, u_lightposition, u_eyeposition, u_lightson, u_params, u_textloc;
	GLenum drawmode;
	Ulights u_lights[MAXLIGHTS];
	Umaterial u_materials[2];
	UFog u_fog;
	int bitlightson[MAXLIGHTS], bitparms[MAXPARAMS];
	float curcolor[4], sceneambient[4], rasterpos[3], rasteroff[3];
	int BEGIN;
	float wres, hres;
	unsigned int textures[MAXTEX], ntex, texture;
};

Glcontext *glcontext[MAXCONTEXTS], *curcxt = NULL;
extern Anglecode *Glmfont;
void readanglecode(char *src, Glcontext *cxt);

void
checkerr()
{
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR) {
		printf("error: %d", err);
	}
}

void
bufferinitf(Array *array, int dim)
{
	array->dim = dim;
	array->nval = 0;
	array->buffer = (float *) malloc(sizeof(float) * BUFINIT * dim);
	array->max = BUFINIT;
}

void
bufferclearf(Array *array)
{
	array->nval = 0;
}

void
bufferadd4f(Array *array, float x, float y, float z, float a)
{
	int i = array->nval * array->dim;

	if (array->nval == array->max) {
		float *na = (float *) realloc(array->buffer, sizeof(float) *
		        array->dim * array->max * BUFGROW);

		array->buffer = na;
		array->max = array->max * BUFGROW;
	}
	array->buffer[i++] = x;
	array->buffer[i++] = y;
	array->buffer[i++] = z;
	array->buffer[i++] = a;
	array->nval++;
}

void
bufferadd3f(Array *array, float x, float y, float z)
{
	int i = array->nval * array->dim;

	if (array->nval == array->max) {
		float *na = (float *) realloc(array->buffer,
		        sizeof(float) * array->dim * array->max * BUFGROW);

		array->buffer = na;
		array->max = array->max * BUFGROW;
	}
	array->buffer[i++] = x;
	array->buffer[i++] = y;
	array->buffer[i++] = z;

	array->nval++;
}

void
bufferadd2f(Array *array, float x, float y)
{
	int i = array->nval * array->dim;

	if (array->nval == array->max) {
		float *na = (float *) realloc(array->buffer,
		        sizeof(float) * array->dim * array->max * BUFGROW);

		array->buffer = na;
		array->max = array->max * BUFGROW;
	}
	array->buffer[i++] = x;
	array->buffer[i++] = y;

	array->nval++;
}

float *
bufferloadf(Array *array)
{
	return array->buffer;
}

void
shadererr(int shader, int islink)
{
	GLint status;
	int typ = islink == 0 ? GL_COMPILE_STATUS : GL_LINK_STATUS;

	islink == 0 ? glGetShaderiv(shader, typ, &status) :
	glGetProgramiv(shader, typ, &status);
	if (status == GL_FALSE) {
		GLint length;
		char *s;
		islink == 0 ? glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length) :
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);
		s = (char *) malloc(length);
		islink == 0 ? glGetShaderInfoLog(shader, length, &length, s) :
		glGetProgramInfoLog(shader, length, &length, s);

		printf("error: %s", s);
		exit(1);
	}
}

void
initlights(int pgm, Glcontext *cxt)
{
	Ulights *ul;
	char s[32];
	float d0[4] = {1, 1, 1, 1}, d1[4] = {0, 0, 0, 1}, pos[4] = {0, 0, 1, 0},
	            sdir[3] = {0, 0, -1}, at[3] = {1, 0, 0},
	                    mamb[4] = {0.2, 0.2, 0.2, 1}, mdif[4] = {0.8, 0.8, 0.8, 1};

	cxt->u_sceneambient = glGetUniformLocation(pgm, "u_sceneambient");
	memcpy(cxt->sceneambient, mamb, sizeof(float) * 4);

	for (int i = 0; i < MAXLIGHTS; i++) {
		ul = &cxt->u_lights[i];
		sprintf(s, "u_lights[%d].ambient", i);
		ul->ambient = glGetUniformLocation(pgm, s);
		sprintf(s, "u_lights[%d].diffuse", i);
		ul->diffuse = glGetUniformLocation(pgm, s);
		sprintf(s, "u_lights[%d].specular", i);
		ul->specular = glGetUniformLocation(pgm, s);
		sprintf(s, "u_lights[%d].kaklkq", i);
		ul->kaklkq = glGetUniformLocation(pgm, s);
		sprintf(s, "u_lights[%d].position", i);
		ul->position = glGetUniformLocation(pgm, s);
		sprintf(s, "u_lights[%d].spdirc", i);
		ul->spdirc = glGetUniformLocation(pgm, s);
		sprintf(s, "u_lights[%d].spangle", i);
		ul->spangle = glGetUniformLocation(pgm, s);
		sprintf(s, "u_lights[%d].spexp", i);
		ul->spexp = glGetUniformLocation(pgm, s);

		if (i == 0) {
			memcpy(ul->fdif, d0, sizeof(float) * 4);
			memcpy(ul->fspec, d0, sizeof(float) * 4);
		} else {
			memcpy(ul->fdif, d1, sizeof(float) * 4);
			memcpy(ul->fspec, d1, sizeof(float) * 4);
		}

		memcpy(ul->famb, d1, sizeof(float) * 4);
		memcpy(ul->fpos, pos, sizeof(float) * 4);
		memcpy(ul->fsdirc, sdir, sizeof(float) * 3);
		memcpy(ul->fk, at, sizeof(float) * 3);
		ul->fsexp = 0;
		ul->fsang = 180;
	}

	cxt->u_materials[0].ambient = glGetUniformLocation(pgm,
	        "u_materials[0].ambient");
	cxt->u_materials[0].diffuse = glGetUniformLocation(pgm,
	        "u_materials[0].diffuse");
	cxt->u_materials[0].specular = glGetUniformLocation(pgm,
	        "u_materials[0].specular");
	cxt->u_materials[0].emissive = glGetUniformLocation(pgm,
	        "u_materials[0].emissive");
	cxt->u_materials[0].specexp = glGetUniformLocation(pgm,
	        "u_materials[0].specexp");
	memcpy(&cxt->u_materials[0].famb, mamb, sizeof(float) * 4);
	memcpy(&cxt->u_materials[0].fdif, mdif, sizeof(float) * 4);
	memcpy(&cxt->u_materials[0].fspec, d1, sizeof(float) * 4);
	memcpy(&cxt->u_materials[0].femis, d1, sizeof(float) * 4);
	cxt->u_materials[0].fexp = 0;

	cxt->u_materials[1].ambient = glGetUniformLocation(pgm,
	        "u_materials[1].ambient");
	cxt->u_materials[1].diffuse = glGetUniformLocation(pgm,
	        "u_materials[1].diffuse");
	cxt->u_materials[1].specular = glGetUniformLocation(pgm,
	        "u_materials[1].specular");
	cxt->u_materials[1].emissive = glGetUniformLocation(pgm,
	        "u_materials[1].emissive");
	cxt->u_materials[1].specexp = glGetUniformLocation(pgm,
	        "u_materials[1].specexp");
	memcpy(&cxt->u_materials[1].famb, mamb, sizeof(float) * 4);
	memcpy(&cxt->u_materials[1].fdif, mdif, sizeof(float) * 4);
	memcpy(&cxt->u_materials[1].fspec, d1, sizeof(float) * 4);
	memcpy(&cxt->u_materials[1].femis, d1, sizeof(float) * 4);
	cxt->u_materials[1].fexp = 0;
}

void
setmaterials(Umaterial *mat, GLenum pname, const float *params)
{
	switch(pname) {
	case GL_AMBIENT_AND_DIFFUSE:
		memcpy(&mat->famb, params, sizeof(float) * 4);
		memcpy(&mat->fdif, params, sizeof(float) * 4);
		break;
	case GL_AMBIENT:
		memcpy(&mat->famb, params, sizeof(float) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(&mat->fdif, params, sizeof(float) * 4);
		break;
	case GL_SPECULAR:
		memcpy(&mat->fspec, params, sizeof(float) * 4);
		break;
	case GL_EMISSION:
		memcpy(&mat->femis, params, sizeof(float) * 4);
		break;
	case GL_SHININESS:
		memcpy(&mat->fexp, params, sizeof(float) * 1);
		break;
	default:
		assert(FALSE);
	}

}

void
initfog(int pgm, Glcontext *cxt)
{
	UFog *fog = &cxt->u_fog;
	cxt->u_fog.mode = glGetUniformLocation(pgm, "u_fog.mode");
	cxt->u_fog.start = glGetUniformLocation(pgm, "u_fog.start");
	cxt->u_fog.end = glGetUniformLocation(pgm, "u_fog.end");
	cxt->u_fog.density = glGetUniformLocation(pgm, "u_fog.density");
	cxt->u_fog.color = glGetUniformLocation(pgm, "u_fog.color");
	memset(cxt->u_fog.fcolor, 0, sizeof(float) * 4);
	cxt->u_fog.fstart = 0;
	cxt->u_fog.fend = 1;
	cxt->u_fog.fdensity = 1;
	cxt->u_fog.imode = FOG_EXP;
}

char *
filetostr(char *filename)
{
	struct stat st;
	int size;
	char *str;
	FILE *fp;

	stat(filename, &st);
	size = st.st_size;
	str = malloc(size + 1);

	fp = fopen(filename, "r");
	if (fp == NULL) {
		assert(0);
	}
	size = fread(str, 1, size, fp);
	fclose(fp);
	str[size] = 0;

	return str;
}

void
initstacks(Glcontext *cxt)
{
	float *m;

	m = malloc(sizeof(float) * 16);
	vslsetidentitymatrix(m, 4);
	cxt->stprojmat.stack[0] = m;
	cxt->stprojmat.dim = 4;
	cxt->stprojmat.mode = GL_PROJECTION_MATRIX;
	cxt->stprojmat.nval = 0;

	m = malloc(sizeof(float) * 16);
	vslsetidentitymatrix(m, 4);
	cxt->stviewmat.stack[0] = m;
	cxt->stviewmat.dim = 4;
	cxt->stviewmat.mode = GL_MODELVIEW_MATRIX;
	cxt->stviewmat.nval = 0;

	m = malloc(sizeof(float) * 16);
	vslsetidentitymatrix(m, 4);
	cxt->sttexmat.stack[0] = m;
	cxt->sttexmat.dim = 4;
	cxt->sttexmat.mode = GL_TEXTURE_MATRIX;
	cxt->sttexmat.nval = 0;

	m = malloc(sizeof(float) * 9);
	vslsetidentitymatrix(m, 3);
	cxt->stnormat.stack[0] = m;
	cxt->curstack = &cxt->stviewmat;

}

void
exinitialize(int i)
{
	Glcontext *cxt;
	int glpgm11;
	float c[4] = {1, 1, 1, 1};
	int vshad = -1, fshad = -1;
	const char *vsrc, *fsrc;
	int vlen = -1, flen = -1;
#if !__APPLE__
	if (glewInit()) {
		assert(0);
	}
#endif
	assert(i < MAXCONTEXTS);
	cxt = malloc(sizeof(Glcontext));
	glcontext[i] = cxt;

	vsrc = filetostr(VSRC);
	fsrc = filetostr(FSRC);
	vlen = strlen(vsrc);
	flen = strlen(fsrc);
	vshad = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshad, 1, &vsrc, &vlen);
	glCompileShader(vshad);
	shadererr(vshad, 0);
	fshad = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshad, 1, &fsrc, &flen);
	glCompileShader(fshad);
	shadererr(fshad, 0);
	glpgm11 = glCreateProgram();
	glAttachShader(glpgm11, vshad);
	glAttachShader(glpgm11, fshad);
	glLinkProgram(glpgm11);
	shadererr(glpgm11, 1);
	cxt->glpgm11 = glpgm11;
	glUseProgram(glpgm11);
	cxt->a_position = glGetAttribLocation(glpgm11, "a_position");
	cxt->a_normal = glGetAttribLocation(glpgm11, "a_normal");
	cxt->a_texture = glGetAttribLocation(glpgm11, "a_texture");
	checkerr();
	cxt->a_color = glGetAttribLocation(glpgm11, "a_color");
	checkerr();
	cxt->u_texunit1 =  glGetUniformLocation(glpgm11, "u_texunit1");
	cxt->u_texttex =  glGetUniformLocation(glpgm11, "u_texttex");

	cxt->u_params = glGetUniformLocation(glpgm11, "u_params");
	cxt->u_textloc = glGetUniformLocation(glpgm11, "u_textloc");
	cxt->u_lightson = glGetUniformLocation(glpgm11, "u_lightson");
	cxt->u_modelview = glGetUniformLocation(glpgm11, "u_modelview");
	cxt->u_projection = glGetUniformLocation(glpgm11, "u_projection");
	cxt->u_normalmatrix = glGetUniformLocation(glpgm11, "u_normalmatrix");
	cxt->u_texturematrix = glGetUniformLocation(glpgm11, "u_texturematrix");
	cxt->u_viewport = glGetUniformLocation(glpgm11, "u_viewport");
	initlights(glpgm11, cxt);
	bufferinitf(&cxt->arrcolor, 4);
	bufferinitf(&cxt->arrvertex, 3);
	bufferinitf(&cxt->arrnormal, 3);
	bufferinitf(&cxt->arrtex, 2);
	initstacks(cxt);

	memcpy(&cxt->curcolor, c, sizeof(float) * 4);
	cxt->BEGIN = FALSE;
	memset(cxt->bitlightson, 0, sizeof(int) * MAXLIGHTS);
	memset(cxt->bitparms, 0, sizeof(int) * MAXPARAMS);
	cxt->bitlightson[LIGHT0] = 1;
	cxt->bitparms[MATAMBIENT] = 1;
	cxt->bitparms[MATDIFFUSE] = 1;
	cxt->bitparms[MATFRONT] = 1;
	cxt->bitparms[MATBACK] = 1;
	cxt->ntex = 0;
	initfog(glpgm11, cxt);
	readanglecode(NULL, cxt);
	exuseprogram(glutGetWindow());
}

void
exuseprogram(int n)
{
	curcxt = glcontext[n];
	glUseProgram(curcxt->glpgm11);
}

void
setcolor(float x, float y, float z, float w)
{

	if (curcxt->BEGIN) {
		int co = curcxt->arrvertex.nval - curcxt->arrcolor.nval;
		assert(curcxt->arrvertex.nval >= curcxt->arrcolor.nval);
		for(int i = 0; i < co; i++) {
			bufferadd4f(&curcxt->arrcolor, curcxt->curcolor[0], curcxt->curcolor[1],
			    curcxt->curcolor[2], curcxt->curcolor[3]);
		}
	}

	curcxt->curcolor[0] = x;
	curcxt->curcolor[1] = y;
	curcxt->curcolor[2] = z;
	curcxt->curcolor[3] = w;

}
void
excolor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
	setcolor(red / 255., green / 255., blue / 255., 1);
}

void
excolor3f(float x, float y, float z)
{
	setcolor(x, y, z, 1);
}

void
excolor3fv(const GLfloat *p)
{
	setcolor(p[0], p[1], p[2], 1);
}

void
excolor4f(float x, float y, float z, float w)
{
	setcolor(x, y, z, w);
}

void
addcolorv()
{
	assert(curcxt->arrvertex.nval >= curcxt->arrcolor.nval);
	if (curcxt->arrvertex.nval == curcxt->arrcolor.nval)
		return;
	int i = (curcxt->arrcolor.nval - 1) * 4;
	excolor3f(curcxt->arrcolor.buffer[i], curcxt->arrcolor.buffer[i + 1],
	    curcxt->arrcolor.buffer[i + 2]);
}

void
exvertex3fv(const GLfloat *p)
{
	bufferadd3f(&curcxt->arrvertex,  p[0], p[1], p[2]);
}

void
exvertex2f(float x, float y)
{
	bufferadd3f(&curcxt->arrvertex,  x, y, 0);
}
void
exvertex2i(int x, int y)
{
	exvertex2f(x, y);
}

void
exnormal3fv(const GLfloat *p)
{
	bufferadd3f(&curcxt->arrnormal, p[0], p[1], p[2]);
}

void
extexcoord2fv(const GLfloat *p)
{
	bufferadd2f(&curcxt->arrtex, p[0], p[1]);
}

void
exvertex3f(float x, float y, float z)
{
	bufferadd3f(&curcxt->arrvertex, x, y, z);
}

void
exvertex3i(int x, int y, int z)
{
	bufferadd3f(&curcxt->arrvertex, x, y, z);
}

void
exnormal3f(float x, float y, float z)
{
	bufferadd3f(&curcxt->arrnormal, x, y, z);
}

void
extexcoord2f(float x, float y)
{
	bufferadd2f(&curcxt->arrtex, x, y);
}

void
exbeginmode(GLenum mode)
{
	curcxt->drawmode = mode;
	bufferclearf(&curcxt->arrvertex);
	bufferclearf(&curcxt->arrnormal);
	bufferclearf(&curcxt->arrtex);
	curcxt->BEGIN = TRUE;
	bufferclearf(&curcxt->arrcolor);
}

void
exmaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
	if (face == GL_FRONT_AND_BACK || face == GL_FRONT) {
		setmaterials(&curcxt->u_materials[FRONT], pname, params);
	}
	if (face == GL_FRONT_AND_BACK || face == GL_BACK) {
		setmaterials(&curcxt->u_materials[BACK], pname, params);
	}
}

void
exmaterialf(GLenum face, GLenum pname, const GLfloat params)
{
	if (pname == GL_SHININESS) {
		return exmaterialfv(face, pname, &params);
	}
	assert(FALSE);
}

void
setuplights(Glcontext *cxt)
{
	Ulights *ul;

	glUniform4fv(cxt->u_sceneambient, 1, cxt->sceneambient);
	checkerr();
	for (int i = 0; i < MAXLIGHTS; i++) {
		if (curcxt->bitlightson[i]) {
			ul = &curcxt->u_lights[i];
			glUniform4fv(ul->ambient, 1, ul->famb);
			glUniform4fv(ul->diffuse, 1, ul->fdif);
			glUniform4fv(ul->specular, 1, ul->fspec);
			glUniform4fv(ul->position, 1, ul->fpos);
			glUniform3fv(ul->spdirc, 1, ul->fsdirc);
			glUniform3fv(ul->kaklkq, 1, ul->fk);
			glUniform1fv(ul->spexp, 1, &ul->fsexp);
			glUniform1fv(ul->spangle, 1, &ul->fsang);
		}
	}
	checkerr();
	glUniform4fv(cxt->u_materials[0].ambient, 1, cxt->u_materials[0].famb);
	glUniform4fv(cxt->u_materials[0].diffuse, 1, cxt->u_materials[0].fdif);
	glUniform4fv(cxt->u_materials[0].specular, 1, cxt->u_materials[0].fspec);
	glUniform4fv(cxt->u_materials[0].emissive, 1, cxt->u_materials[0].femis);
	glUniform1fv(cxt->u_materials[0].specexp, 1, &cxt->u_materials[0].fexp);
	checkerr();
	glUniform4fv(cxt->u_materials[BACK].ambient, 1, cxt->u_materials[BACK].famb);
	glUniform4fv(cxt->u_materials[BACK].diffuse, 1, cxt->u_materials[BACK].fdif);
	glUniform4fv(cxt->u_materials[BACK].specular, 1, cxt->u_materials[BACK].fspec);
	glUniform4fv(cxt->u_materials[BACK].emissive, 1, cxt->u_materials[BACK].femis);
	glUniform1fv(cxt->u_materials[BACK].specexp, 1, &cxt->u_materials[BACK].fexp);
	checkerr();
}

void
exendmode()
{
	float *vbo, *cbo, *nbo, *tbo;
	float nm[16];
	float normalmatrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};

	checkerr();
	//glUseProgram(glpgm11);
	checkerr();
	vbo = bufferloadf(&curcxt->arrvertex);
	setcolor(curcxt->curcolor[0], curcxt->curcolor[1], curcxt->curcolor[2],
	    curcxt->curcolor[3]);
	cbo = bufferloadf(&curcxt->arrcolor);
	nbo = bufferloadf(&curcxt->arrnormal);
	tbo = bufferloadf(&curcxt->arrtex);
	if (curcxt->bitparms[LIGHTING]) {
		setuplights(curcxt);
	}
	checkerr();
	glUniform1iv(curcxt->u_params, MAXPARAMS, curcxt->bitparms);
	checkerr();
	glUniform1iv(curcxt->u_lightson, MAXLIGHTS, curcxt->bitlightson);
	checkerr();
	if (curcxt->bitparms[TEXTURE]) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, curcxt->textures[0]);
		glUniform1i(curcxt->u_texunit1, 1);
	}
	if (curcxt->bitparms[FOG]) {
		glUniform1i(curcxt->u_fog.mode, curcxt->u_fog.imode);
		glUniform1f(curcxt->u_fog.start, curcxt->u_fog.fstart);
		glUniform1f(curcxt->u_fog.end, curcxt->u_fog.fend);
		glUniform1f(curcxt->u_fog.density, curcxt->u_fog.fdensity);
		glUniform4fv(curcxt->u_fog.color, 1, curcxt->u_fog.fcolor);
		checkerr();
	}
	glUniformMatrix4fv(curcxt->u_projection, 1, FALSE,
	    curcxt->stprojmat.stack[curcxt->stprojmat.nval]);
	checkerr();
	glUniformMatrix4fv(curcxt->u_modelview, 1, FALSE,
	    curcxt->stviewmat.stack[curcxt->stviewmat.nval]);
	checkerr();
	glUniformMatrix4fv(curcxt->u_texturematrix, 1, FALSE,
	    curcxt->sttexmat.stack[curcxt->sttexmat.nval]);
	checkerr();

	if (vslinvert(curcxt->stviewmat.stack[curcxt->stviewmat.nval], nm) == 0) {
		extranspose4x3f(nm, normalmatrix);
	} else {
		//ex4x4to3x3(curcxt->stviewmat.stack[curcxt->stviewmat.nval], normalmatrix);
		vslsetidentitymatrix(normalmatrix, 3);
	}
	glUniformMatrix3fv(curcxt->u_normalmatrix, 1, FALSE, normalmatrix);
	checkerr();
	glEnableVertexAttribArray(curcxt->a_position);
	checkerr();

	glVertexAttribPointer(curcxt->a_position, 3, GL_FLOAT, GL_FALSE, 0, vbo);
	checkerr();
	if (cbo  != NULL && curcxt->arrcolor.nval > 0) {
		glEnableVertexAttribArray(curcxt->a_color);
		glVertexAttribPointer(curcxt->a_color, 4, GL_FLOAT, GL_FALSE, 0, cbo);
	}
	checkerr();
	if (nbo != NULL && curcxt->arrnormal.nval > 0) {
		glEnableVertexAttribArray(curcxt->a_normal);
		glVertexAttribPointer(curcxt->a_normal, 3, GL_FLOAT, GL_FALSE, 0, nbo);
	}
	checkerr();
	if (tbo != NULL && curcxt->arrtex.nval > 0) {
		glEnableVertexAttribArray(curcxt->a_texture);
		glVertexAttribPointer(curcxt->a_texture, 2, GL_FLOAT, GL_FALSE, 0, tbo);
	}
	checkerr();

	glDrawArrays(curcxt->drawmode, 0, curcxt->arrvertex.nval);
	checkerr();
	glFinish();
	//glUseProgram(0);
	if (curcxt->bitparms[TEXTURE]) {
		;//glDisable(GL_BLEND);
		//glEnable(GL_DEPTH_TEST);
	}
	bufferclearf(&curcxt->arrcolor);
	curcxt->BEGIN = FALSE;
}



void
exrotatef(GLfloat angle, GLfloat x,  GLfloat y,  GLfloat z)
{
	vslrotate(curcxt->curstack->stack[curcxt->curstack->nval], angle, x, y, z);
}

void
extranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	vsltranslate(curcxt->curstack->stack[curcxt->curstack->nval], x, y, z);
}

void
exscalef(GLfloat x, GLfloat y, GLfloat z)
{
	vslscale(curcxt->curstack->stack[curcxt->curstack->nval], x, y, z);
}

void
exsetmatrixmode(int mode, int reset)
{
	switch (mode) {
	case GL_PROJECTION:
		curcxt->curstack = &curcxt->stprojmat;
		break;
	case GL_MODELVIEW:
		curcxt->curstack = &curcxt->stviewmat;
		break;
	case GL_TEXTURE:
		curcxt->curstack = &curcxt->sttexmat;
		break;
	default:
		assert(0);
	}
	if (reset) {
		vslsetidentitymatrix(curcxt->curstack->stack[curcxt->curstack->nval],
		    curcxt->curstack->dim);
	}
}

void
expushmatrix()
{
	int bytes = sizeof(float) * curcxt->curstack->dim * curcxt->curstack->dim;
	float *matrix = (float *) malloc(bytes);
	memcpy(matrix, curcxt->curstack->stack[curcxt->curstack->nval], bytes);
	curcxt->curstack->stack[++curcxt->curstack->nval] = matrix;
}

void
expopmatrix()
{
	float *m = curcxt->curstack->stack[curcxt->curstack->nval--];
	free(m);
}

void
exmultmatrixd(const GLdouble *m)
{
	float mf[16];
	for (int i = 0; i < 16; i++)
		mf[i] = (float) m[i];
	vslmultmatrix(curcxt->curstack->stack[curcxt->curstack->nval], mf);
}

void
exdisable(GLenum mode)
{
	switch (mode) {
	case GL_LIGHT0:
		curcxt->bitlightson[LIGHT0] = 0;
		break;
	case GL_LIGHTING:
		curcxt->bitparms[LIGHTING] = 0;
		break;
	case GL_COLOR_MATERIAL:
		curcxt->bitparms[MATTRACKCOL] = 0;
		break;
	case GL_FOG:
		curcxt->bitparms[FOG] = 0;
		break;
	case GL_TEXTURE_2D:
		curcxt->bitparms[TEXTURE] = 0;
		break;
	default:
		assert(FALSE);
	}
}

void
exenable(GLenum mode)
{
	switch (mode) {
	case GL_LIGHT0:
		curcxt->bitlightson[LIGHT0] = 1;
		break;
	case GL_LIGHTING:
		curcxt->bitparms[LIGHTING] = 1;
		break;
	case GL_COLOR_MATERIAL:
		curcxt->bitparms[MATTRACKCOL] = 1;
		break;
	case GL_FOG:
		curcxt->bitparms[FOG] = 1;
		break;
	case GL_TEXTURE_2D:
		curcxt->bitparms[TEXTURE] = 1;
		break;
	default:
		assert(FALSE);
	}
}

void
exfogfv(GLenum pname, GLfloat *params)
{
	int mode;
	switch (pname) {
	case GL_FOG_MODE:
		mode = *params;
		curcxt->u_fog.imode = mode == GL_EXP ? FOG_EXP : (mode == GL_EXP2 ? FOG_EXP2 :
		        FOG_LINEAR);
		break;
	case GL_FOG_START:
		curcxt->u_fog.fstart = *params;
		break;
	case GL_FOG_END:
		curcxt->u_fog.fend = *params;
		break;
	case GL_FOG_DENSITY:
		curcxt->u_fog.fdensity = *params;
		break;
	case GL_FOG_COLOR:
		memcpy(curcxt->u_fog.fcolor, params, sizeof(float) * 4);
		break;
	default:
		assert(FALSE);
	}
}

void
exfogi(GLenum pname, int params)
{
	float f = params;
	exfogfv(pname, &f);
}

void
exfogf(GLenum pname, GLfloat params)
{
	exfogfv(pname, &params);
}

void
matftod(double *m, float *mf, int size)
{
	int mz = size * size;
	for (int i = 0; i < mz; i++) {
		m[i] = mf[i];
	}
}

void
exgetparamd(GLenum mode, double *params)
{
	switch (mode) {
	case GL_PROJECTION_MATRIX:
		matftod(params, curcxt->stprojmat.stack[curcxt->stprojmat.nval], 4);
		break;
	case GL_MODELVIEW_MATRIX:
		matftod(params, curcxt->stviewmat.stack[curcxt->stviewmat.nval], 4);
		break;
	case GL_TEXTURE_MATRIX:
		matftod(params, curcxt->sttexmat.stack[curcxt->sttexmat.nval], 4);
		break;
	default:
		assert(FALSE);
	}
}

void
experspective(float fov, float ratio, float nearp, float farp)
{
	vslperspective(curcxt->curstack->stack[curcxt->curstack->nval], fov, ratio,
	    nearp, farp);
}

void
exlookat(float xpos, float ypos, float zpos, float xlook,
    float ylook, float zlook, float xup, float yup, float zup)
{
	vsllookat(curcxt->curstack->stack[curcxt->curstack->nval], xpos, ypos, zpos,
	    xlook, ylook,
	    zlook, xup, yup, zup);
}

void
exortho2d(float left, float right, float bottom, float top)
{
	vslortho2d(curcxt->curstack->stack[curcxt->curstack->nval], left, right, bottom,
	    top);
}

void
exortho(float left, float right, float bottom, float top,
    float nearp, float farp)
{
	vslortho(curcxt->curstack->stack[curcxt->curstack->nval], left, right, bottom,
	    top, nearp, farp);
}

void
exfrustum(float left, float right, float bottom, float top,
    float nearp, float farp)
{
	vslfrustum(curcxt->curstack->stack[curcxt->curstack->nval], left, right, bottom,
	    top, nearp, farp);
}

void
identityd(GLdouble m[16])
{
	m[0 + 4 * 0] = 1;
	m[0 + 4 * 1] = 0;
	m[0 + 4 * 2] = 0;
	m[0 + 4 * 3] = 0;
	m[1 + 4 * 0] = 0;
	m[1 + 4 * 1] = 1;
	m[1 + 4 * 2] = 0;
	m[1 + 4 * 3] = 0;
	m[2 + 4 * 0] = 0;
	m[2 + 4 * 1] = 0;
	m[2 + 4 * 2] = 1;
	m[2 + 4 * 3] = 0;
	m[3 + 4 * 0] = 0;
	m[3 + 4 * 1] = 0;
	m[3 + 4 * 2] = 0;
	m[3 + 4 * 3] = 1;
}

void
identityf(GLfloat m[16])
{
	m[0 + 4 * 0] = 1;
	m[0 + 4 * 1] = 0;
	m[0 + 4 * 2] = 0;
	m[0 + 4 * 3] = 0;
	m[1 + 4 * 0] = 0;
	m[1 + 4 * 1] = 1;
	m[1 + 4 * 2] = 0;
	m[1 + 4 * 3] = 0;
	m[2 + 4 * 0] = 0;
	m[2 + 4 * 1] = 0;
	m[2 + 4 * 2] = 1;
	m[2 + 4 * 3] = 0;
	m[3 + 4 * 0] = 0;
	m[3 + 4 * 1] = 0;
	m[3 + 4 * 2] = 0;
	m[3 + 4 * 3] = 1;
}

GLboolean
exinvertf(GLfloat src[16], GLfloat inverse[16])
{
	double t;
	int i, j, k, swap;
	GLfloat tmp[4][4];

	identityf(inverse);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			tmp[i][j] = src[i * 4 + j];
		}
	}

	for (i = 0; i < 4; i++) {
		/* look for largest element in column. */
		swap = i;
		for (j = i + 1; j < 4; j++) {
			if (fabs(tmp[j][i]) > fabs(tmp[i][i])) {
				swap = j;
			}
		}

		if (swap != i) {
			/* swap rows. */
			for (k = 0; k < 4; k++) {
				t = tmp[i][k];
				tmp[i][k] = tmp[swap][k];
				tmp[swap][k] = t;

				t = inverse[i * 4 + k];
				inverse[i * 4 + k] = inverse[swap * 4 + k];
				inverse[swap * 4 + k] = t;
			}
		}

		if (tmp[i][i] == 0) {
			/* no non-zero pivot.  the matrix is singular, which
			shouldn't happen.  This means the user gave us a bad
			matrix. */
			return GL_FALSE;
		}

		t = tmp[i][i];
		for (k = 0; k < 4; k++) {
			tmp[i][k] /= t;
			inverse[i * 4 + k] /= t;
		}
		for (j = 0; j < 4; j++) {
			if (j != i) {
				t = tmp[j][i];
				for (k = 0; k < 4; k++) {
					tmp[j][k] -= tmp[i][k] * t;
					inverse[j * 4 + k] -= inverse[i * 4 + k] * t;
				}
			}
		}
	}
	return GL_TRUE;
}

GLboolean
exinvertd(GLdouble src[16], GLdouble inverse[16])
{
	double t;
	int i, j, k, swap;
	GLdouble tmp[4][4];

	identityd(inverse);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			tmp[i][j] = src[i * 4 + j];
		}
	}

	for (i = 0; i < 4; i++) {
		/* look for largest element in column. */
		swap = i;
		for (j = i + 1; j < 4; j++) {
			if (fabs(tmp[j][i]) > fabs(tmp[i][i])) {
				swap = j;
			}
		}

		if (swap != i) {
			/* swap rows. */
			for (k = 0; k < 4; k++) {
				t = tmp[i][k];
				tmp[i][k] = tmp[swap][k];
				tmp[swap][k] = t;

				t = inverse[i * 4 + k];
				inverse[i * 4 + k] = inverse[swap * 4 + k];
				inverse[swap * 4 + k] = t;
			}
		}

		if (tmp[i][i] == 0) {
			/* no non-zero pivot.  the matrix is singular, which
			shouldn't happen.  This means the user gave us a bad
			matrix. */
			return GL_FALSE;
		}

		t = tmp[i][i];
		for (k = 0; k < 4; k++) {
			tmp[i][k] /= t;
			inverse[i * 4 + k] /= t;
		}
		for (j = 0; j < 4; j++) {
			if (j != i) {
				t = tmp[j][i];
				for (k = 0; k < 4; k++) {
					tmp[j][k] -= tmp[i][k] * t;
					inverse[j * 4 + k] -= inverse[i * 4 + k] * t;
				}
			}
		}
	}
	return GL_TRUE;
}

void
extranspose4x4d(GLdouble src[16])
{
	GLdouble tmp[16];
	int i, j;

	memcpy(tmp, src, sizeof(GLdouble) * 16);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			src[i + j * 4] = tmp[i * 4 + j];
		}
	}
}

void
extranspose4x3d(GLdouble src[16], GLdouble dst[9])
{
	int i, j;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			dst[i + j * 3] = src[i * 4 + j];
		}
	}
}

void
extranspose4x4f(GLfloat src[16])
{
	GLfloat tmp[16];
	int i, j;

	memcpy(tmp, src, sizeof(GLfloat) * 16);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			src[i + j * 4] = tmp[i * 4 + j];
		}
	}
}

void
extranspose4x3f(GLfloat src[16], GLfloat dst[9])
{
	int i, j;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			dst[i + j * 3] = src[i * 4 + j];
		}
	}
}

void
ex4x4to3x3f(GLfloat src[16], GLfloat dst[9])
{
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			dst[i * 4 + j] = src[i * 4 + j];
		}
	}
}

void
readanglecode(char *fnt, Glcontext *cxt)
{
	unsigned int texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Glmfont->texwidth, Glmfont->texheight,
	    0,
	    GL_RGBA, GL_UNSIGNED_BYTE, Glmfont->rgba);
	checkerr();
	glBindTexture(GL_TEXTURE_2D, 0);
	cxt->texture = texture;
	//GLfloat dim[4] = {0};
	//glGetIntegerv(GL_VIEWPORT, dim);
	//cxt->wres = 1;//2. / dim[2];
	//cxt->hres = 1;//2. / dim[3];
}

void
exlightmodelfv(GLenum  pname, const GLfloat *params)
{

	switch(pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		memcpy(curcxt->sceneambient, params, sizeof(float) * 4);
		break;
	case GL_LIGHT_MODEL_LOCAL_VIEWER:
		if (*params == 0.0) {
			curcxt->bitparms[LOCALVIEWER] = 0;
		} else {
			curcxt->bitparms[LOCALVIEWER] = 1;
		}
		break;
	case GL_LIGHT_MODEL_TWO_SIDE:
		if (*params == 0.0) {
			curcxt->bitparms[TWOSIDED] = 0;
		} else {
			curcxt->bitparms[TWOSIDED] = 1;
		}
		break;
	}
}

void
exlightmodelf(GLenum  pname, const GLfloat params)
{
	exlightmodelfv(pname, &params);
}

void
exlightfv(GLenum  light, GLenum  pname, const GLfloat *params)
{
	Ulights *ul = &curcxt->u_lights[light - GL_LIGHT0];
	float o[4] = {0}, spm[16] = {0}, nm[16] = {0}, i[4] = {0};
	const float *dir;

	switch(pname) {
	case GL_POSITION:
		vslmultmatrixpoint(curcxt->stviewmat.stack[curcxt->stviewmat.nval], params, o);
		memcpy(ul->fpos, o, sizeof(float) * 4);
		break;
	case GL_AMBIENT:
		memcpy(ul->famb, params, sizeof(float) * 4);
		break;
	case GL_DIFFUSE:
		memcpy(ul->fdif, params, sizeof(float) * 4);
		break;
	case GL_SPECULAR:
		memcpy(ul->fspec, params, sizeof(float) * 4);
		break;
	case GL_SPOT_EXPONENT:
		ul->fsexp = *params;
		break;
	case GL_SPOT_CUTOFF:
		ul->fsang = *params;
		break;
	case GL_CONSTANT_ATTENUATION:
		ul->fk[0] = *params;
		break;
	case  GL_LINEAR_ATTENUATION:
		ul->fk[1] = *params;
		break;
	case  GL_QUADRATIC_ATTENUATION:
		ul->fk[2] = *params;
		break;
	case GL_SPOT_DIRECTION:
		if (vslinvert(curcxt->stviewmat.stack[curcxt->stviewmat.nval], spm) == 0) {
			memcpy(i, params, sizeof(float) * 3);
			//extranspose4x4f(spm);
			//vslmultmatrixpoint(spm, i, o);
			//same as above
			vslmultpointmatrix(i, spm, o);
			dir = o;
		} else {
			dir = params;
		}
		memcpy(ul->fsdirc, dir, sizeof(float) * 3);
		break;
	default:
		assert(FALSE);
	}

}


void
exlighti(GLenum light, GLenum pname, GLint param)
{
	float cutof = param;
	//assert(pname == GL_SPOT_CUTOFF);
	exlightfv(light, pname, &cutof);
}

void
exlightf(GLenum light, GLenum pname, GLfloat param)
{
	exlightfv(light, pname, &param);
}

void
exrasterpos3f(float x, float y, float z)
{
	curcxt->rasterpos[0] = x;
	curcxt->rasterpos[1] = y;
	curcxt->rasterpos[2] = z;
	memset(curcxt->rasteroff, 0, sizeof(float) * 3);
}

void
exrasterpos2f(float x, float y)
{
	exrasterpos3f(x, y, 0);
}

void
exrasterpos2i(int x, int y)
{
	exrasterpos3f(x, y, 0);
}

void
exbitmapcharacter(GLvoid *style, char c)
{
	Glyph *g;
	float px = 0, py = 0, ratio;
	int depth = 0, noblend = 0;

	int dim[4] = {256, 256};
	glGetIntegerv(GL_VIEWPORT, dim);
	curcxt->wres = 2.0f / dim[2];
	curcxt->hres = 2.0f / dim[3];
	if (style == GLUT_BITMAP_HELVETICA_10)
		ratio = 10. / Glmfont->size;
	else if (style == GLUT_BITMAP_HELVETICA_12)
		ratio = 12. / Glmfont->size;
	else if (style == GLUT_BITMAP_HELVETICA_18)
		ratio = 18. / Glmfont->size;
	else
		ratio = 1.0;

	curcxt->wres *= ratio;
	curcxt->hres *= ratio;
	exbeginmode(GL_TRIANGLES);

	int dny, upy;
	g = &Glmfont->glyps[c];

	dny = g->yoff - (Glmfont->base - g->h);
	upy = g->h - dny;
	px = curcxt->rasteroff[0];
	py = curcxt->rasteroff[1];
	exvertex3f((px + g->xoff), (py + upy), 0);
	extexcoord2f(g->s1, g->t1);

	exvertex3f((px + g->xoff), (py - dny), 0);
	extexcoord2f(g->s1, g->t2);

	exvertex3f((px + g->xoff + g->w), (py + upy), 0);
	extexcoord2f(g->s2, g->t1);

	exvertex3f((px + g->xoff + g->w), (py + upy), 0);
	extexcoord2f(g->s2, g->t1);

	exvertex3f((px + g->xoff), (py - dny), 0);
	extexcoord2f(g->s1, g->t2);

	exvertex3f((px + g->xoff + g->w), py - dny, 0);
	extexcoord2f(g->s2, g->t2);
	px += g->adv;

	curcxt->rasteroff[0] = px;
	curcxt->rasteroff[1] = py;

	if ((depth = glIsEnabled(GL_DEPTH_TEST))) {
		glDisable(GL_DEPTH_TEST);
	};
	if ((noblend = !glIsEnabled(GL_BLEND))) {
		glEnable(GL_BLEND);
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, curcxt->texture);
	glUniform1i(curcxt->u_texttex, 0);
	glUniform3f(curcxt->u_textloc, curcxt->rasterpos[0], curcxt->rasterpos[1],
	    curcxt->rasterpos[2]);
	glUniform3f(curcxt->u_viewport, curcxt->wres, curcxt->hres, 0);
	curcxt->bitparms[TXTRENDER] = 1;
	exendmode();
	if (depth) glEnable(GL_DEPTH_TEST);
	if (noblend) glDisable(GL_BLEND);
	//
	curcxt->bitparms[TXTRENDER] = 0;
}

// From glut 3.7
void
exglutsolidtorus(GLfloat r, GLfloat R, GLint nsides, GLint rings)
{
	int i, j;
	GLfloat theta, phi, theta1;
	GLfloat cosTheta, sinTheta;
	GLfloat cosTheta1, sinTheta1;
	GLfloat ringDelta, sideDelta;

	ringDelta = 2.0 * M_PI / rings;
	sideDelta = 2.0 * M_PI / nsides;

	theta = 0.0;
	cosTheta = 1.0;
	sinTheta = 0.0;
	for (i = rings - 1; i >= 0; i--) {
		theta1 = theta + ringDelta;
		cosTheta1 = cos(theta1);
		sinTheta1 = sin(theta1);
		exbeginmode(GL_QUAD_STRIP);
		phi = 0.0;
		for (j = nsides; j >= 0; j--) {
			GLfloat cosPhi, sinPhi, dist;

			phi += sideDelta;
			cosPhi = cos(phi);
			sinPhi = sin(phi);
			dist = R + r * cosPhi;

			exnormal3f(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi);
			exvertex3f(cosTheta1 * dist, -sinTheta1 * dist, r * sinPhi);
			exnormal3f(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi);
			exvertex3f(cosTheta * dist, -sinTheta * dist,  r * sinPhi);
		}
		exendmode();
		theta = theta1;
		cosTheta = cosTheta1;
		sinTheta = sinTheta1;
	}
}

int
exbuildtex2d(GLenum target, GLenum intrfmt, GLsizei width, GLsizei height,
    GLenum format, GLenum type, const void * data, void texparams(void))
{
	// modify if using more than one sampler
	glGenTextures(1, &curcxt->textures[0]);
	glBindTexture(GL_TEXTURE_2D, curcxt->textures[0]);

	texparams();
	glTexImage2D(GL_TEXTURE_2D, 0, intrfmt, width, height, 0,
	    format, type, data);
	checkerr();
	glBindTexture(GL_TEXTURE_2D, 0);
	curcxt->ntex = 1;

	return 0;
}