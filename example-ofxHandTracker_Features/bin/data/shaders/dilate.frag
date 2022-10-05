// dilation.fs
//
// maximum of 3x3 kernel

uniform sampler2DRect sampler0;
//uniform vec2 mouse; // if testing with mouse
uniform int kernel_size;

// useful random function?
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{	/* // alter fragments only around mouse coords
	float dist = distance(gl_FragCoord.xy, mouse); 
	float radius = 240.0;
	
	if(dist < radius) {
		int size = kernel_size;
		vec4 maxValue = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 currValue;
		for (int i = -size; i<=size; i++) {
			for (in