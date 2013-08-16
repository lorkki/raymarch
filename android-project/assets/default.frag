#version 100

precision highp	float;

uniform vec2	resolution;
uniform float	time;
uniform vec3	cam_pos;

varying	vec2	v_texcoord;

const 	float	PI			= 3.14159265;


// Primitives

vec2 obj_box (in vec3 p, in vec3 s, float tex)
{
	return vec2(length(max(abs(p)-s, 0.0)), tex);
}

vec2 obj_sphere (in vec3 p, float s, float tex)
{
	return vec2(length(p)-s, tex);
}

vec2 obj_torus (in vec3 p, in vec2 t, float tex)
{
	vec2 q = vec2(length(p.xz)-t.x, p.y);
	return vec2(length(q)-t.y, tex);
}

vec2 obj_cylinder (in vec3 p, in vec3 c, float tex)
{
	return vec2(length(p.xz-c.xy)-c.z, tex);
}

vec2 obj_floor (in vec3 p, in float height, float tex)
{
	return vec2(p.y+height, 0);
}


// Combining objects

vec2 obj_union (in vec2 d1, in vec2 d2)
{
	if (d1.x < d2.x)
		return d1;
	else
		return d2;
}

vec2 obj_intersect (in vec2 d1, in vec2 d2)
{
	if (d1.x > d2.x)
		return d1;
	else
		return d2;
}


// Displacements

vec2 obj_on_acid (in vec3 p)
{
	return vec2(
		0.5 * sin(0.5*p.x + 0.5*time) * sin(0.7*p.y) * sin(1.1*p.z + 0.7*time),
		0.0
		);
}


// Scene compositing

vec3 repeat (in vec3 p, in vec3 c)
{
	return mod(p, c) - 0.5*c;
}

vec2 distance_to_obj (in vec3 p)
{
	vec3 p_sp1 = p - vec3(2.0+sin(0.7*time),	0.0,				cos(0.6*time));

	return obj_union(
		obj_floor(p, 10.0, 0.0),
		obj_union(
			obj_box(p, vec3(1.0, 1.0, 2.0), 1.0), //+ obj_on_acid(p),
			obj_sphere(p_sp1, 0.7, 2.0)
			)
		);
}


void main (void)
{
	const float	maxdist		= 60.0;

	// Position on the rendering surface
	vec2		sfc_pos		= -1.0 + 2.0*gl_FragCoord.xy/resolution.xy;

	//Camera setup
	const vec3 	cam_up		= vec3(0, 1, 0);
	const vec3 	cam_lookat	= vec3(0, 0, 0);

	vec3 		cam_dir		= normalize(cam_lookat-cam_pos);

	vec3 		u			= normalize(cross(cam_up, cam_dir));
	vec3 		v			= cross(cam_dir, u);

	vec3 		vcv			= cam_pos + cam_dir;
	vec3 		scr_coord	= vcv + u*sfc_pos.x*resolution.x/resolution.y + v*sfc_pos.y;
	vec3 		ray_normal	= normalize(scr_coord-cam_pos);

	const vec3	e			= vec3(0.05, 0.0, 0.0);

	vec2		d			= vec2(0.1, 0.0);
	
	float 		dist		= 1.0;
	vec3 		color, ray_pos, N;

	// Step until an object is hit
	for (int i = 0; i < 256; i++)
	{
		if (abs(d.x) < 0.02 || dist > maxdist)
			break;
		dist	+=	d.x;
		ray_pos	=	cam_pos + ray_normal*dist;
		d		=	distance_to_obj(ray_pos);
	}

	if (dist < maxdist)
	{
		if (d.y == 0.0)
			color = vec3(0.02, 0.51, 0.62);
		else if (d.y == 1.0)
			color = vec3(1.00, 0.69, 0.00);
		else if (d.y == 2.0)
			color = vec3(1.00, 0.11, 0.00);
		else
			color = vec3(1.00, 0.50, 0.00);

		// Simple lighting

		N = normalize(
			vec3(
				d.x - distance_to_obj(ray_pos - e.xyy).x,
				d.x - distance_to_obj(ray_pos - e.yxy).x,
				d.x - distance_to_obj(ray_pos - e.yyx).x
			)
		);

		float b = dot(N, normalize(cam_pos - ray_pos));

		gl_FragColor = vec4((b*color + pow(b, 8.0)) * (1.0 - dist*.01), 1.0);
	}
	else
	{
		gl_FragColor = vec4(0, 0, 0, 1);
	}
}
