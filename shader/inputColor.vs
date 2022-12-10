#version 300 es

precision mediump float;
in mediump vec3 position;
in mediump vec3 incolor;
uniform mat4 view;
uniform mat4 zoom;
uniform mat4 shift;
out mediump vec3 t_color;

void main()
{
    gl_Position = zoom * view * shift * vec4(position, 1.0);
    t_color = incolor;
}

