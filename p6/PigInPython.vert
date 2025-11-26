#version 120
// 通过 smoothstep 构造鼓包：在 uPigW 范围内放大 Y/Z 截面，输出 vL/vN/vE 供片段光照

uniform float uPigD;
uniform float uPigH;
uniform float uPigW;

varying vec3  vL;
varying vec3  vN;
varying vec3  vE;

const vec3 LightPosition     = vec3( 15.0, 15.0, 15.0 );

void
main( )
{
    vec4 MCvertex = gl_Vertex;

    // pig-in-the-python: smooth pulse along X, scale YZ
    float x = MCvertex.x;
    float pulse = smoothstep(uPigD - uPigW/2.0, uPigD, x)
                - smoothstep(uPigD, uPigD + uPigW/2.0, x);
    float yzscale = 1.0 + pulse * uPigH;
    MCvertex.y *= yzscale;
    MCvertex.z *= yzscale;

    vec4 ECposition = gl_ModelViewMatrix * MCvertex; // eye coordinate position

    vN = normalize( gl_NormalMatrix * gl_Normal ); // normal vector

    vL = LightPosition - ECposition.xyz; // vector from the point to the light position

    vE = vec3( 0.0, 0.0, 0.0 ) - ECposition.xyz; // vector from the point to the eye position

    gl_Position = gl_ModelViewProjectionMatrix * MCvertex;
}
