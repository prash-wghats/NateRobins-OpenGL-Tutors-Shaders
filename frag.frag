#version 120

#if (defined(GL_ES) && __VERSION__ < 300) || (!defined(GL_ES) && __VERSION__ < 130)
#extension GL_OES_standard_derivatives : enable
#define GLSL 0
#else
#define GLSL 1
#endif

#ifdef GL_ES
precision mediump float;
#endif

#if GLSL == 0
#define IN varying
#define FragColor gl_FragColor
#define FTEXTURE texture2D
#else
#define IN in
out vec4 FragColor;
#define FTEXTURE texture
#endif

#define MAXPARAMS 13
#define TWOSIDED	2
#define TXTRENDER	10
#define FOG	11
#define TEXTURE	12

struct Fog {
	int mode;
	float start, end, density;
	vec4 color;
};

uniform sampler2D u_texunit1, u_texttex;
uniform bool u_params[MAXPARAMS];
uniform Fog u_fog;

IN vec4 v_color;
IN vec4 v_bcolor;
IN vec2 v_texco;
IN vec3 v_normal;
IN float v_fog;

void main()
{
	vec4 color = vec4(1.0,0,0,1.);
	if (u_params[TXTRENDER]) {

		float width = 0.49, edge = 0.1;
		float distance = FTEXTURE(u_texttex, v_texco).a;

		//edge = 1. * fwidth(distance);

		float alpha = smoothstep(width - edge, width + edge, distance);
		color = vec4(v_color.rgb, alpha);
		//color = texcol;
	} else if (u_params[TEXTURE]) {
		vec4 tex = FTEXTURE(u_texunit1, v_texco);
		color = tex;
	} else {
		if (u_params[TWOSIDED]) {
		if (gl_FrontFacing)
		color = v_color;
		else
		color = v_bcolor;
		} else {
			color = v_color;
		}
	}

	if (u_params[FOG]) {
		color = v_fog * color + (1 - v_fog) * u_fog.color;
	}

	FragColor = color;
}