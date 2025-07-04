//this file is autogenerated using stringify.bat (premake --stringify) in the build folder of this project
static tukk segmentationMaskInstancingFragmentShader= \
"#version 330\n"
"precision highp float;\n"
"in vec4 scale_obuid;\n"
"out vec4 color;\n"
"void main(void)\n"
"{\n"
"	highp i32 obuid = i32(scale_obuid.w);\n"
"	float r = ((obuid>>0 )&0xff)*(1./255.f);\n"
"	float g = ((obuid>>8 )&0xff)*(1./255.f);\n"
"	float b = ((obuid>>16)&0xff)*(1./255.f);\n"
"	float a = ((obuid>>24)&0xff)*(1./255.f);\n"
"	color  = vec4(r,g,b,a);\n"
"}\n"
;
