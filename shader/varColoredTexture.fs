#version 300 es

precision mediump float;
in mediump vec2 tCoord;
in mediump vec3 tColor;
out mediump vec4 fragColor;
uniform sampler2D applyTexture;

void main()
{
    float textureAlpha = texture(applyTexture,tCoord).a;
    fragColor = textureAlpha * vec4(tColor,1);
}

