#version 120
// 片段着色：使用插值后的 vL/vN/vE 做 Phong 光照，底色为亮橙色，突出鼓包区域

varying vec3  vN;                   // normal vector
varying vec3  vL;                   // vector from point to light
varying vec3  vE;                   // vector from point to eye

const vec3 Color         = vec3( 1.0, 0.65, 0.2 );
const vec3 SpecularColor = vec3( 1.0, 1.0, 1.0 );
const float Ka           = 0.1;
const float Kd           = 0.6;
const float Ks           = 0.3;
const float Shininess    = 30.0;


void
main( )
{
        vec3 myColor = Color;

        vec3 Normal    = normalize(vN);
        vec3 Light     = normalize(vL);
        vec3 Eye       = normalize(vE);

        vec3 ambient = Ka * myColor;

        float dd = max( dot(Normal,Light), 0.0 );       // only do diffuse if the light can see the point
        vec3 diffuse = Kd * dd * myColor;

        float s = 0.0;
        if( dd > 0.0 )              // only do specular if the light can see the point
        {
                vec3 ref = normalize(  reflect( -Light, Normal )  );
                float cosphi = dot( Eye, ref );
                if( cosphi > 0.0 )
                        s = pow( max( cosphi, 0.0 ), Shininess );
        }
        vec3 specular = Ks * s * SpecularColor;
        gl_FragColor = vec4( ambient + diffuse + specular,  1.0 );
}
