/* --------------------------------------------------

Lighthouse3D

VSMathLib - Very Simple Matrix Library

http://www.lighthouse3d.com/very-simple-libs

----------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#if __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h> 
#endif

#include "ex.h"

#if defined(_MSC_VER)
#define M_PI       3.14159265358979323846f
#endif

// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void
vslsetidentitymatrix(float *mat, int size)
{

	// fill matrix with 0s
	for (int i = 0; i < size * size; ++i)
		mat[i] = 0.0f;

	// fill diagonal with 1s
	for (int i = 0; i < size; ++i)
		mat[i + i * size] = 1.0f;
}

float *
vsllookat(float *mat, float xPos, float yPos, float zPos, float xLook, float yLook, float zLook,
    float xUp, float yUp, float zUp)
{
	float dir[3], right[3], up[3];

	up[0] = xUp;
	up[1] = yUp;
	up[2] = zUp;

	dir[0] =  (xLook - xPos);
	dir[1] =  (yLook - yPos);
	dir[2] =  (zLook - zPos);
	vslnormalize(dir);

	vslcrossproduct(dir, up, right);
	vslnormalize(right);

	vslcrossproduct(right, dir, up);
	vslnormalize(up);

	float m1[16], m2[16];

	m1[0]  = right[0];
	m1[4]  = right[1];
	m1[8]  = right[2];
	m1[12] = 0.0f;

	m1[1]  = up[0];
	m1[5]  = up[1];
	m1[9]  = up[2];
	m1[13] = 0.0f;

	m1[2]  = -dir[0];
	m1[6]  = -dir[1];
	m1[10] = -dir[2];
	m1[14] =  0.0f;

	m1[3]  = 0.0f;
	m1[7]  = 0.0f;
	m1[11] = 0.0f;
	m1[15] = 1.0f;

	vslsetidentitymatrix(m2, 4);
	m2[12] = -xPos;
	m2[13] = -yPos;
	m2[14] = -zPos;

	vslmultmatrix(mat, m1);
	vslmultmatrix(mat, m2);
	return mat;
}

float *
vslperspective(float mat[], float fov, float ratio, float nearp, float farp)
{
	float projMatrix[16];

	float f = 1.0f / tanf (fov * ((float)M_PI / 360.0f));

	vslsetidentitymatrix(projMatrix, 4);

	projMatrix[0] = f / ratio;
	projMatrix[1 * 4 + 1] = f;
	projMatrix[2 * 4 + 2] = (farp + nearp) / (nearp - farp);
	projMatrix[3 * 4 + 2] = (2.0f * farp * nearp) / (nearp - farp);
	projMatrix[2 * 4 + 3] = -1.0f;
	projMatrix[3 * 4 + 3] = 0.0f;

	vslmultmatrix(mat, projMatrix);

	return mat;
}

// glOrtho implementation
float *
vslortho(float *m, float left, float right, float bottom, float top,
    float nearp, float farp)
{

	vslsetidentitymatrix(m, 4);

	m[0 * 4 + 0] = 2 / (right - left);
	m[1 * 4 + 1] = 2 / (top - bottom);
	m[2 * 4 + 2] = -2 / (farp - nearp);
	m[3 * 4 + 0] = -(right + left) / (right - left);
	m[3 * 4 + 1] = -(top + bottom) / (top - bottom);
	m[3 * 4 + 2] = -(farp + nearp) / (farp - nearp);

	return m;
}

float *
vslortho2d(float *m, float left, float right, float bottom, float top)
{
	return vslortho(m, left, right, bottom, top, -1, 1);
}

// glFrustum implementation
float *
vslfrustum(float * m, float left, float right, float bottom, float top,
    float nearp, float farp)
{
	vslsetidentitymatrix(m, 4);

	m[0 * 4 + 0] = 2 * nearp / (right - left);
	m[1 * 4 + 1] = 2 * nearp / (top - bottom);
	m[2 * 4 + 0] = (right + left) / (right - left);
	m[2 * 4 + 1] = (top + bottom) / (top - bottom);
	m[2 * 4 + 2] = - (farp + nearp) / (farp - nearp);
	m[2 * 4 + 3] = -1.0f;
	m[3 * 4 + 2] = - 2 * farp * nearp / (farp - nearp);
	m[3 * 4 + 3] = 0.0f;

	return m;
}

void
vslmultmatrix(float *aType, float *aMatrix)
{

	float *a, *b, res[16];
	a = aType;
	b = aMatrix;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}
	memcpy(aType, res, 16 * sizeof(float));
}

void
vslmultmatrixdst(float *aType, float *aMatrix, float *dst)
{

	float *a, *b, *res = dst;
	a = aType;
	b = aMatrix;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}
}

void
vslmultmatrixd(double *aType, double *aMatrix)
{

	double *a, *b, res[16];
	a = aType;
	b = aMatrix;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}
	memcpy(aType, res, 16 * sizeof(double));
}

void
vslmultmatrixdstd(double *aType, double *aMatrix, double *dst)
{

	double *a, *b, *res = dst;
	a = aType;
	b = aMatrix;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}
}

void
vsltranslate(float aType[], float x, float y, float z)
{
	float mat[16];

	vslsetidentitymatrix(mat, 4);
	mat[12] = x;
	mat[13] = y;
	mat[14] = z;

	vslmultmatrix(aType, mat);
}

void
vslscale(float aType[], float x, float y, float z)
{
	float mat[16];

	vslsetidentitymatrix(mat, 4);
	mat[0] = x;
	mat[5] = y;
	mat[10] = z;

	vslmultmatrix(aType, mat);
}
#define DegToRad(ang)	((ang * M_PI) / 180)
void
vslrotate(float *aType, float angle, float x, float y, float z)
{
	float mat[16];
	float v[3];

	v[0] = x;
	v[1] = y;
	v[2] = z;

	float radAngle = DegToRad(angle);
	float co = cosf(radAngle);
	float si = sinf(radAngle);
	vslnormalize(v);
	float x2 = v[0] * v[0];
	float y2 = v[1] * v[1];
	float z2 = v[2] * v[2];

//	mat[0] = x2 + (y2 + z2) * co;
	mat[0] = co + x2 * (1 - co);// + (y2 + z2) * co;
	mat[4] = v[0] * v[1] * (1 - co) - v[2] * si;
	mat[8] = v[0] * v[2] * (1 - co) + v[1] * si;
	mat[12] = 0.0f;

	mat[1] = v[0] * v[1] * (1 - co) + v[2] * si;
//	mat[5] = y2 + (x2 + z2) * co;
	mat[5] = co + y2 * (1 - co);
	mat[9] = v[1] * v[2] * (1 - co) - v[0] * si;
	mat[13] = 0.0f;

	mat[2] = v[0] * v[2] * (1 - co) - v[1] * si;
	mat[6] = v[1] * v[2] * (1 - co) + v[0] * si;
//	mat[10]= z2 + (x2 + y2) * co;
	mat[10] = co + z2 * (1 - co);
	mat[14] = 0.0f;

	mat[3] = 0.0f;
	mat[7] = 0.0f;
	mat[11] = 0.0f;
	mat[15] = 1.0f;

	vslmultmatrix(aType, mat);
}

// inversion implementation
int
vslinvert(float *mat, float *dmat)
{

	float    tmp[12]; /* temp array for pairs                      */
	float    src[16]; /* array of transpose source matrix */
	float    det;     /* determinant                                  */
	float	dst[16];
	/* transpose matrix */
	for (int i = 0; i < 4; ++i) {
		src[i] = mat[i * 4];
		src[i + 4] = mat[i * 4 + 1];
		src[i + 8] = mat[i * 4 + 2];
		src[i + 12] = mat[i * 4 + 3];
	}

	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];

	/* calculate first 8 elements (cofactors) */
	dst[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
	dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
	dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
	dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
	dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
	dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
	dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
	dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
	dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
	dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
	dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
	dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
	dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
	dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
	dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
	dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0] = src[2] * src[7];
	tmp[1] = src[3] * src[6];
	tmp[2] = src[1] * src[7];
	tmp[3] = src[3] * src[5];
	tmp[4] = src[1] * src[6];
	tmp[5] = src[2] * src[5];
	tmp[6] = src[0] * src[7];
	tmp[7] = src[3] * src[4];
	tmp[8] = src[0] * src[6];
	tmp[9] = src[2] * src[4];
	tmp[10] = src[0] * src[5];
	tmp[11] = src[1] * src[4];

	/* calculate second 8 elements (cofactors) */
	dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
	dst[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
	dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
	dst[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
	dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
	dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
	dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
	dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
	dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
	dst[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
	dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
	dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
	dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
	dst[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
	dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
	dst[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];

	/* calculate determinant */
	det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];

	if (fabs(det) < 0.00001)
		return -1;

	/* calculate matrix inverse */
	det = 1 / det;
	for (int j = 0; j < 16; j++)
		dmat[j] = dst[j] * det;
	
	return 0;
}

void
vslcrossproduct(float *a, float *b, float *res)
{

	res[0] = a[1] * b[2]  -  b[1] * a[2];
	res[1] = a[2] * b[0]  -  b[2] * a[0];
	res[2] = a[0] * b[1]  -  b[0] * a[1];
}


// returns a . b
float
vsldotProduct(float *a, float *b)
{

	float res = a[0] * b[0]  +  a[1] * b[1]  +  a[2] * b[2];

	return res;
}


// Normalize a vec3
void
vslnormalize(float *a)
{

	float mag = sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]);

	a[0] /= mag;
	a[1] /= mag;
	a[2] /= mag;
}


// res = b - a
void
vslsubtract(float *a, float *b, float *res)
{

	res[0] = b[0] - a[0];
	res[1] = b[1] - a[1];
	res[2] = b[2] - a[2];
}


// res = a + b
void
vsladd( float *a, float *b, float *res)
{

	res[0] = b[0] + a[0];
	res[1] = b[1] + a[1];
	res[2] = b[2] + a[2];
}


// returns |a|
float
vsllength(float *a)
{

	return(sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]));

}

// Compute res = M * point
void
vslmultmatrixpoint(const float *aType, const float *point, float *res)
{

	for (int i = 0; i < 4; ++i) {

		res[i] = 0.0f;

		for (int j = 0; j < 4; j++) {

			res[i] += point[j] * aType[j * 4 + i];
		}
	}
}


// Compute res = point * M
void
vslmultpointmatrix(const float *point, const float *aType, float *res)
{
	for (int i = 0; i < 4; ++i) {

		res[i] = 0.0f;

		for (int j = 0; j < 4; j++) {

			res[i] += point[j] * aType[i * 4 + j];
		}
	}
}
