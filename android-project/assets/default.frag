precision highp	float;

uniform vec2	resolution;
uniform float	time;
uniform vec3	cam_pos;

varying	vec2	v_texcoord;

const 	float	PI			= 3.14159265;


// Primitives

vec2 obj_box (in vec3 p, in vec3 s)
{
	return vec2(length(max(abs(p)-s, 0.0)), 0);
}

vec2 obj_floor (in vec3 p, in float height)
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


vec2 distance_to_obj (in vec3 p)
{
	return obj_union(obj_box(p, vec3(1.0, 1.0, 2.0)), obj_floor(p, 10.0));
}


void main (void)
{
	const float	maxdist		= 60.0;

	// Position on the rendering surface
	//vec2 		sfc_pos 	= 2.0*v_texcoord.xy - 1.0;
	vec2 sfc_pos=-1.0+2.0*gl_FragCoord.xy/resolution.xy;

	//Camera setup
	const vec3 	cam_up		= vec3(0, 1, 0);
	const vec3 	cam_lookat	= vec3(0, 0, 0);

	vec3 		cam_dir		= normalize(cam_lookat-cam_pos);

	vec3 		u			= normalize(cross(cam_up, cam_dir));
	vec3 		v			= cross(cam_dir, u);

	vec3 		vcv			= cam_pos + cam_dir;
	vec3 		scr_coord	= vcv + u*sfc_pos.x*resolution.x/resolution.y + v*sfc_pos.y;
	vec3 		ray_normal	= normalize(scr_coord-cam_pos);

	const vec3	e			= vec3(0.1, 0.0, 0.0);

	vec2		d			= vec2(0.1, 0.0);
	
	float 		dist		= 1.0;
	vec3 		color, ray_pos, N;

	// Step until an object is hit
	for (int i = 0; i < 256; i++)
	{
		if (abs(d.x) < 0.01 || dist > maxdist)
			break;
		dist	+=	d.x;
		ray_pos	=	cam_pos + ray_normal*dist;
		d		=	distance_to_obj(ray_pos);
	}

	if (dist < maxdist)
	{
		//if (d.y == 0.0)
		color = vec3(0.6, 0.6, 0.8);

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
