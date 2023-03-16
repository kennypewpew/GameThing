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
    float r = position.x;
    float theta = position.y;
    float phi = position.z;

    float dTheta = pow(abs(cos(phi)),3.) * 0.5*time;
    float dPhi   = pow(abs(cos(phi)),3.) * 0.5*time;

    float x = center.x + r * sin(theta + dTheta) * cos(phi + dPhi);
    float y = center.y + r * cos(theta + dTheta) * cos(phi + dPhi);
    float z = center.z + r                       * sin(phi + dPhi);

    gl_Position = zoom * view * shift * vec4(x,y,z, 1.0);
    t_color = incolor;
}

