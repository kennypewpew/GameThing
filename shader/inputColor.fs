#version 300 es

precision mediump float;
out mediump vec4 fragColor;
in mediump vec3 t_color;

void main()
{
    fragColor = vec4(t_color, 1.0);
}

