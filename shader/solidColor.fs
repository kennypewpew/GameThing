#version 300 es

precision mediump float;
out mediump vec4 fragColor;
uniform vec4 incolor;

void main()
{
    fragColor = incolor;
}
