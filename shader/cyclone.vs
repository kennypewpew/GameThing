#version 300 es

precision mediump float;
in mediump vec3 position;
in mediump vec3 incolor;
uniform mat4 view;
uniform mat4 zoom;
uniform mat4 shift;
uniform float time;
uniform vec3 center;
out mediump vec3 t_color;

void main()
{
    float z = center.z + position.z;

    float r = position.x;
    float theta = position.y;

    float x = center.x + r * sin(theta + r*time);
    float y = center.y + r * cos(theta + r*time);
    gl_Position = zoom * view * shift * vec4(x,y,z, 1.3);
    t_color = incolor;
}

