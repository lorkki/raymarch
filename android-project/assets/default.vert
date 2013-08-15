attribute	vec2	a_texcoord;
attribute	vec4	a_vertex;

varying		vec2	v_texcoord;

void main (void)
{
	v_texcoord	= a_texcoord;
	gl_Position	= a_vertex;
}
