#version 120

uniform sampler2DRect tex0;
uniform sampler2DRect imageMask;
uniform sampler2DRect bg;

varying vec2 texCoordVarying;

void main()
{
    vec4 color;
    vec4 mask;
    vec4 background;
    color = texture2DRect(tex0, texCoordVarying);
    mask = texture2DRect(imageMask, texCoordVarying);
    background = texture2DRect(bg, texCoordVarying);
    
    gl_FragColor = background*mask.r+color*(1.0-mask.r);
    
}