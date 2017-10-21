attribute vec3 vertex;
attribute vec3 normal;
attribute vec4 color;
uniform mat4 projection,modelView;
uniform vec3 lightDir;
uniform float ambient;
uniform float specularIntensity;
uniform float specularExponent;
varying vec4 colorOut;

void main()
{
    vec3 nom=normalize((modelView*vec4(normal,0.0)).xyz);
    vec3 lit=normalize(lightDir);

    float diffuse=max(0.0,dot(nom,lit));

    vec4 posInView=modelView*vec4(vertex,1.0);
    vec3 viewDir=-normalize(posInView.xyz);
    vec3 midDir=normalize(viewDir+lightDir);
    float specular=specularIntensity*pow(max(0.0,dot(midDir,nom)),specularExponent);

    colorOut=vec4(color.rgb*(ambient+diffuse),color.a)
            +vec4(specular,specular,specular,0.0);

    gl_Position=projection*modelView*vec4(vertex,1.0);
}
