// match.frag (calcs abs diff image matching in squared regions) - by blaz

uniform sampler2DRect sampler0;
//uniform vec2 mouse; // if testing with mouse
uniform int kernel_width;
uniform int kernel_height;

uniform int frag_width;
uniform int frag_height;

//uniform vec2i kernel_size; // if we could use this would be better (but it wont work on with i)

void main(void)
{	
/*----------