#pragma once

#define TRUE	1
#define FALSE	0

float *vsllookat(float *m, float xPos, float yPos, float zPos, float xLook,
    float yLook, float zLook, float xUp, float yUp, float zUp);
float *vslperspective(float *m, float fov, float ratio, float nearp,
    float farp);
float *vslortho2d(float *m, float left, float right, float bottom, float top);
float *vslortho(float *m, float left, float right, float bottom, float top,
    float nearp, float farp);
float *
vslfrustum(float * m, float left, float right, float bottom, float top,
    float nearp, float farp);
void vsltranslate(float *aType, float x, float y, float z);
void vslscale(float *aType, float x, float y, float z);
void vslrotate(float *aType, float angle, float x, float y, float z);
void vslsetidentitymatrix(float *mat, int size);
void vslmultmatrix(float *aType, float *aMatrix);
void vslmultmatrixdst(float *aType, float *aMatrix, float *dst);
void vslmultmatrixd(double *aType, double *aMatrix);
void vslmultmatrixdstd(double *aType, double *aMatrix, double *dst);

void vsltranslate(float *aType, float x, float y, float z);
void vslscale(float *aType, float x, float y, float z);
void vslrotate(float *aType, float angle, float x, float y, float z);
void vslcrossproduct(float *a, float *b, float *res);
float vsldotProduct(float *a, float *b);
void vslnormalize(float *a);
void vslsubtract(float *a, float *b, float *res);
void vsladd( float *a, float *b, float *res);
float vsllength(float *a);
void vslmultmatrixpoint(const float *aType, const float *point, float *res);
void vslmultpointmatrix(const float *point, const float *aType, float *res);
int vslinvert(float f4x4[], float inv3x3[]);

void exinitialize(int i);
void exrendtxt(GLvoid *style, char *txt, float x, float y, float z);
void exrendtxt0(GLvoid *style, char *txt, float x, float y, float z);
void excolor3ub(GLubyte red, GLubyte green, GLubyte blue);
void excolor3f(float x, float y, float z);
void excolor4f(float x, float y, float z, float w);
void exvertex3f(float x, float y, float z);
void exvertex3i(int x, int y, int z);
void exbeginmode(GLenum mode);
void exendmode();
void exrotatef(GLfloat angle, GLfloat x,  GLfloat y,  GLfloat z);
void extranslatef(GLfloat x, GLfloat y, GLfloat z);
void exscalef(GLfloat x, GLfloat y, GLfloat z);
void exsetmatrixmode(int mode, int reset);
void expushmatrix();
void expopmatrix();
void exmultmatrixd(const GLdouble *m);
void exlightfv(GLenum  light, GLenum  pname, const GLfloat *params);
void exmaterialfv(GLenum face, GLenum pname, const GLfloat *params);
void exdisable(GLenum mode);
void exenable(GLenum mode);
void exgetparamd(GLenum mode, double *params);

void experspective(float fov, float ratio, float nearp, float farp);
void exortho2d(float left, float right, float bottom, float top);
void exlookat(float xpos, float ypos, float zpos, float xlook,
    float ylook, float zlook, float xup, float yup, float zup);
void exortho(float left, float right, float bottom, float top,
    float nearp, float farp);
void exfrustum(float left, float right, float bottom, float top,
    float nearp, float farp);
void exvertex3fv(const GLfloat *p);
void exnormal3fv(const GLfloat *p);
void exnormal3f(float x, float y, float z);
void extexcoord2fv(const GLfloat *p);
void
extexcoord2f(float x, float y);
void excolor3fv(const GLfloat *p);
void exuseprogram(int n);
void exsubinitialize(int n);

GLboolean exinvertd(GLdouble src[16], GLdouble inverse[16]);
void extranspose4x4d(GLdouble srcdst[16]);
void extranspose4x3d(GLdouble src[16], GLdouble dst[9]);
void extranspose4x4f(GLfloat srcdst[16]);
void extranspose4x3f(GLfloat src[16], GLfloat dst[9]);
void ex4x4to3x3f(GLfloat src[16], GLfloat dst[9]);
void exlightf(GLenum light, GLenum pname, GLfloat param);
void exlighti(GLenum light, GLenum pname, GLint param);
void exlightmodelf(GLenum  pname, const GLfloat params);
void exlightmodelfv(GLenum  pname, const GLfloat *params);
void exrasterpos3f(float x, float y, float z);
void exrasterpos2f(float x, float y);
void exrasterpos2i(int x, int y);
void exbitmapcharacter(GLvoid *style, char c);
void exmaterialf(GLenum face, GLenum pname, const GLfloat params);
void exglutsolidtorus(GLfloat r, GLfloat R, GLint nsides, GLint rings);
GLboolean exinvertf(GLfloat src[16], GLfloat inverse[16]);
void exfogf(GLenum pname, GLfloat params);
void exfogi(GLenum pname, int params);
void exfogfv(GLenum pname, GLfloat *params);
void exvertex2i(int x, int y);
void exvertex2f(float x, float y);
int exbuildtex2d(GLenum target, GLenum intrfmt, GLsizei width, GLsizei height,
    GLenum format, GLenum type, const void * data, void texparams(void));

typedef struct Glyph Glyph;
typedef struct Anglecode Anglecode;
struct Anglecode {
	int padding[4];
	int spacing[2];
	int lineheight;
	int texwidth, texheight;
	int count, size, base;
	char name[32];
	Glyph *glyps;
	int glysz;
	unsigned char *rgba;
};

struct Glyph {
	char c;
	float s1, t1;    //Top left s,t
	float s2, t2;    //bottom right s,t
	float w, h, xoff, yoff, adv;
};