// dilation.fs
//
// maximum of 3x3 kernel

uniform sampler2DRect sampler0;
//uniform vec2 mouse; // if testing with mouse
uniform int kernel_size;

// useful random function?
float ra