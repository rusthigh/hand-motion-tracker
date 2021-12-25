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
/*---------------------------------------------------------------------------------------------------------
	//float kernel_w_f = float(kernel_width);
	//float kernel_h_f = float(kernel_width);
	
	vec2 kernel_size_f = vec2(float(kernel_width), float(kernel_height));
	
	vec2 frag_coord = gl_TexCoord[0].st;
	
	float max_val = kernel_width * kernel_height; // max value for later normalization
	vec4 maxValue = vec4(max_val, max_val, max_val, max_val);
	
	int scaled_kernel_width = kernel_width * frag_width;
	int scaled_kernel_height = kernel_height * frag_height;
	
	// run kernel sum operation only in %kernel == 0 regions
	if((int(frag_c