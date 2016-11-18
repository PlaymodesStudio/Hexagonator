#version 400

#pragma include <of_default_vertex_in_attributes.glsl>
#pragma include <of_default_uniforms.glsl>

out vec4 hexColor;
uniform samplerBuffer tex;

void main()
{
//    int hexagonId = gl_VertexID % (1301*7); // gets hexagon id
    int hexagonId = gl_VertexID % (1301*7); // gets hexagon id
    
    hexColor = vec4( 0.0, 0.0, 0.0, 1.0 );
    hexColor.r = float(hexagonId) / float(1301.0*7); // a red color of 0.0 to 1.0 based on hex id
    hexColor = vec4( hexColor.r, hexColor.r,hexColor.r,hexColor.r);
    
    mat4 transformMatrix = mat4(
                                texelFetch(tex, hexagonId),
                                texelFetch(tex, hexagonId+1),
                                texelFetch(tex, hexagonId+2),
                                texelFetch(tex, hexagonId+3)
                                );

    mat4 m ;
    m[0] = vec4 (1.0f,0.0,0.0,0.0);
    m[1] = vec4 (0.0f,1.0,0.0,0.0);
    m[2] = vec4 (0.0f,0.0,1.0,0.0);
    m[3] = vec4 (0.0f,0.0,0.0,0.5);
    
    gl_Position = modelViewProjectionMatrix * m * position;
}

