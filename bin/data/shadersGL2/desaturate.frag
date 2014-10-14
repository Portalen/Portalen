#version 120

uniform sampler2DRect tex0;

varying vec2 texCoordVarying;

void main()
{
    vec4 color;
    color = texture2DRect(tex0, texCoordVarying);
    
    gl_FragColor = vec4( mix( vec3( dot( color.rgb, vec3( 0.2125, 0.7154, 0.0721 ) ) ),
                             color.rgb, 0.2 ), color.a );
    
}