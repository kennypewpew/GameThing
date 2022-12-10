#version 300 es

precision mediump float;
in mediump vec2 tCoord;
out mediump vec4 fragColor;
uniform sampler2D applyTexture;
uniform mediump vec3 texColor;

void main()
{
    float textureAlpha = texture(applyTexture,tCoord).a;
    fragColor = textureAlpha * vec4(texColor,1);
}

