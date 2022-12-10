#version 300 es

precision mediump float;
in mediump vec2 tCoord;
out mediump vec4 fragColor;
uniform sampler2D applyTexture;

void main()
{
    fragColor = texture(applyTexture,tCoord);
}

