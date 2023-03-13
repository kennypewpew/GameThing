#version 300 es

precision mediump float;
in mediump vec3 position;
in mediump vec2 texCoord;
in mediump vec3 texColor;
out mediump vec2 tCoord;
out mediump vec3 tColor;
uniform mat4 zoom;
uniform mat4 view;
uniform mat4 shift;

void main()
{
    gl_Position = zoom * view * shift * vec4(position, 1.0);
    tCoord = texCoord;
    tColor = texColor;
}

