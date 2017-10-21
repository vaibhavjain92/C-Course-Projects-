attribute vec2 param;

uniform mat4 projection,modelView;
uniform vec3 ctrlpts[16];

varying float ycoord;

vec3 CubicBezierCurve(vec3 ctp0, vec3 ctp1, vec3 ctp2, vec3 ctp3,float s)
{
	vec3 a[3]= vec3[3]
	(
		ctp0*(1.0-s)+ctp1*s,
		ctp1*(1.0-s)+ctp2*s,
		ctp2*(1.0-s)+ctp3*s
	);
	vec3 b[2]= vec3[2]
	(
		a[0]*(1.0-s)+a[1]*s,
		a[1]*(1.0-s)+a[2]*s
	);
	return b[0]*(1.0-s)+b[1]*s;;
	
}

vec3 CubicBezierSurface(float s, float t)
{
	vec3 x1 = CubicBezierCurve(ctrlpts[0], ctrlpts[1], ctrlpts[2], ctrlpts[3],s);	
	vec3 x2 = CubicBezierCurve(ctrlpts[4], ctrlpts[5], ctrlpts[6], ctrlpts[7],s);
	vec3 x3 = CubicBezierCurve(ctrlpts[8], ctrlpts[9], ctrlpts[10], ctrlpts[11],s);
	vec3 x4 = CubicBezierCurve(ctrlpts[12], ctrlpts[13], ctrlpts[14], ctrlpts[15],s);	
	return CubicBezierCurve(x1,x2,x3,x4,t);
}

void main()
{
	
	vec3 vertex = CubicBezierSurface(param[0], param[1]);
	gl_Position=projection*modelView*vec4(vertex,1.0);	
	ycoord = vertex.y;
}
