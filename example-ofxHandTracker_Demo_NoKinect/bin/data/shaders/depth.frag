// depth.frag
//
uniform sampler2DRect sampler0;
//uniform vec2 mouse; // if testing with mouse
//uniform int kernel_size;

varying float DEPTH;

void main(void)
{	
	// far things appear white, near things black
	//gl_Color.rgb=vec3(DEPTH,DEPTH,DEPT