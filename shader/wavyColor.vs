#version 300 es

precision mediump float;
in mediump vec3 position;
in mediump vec3 incolor;
uniform mat4 view;
uniform mat4 zoom;
uniform mat4 shift;
uniform float time;
out mediump vec3 t_color;

out vec3 Normal;
out vec3 myPos;

void main()
{
    float x = position.x;
    float y = position.y;
    float dx = 10.*(x-0.);
    float dy = 10.*(y-0.);
    float dist = sqrt( dx*dx + dy*dy );
    float z = 0.;
    float wavespeed = 1.3;
    float transition = 0.5;

    if ( dist < time*wavespeed ) {
      z += 0.3/sqrt(time) * sin(dist + time);
    }
    else if ( dist < (time*wavespeed + transition) ) {
      float waveDist = dist - time*wavespeed;
      z += (1.-waveDist/transition) * 0.3/sqrt(time) * sin(time*wavespeed + time);
    }
    else {
      z += 0.;
    }

    //// No explicit ifs, less legible, same FPS. Not worth it
    // float distFromWaveFront = dist - time*wavespeed;
    // float waveDist = dist - time*wavespeed;
    // float sineTerm = distFromWaveFront < 0. ? dist : time*wavespeed;
    // float multTerm = max(0.,min(1.-waveDist/transition,1.));
    // z = 0.3/sqrt(time) * sin(sineTerm + time) * multTerm;


    gl_Position = zoom * view * shift * vec4(x,y,z, 1.3);
    t_color = incolor;
    myPos = vec3(x,y,z);

    float x1 = x - 0.01;
    float y1 = y - 0.01;
    float x2 = x + 0.01;
    float y2 = y - 0.01;
    float dx1 = 10.*(x1-0.);
    float dy1 = 10.*(y1-0.);
    float dx2 = 10.*(x2-0.);
    float dy2 = 10.*(y2-0.);
    float dist1 = sqrt( dx1*dx1 + dy1*dy1 );
    float dist2 = sqrt( dx2*dx2 + dy2*dy2 );
    float z1;
    float z2;

    if ( dist1 < time*wavespeed ) {
      z1 = 0.3/sqrt(time) * sin(dist1 + time);
    }
    else if ( dist1 < (time*wavespeed + transition) ) {
      float waveDist = dist1 - time*wavespeed;
      z1 = (1.-waveDist/transition) * 0.3/sqrt(time) * sin(time*wavespeed + time);
    }
    else {
      z1 = 0.;
    }

    if ( dist2 < time*wavespeed ) {
      z2 = 0.3/sqrt(time) * sin(dist2 + time);
    }
    else if ( dist2 < (time*wavespeed + transition) ) {
      float waveDist = dist2 - time*wavespeed;
      z2 = (1.-waveDist/transition) * 0.3/sqrt(time) * sin(time*wavespeed + time);
    }
    else {
      z2 = 0.;
    }

    //distFromWaveFront = dist1 - time*wavespeed;
    //waveDist = dist1 - time*wavespeed;
    //sineTerm = distFromWaveFront < 0. ? dist1 : time*wavespeed;
    //multTerm = max(0.,min(1.-waveDist/transition,1.));
    //z1 = 0.3/sqrt(time) * sin(sineTerm + time) * multTerm;
    //
    //distFromWaveFront = dist2 - time*wavespeed;
    //waveDist = dist2 - time*wavespeed;
    //sineTerm = distFromWaveFront < 0. ? dist2 : time*wavespeed;
    //multTerm = max(0.,min(1.-waveDist/transition,1.));
    //z2 = 0.3/sqrt(time) * sin(sineTerm + time) * multTerm;


    vec3 v1 = vec3(x1,y1,z1) - vec3(x,y,z);
    vec3 v2 = vec3(x2,y2,z2) - vec3(x,y,z);

    Normal = cross(v1,v2);
}

