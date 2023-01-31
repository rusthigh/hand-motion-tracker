// absdiff.frag - by blaz
// help from: http://forum.openframeworks.cc/index.php/topic,419.30/wap2.html
/*
mix.vert - use the built-in varying output
Code:
void main(void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = ftransform();   
} 


mix.frag - replace all occurences of sampler2DRect & texture2DRect
Code:
uniform sampler2DRect tex1;
uniform sampler2DRect tex2;

void main(void)
{
   // get texture info
   vec4 v