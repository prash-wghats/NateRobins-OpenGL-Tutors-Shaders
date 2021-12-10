#version 120

#if (defined(GL_ES) && __VERSION__ < 300) || (!defined(GL_ES) && __VERSION__ < 130)
#define IN attribute
#define OUT varying
#else
#define IN in
#define OUT out
#endif

#ifdef GL_ES
precision highp float;
precision highp int;
#endif


#define MAXLIGHTS 2
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

const vec4 defamb = vec4(0.2,0.2,0.2,1.0), defdiff = vec4(0.8,0.8,0.8,1.0), defzero3 = vec4(0.,0.,0.,1.0),
	defone = vec4(1.,1.,1.,1.0), defzero4 = vec4(0.,0.,0.,0.0), pos = vec4(0.,0.,1.,0.0);
const vec3 xone = vec3(1.0,0.,0.), spot = vec3(0.0,0.,-1.);
struct Light {
	vec4 position;
	vec4 ambient, diffuse, specular;
	vec3 kaklkq;
	vec3 spdirc;
	float spexp, spangle;
};

struct Material {
	vec4 ambient, diffuse, specular, emissive;
	float specexp;
};
struct Fog {
	int mode;
	float start, end, density;
	vec4 color;
};

uniform mat4 u_projection, u_modelview, u_texturematrix;
uniform mat3 u_normalmatrix;
uniform vec4 u_lightposition, u_color;
uniform vec3 u_eyeposition, u_textloc, u_viewport;

uniform bool u_params[MAXPARAMS], u_lightson[MAXLIGHTS];
uniform vec4 u_sceneambient;
uniform Material u_materials[2];
uniform Light u_lights[MAXLIGHTS];
uniform Fog u_fog;

/*
uniform int u_params = 0, u_lightson = LIGHT0;
uniform vec4 u_sceneambient = defamb;
uniform Material u_materials[2] = Material[2](
	Material(defamb, defdiff, defzero3, defzero3, 0),
	Material(defamb, defdiff, defzero3, defzero3, 0)
);
uniform Light u_lights[MAXLIGHTS] = Light[MAXLIGHTS](
	Light(pos, defzero3, defone, defone, xone, spot, 0, 180),
	Light(pos, defzero3, defzero3, defzero3, xone, spot, 0, 180)
);
*/
IN vec4 a_color;
IN vec3 a_position, a_normal;
IN vec2 a_texture;

OUT vec4 v_color;
OUT vec4 v_bcolor;
OUT float v_fog;
OUT vec2 v_texco;
OUT vec3 v_normal;

vec4 phong(int i, int mi, vec3 position, vec3 normal, vec4 color, vec3 eyedir)
{
	float diffuse,specular = 0.0, dist = 0.0, att = 1.0;
	vec3 lightdir;
	vec4 lightpos = u_lights[i].position;
	vec3 spotdir = u_lights[i].spdirc;
	float specexp = u_materials[i].specexp;
	float angle = u_lights[i].spangle;
	float spotexp = u_lights[i].spexp;
	vec4 fcolor = vec4(0.,0.,0.,1.);

	normal = normalize(normal);

	if (lightpos.w == 0.0) {
		lightdir = (lightpos).xyz;
		lightdir = normalize(lightdir);
		dist = 0.0;
	} else {
		lightdir = lightpos.xyz - position;
		dist = length(lightdir);
		lightdir = lightdir / dist;
	}

	att = 1. / (u_lights[i].kaklkq.x + u_lights[i].kaklkq.y * dist +
		pow(u_lights[i].kaklkq.z, 2) *dist);

	if (angle != 180.0) {
		//lightdir = normalize(lightdir);
		spotdir = normalize(spotdir);
		float ldots = dot(-lightdir, spotdir);
		float cang = cos(radians(angle));
		float spot = ldots >= cang ? pow(ldots, spotexp) : 0.;
		
		att *= spot;

	}
	
	diffuse = max(0.0, dot(normal, lightdir));
	if (diffuse != 0.0) {
		vec3 hv = normalize(lightdir + eyedir);
		float hdotn = max(0.0, dot(normal, hv));
		specular = pow(hdotn, specexp);
	}
	if (u_params[MATTRACKCOL]) {
		fcolor = att * (
		u_lights[i].ambient * (u_params[MATAMBIENT] ? u_materials[i].ambient : color) +
		u_lights[i].diffuse * (u_params[MATDIFFUSE] ? u_materials[i].diffuse : color) * diffuse +
		u_lights[i].specular * (u_params[MATSPECULAR] ? u_materials[i].specular : color) * specular);
	} else {
	fcolor = att * (
		u_lights[i].ambient *  u_materials[i].ambient +
		u_lights[i].diffuse * u_materials[i].diffuse * diffuse +
		u_lights[i].specular *  u_materials[i].specular * specular);
	}

	return fcolor;
}

vec4 lighting(vec3 position, vec3 normal, vec4 color, int side)
{
	int no = 1;
	vec4 col = vec4(0.0);
	vec3 eyepos; 
	
	for (int i = 0; i < MAXLIGHTS; i++) {
		if (u_lightson[i]) {
			if (u_params[LOCALVIEWER]) {
				vec4 e =  vec4(0.,0.,0.,1.);
				eyepos = normalize(e.xyz - position);
				
			} else
				eyepos = vec3(0,0,1);
			//eyepos = (u_params[LOCALVIEWER]) ?
			//	vec3(0) : vec3(0,0,1);
			col += phong(i, side, position, normal, color, eyepos);
		}
	}

	if (u_params[MATTRACKCOL]) {
		col = col + u_materials[side].ambient * u_sceneambient + 
		(u_params[MATEMISSIVE] ? u_materials[side].emissive : color);
	} else {
		col = col + u_materials[side].ambient * u_sceneambient + 
		u_materials[side].emissive;
	}
	
	col = min(col.rgba, vec4(1.0));
	
	return col;
}

void calcfog(vec4 position)
{
	float dist = length(position.xyz);
	if (u_fog.mode == FOG_EXP) {
		v_fog = exp(-u_fog.density * dist);
	} else if (u_fog.mode == FOG_EXP2) {
		v_fog = exp(-(pow(u_fog.density * dist, 2)));
	} else {
		v_fog = (u_fog.end - dist) / (u_fog.end - u_fog.start);
	}
	v_fog = clamp(v_fog, 0, 1);
}

void main()
{
	vec4 position;

	
	v_normal = a_normal;
	v_texco = (u_texturematrix * vec4(a_texture, 0, 1)).xy;

	if (u_params[TXTRENDER]) {
		position = u_projection * u_modelview * vec4(u_textloc, 1.0);
		position = position / position.w;
		position = vec4(position.xy + (a_position * u_viewport).xy, 0, 1.0);
		///position = vec4(position.xy, 0, 1);
		//position = u_modelview * vec4(u_textloc, 1.0);
		//position = vec4(position.xyz + a_position, 1.0);
		//position = u_projection * position;
		v_color = a_color;
	} else if (u_params[LIGHTING]) {
		vec4 mvpos = u_modelview * vec4(a_position, 1.0);
		//vec3 normal = (u_modelview * vec4(a_normal, 0.0)).xyz;
		vec3 normal = u_normalmatrix * a_normal;
		v_color = lighting(mvpos.xyz, normal, a_color, 0);
		v_bcolor = lighting(mvpos.xyz, -normal, a_color, 1);
		position = u_projection * mvpos;
	} else {
		v_color = a_color;
		position = u_projection * u_modelview * vec4(a_position, 1.0);
	}
	 if (u_params[FOG]) {
		 vec4 mvpos = u_modelview * vec4(a_position, 1.0);
		 calcfog(mvpos);
	 }
	gl_Position = position;

}