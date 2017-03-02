#version 400

#pragma include <of_default_vertex_in_attributes.glsl>
#pragma include <of_default_uniforms.glsl>

out vec4 hexColor;
uniform samplerBuffer tex;
uniform int u_numHexags;
uniform vec4 u_color;
uniform int u_useMatrix;

void main()
{
    int numVertsXHexa = 7;
    int numVerts = numVertsXHexa * u_numHexags;
    
    int hexagonId = ((gl_VertexID) / numVertsXHexa);
  
    hexColor = u_color;
    //hexColor.r = float(hexagonId) / float(u_numHexags); // a red color of 0.0 to 1.0 based on hex id
    //hexColor = vec4( hexColor.r, hexColor.r,hexColor.r,hexColor.r);
    
    int tx = hexagonId*4;

    
    if(u_useMatrix==1)
    {
        mat4 transformMatrix = mat4(
                                    texelFetch(tex, tx),
                                    texelFetch(tex, tx+1),
                                    texelFetch(tex, tx+2),
                                    texelFetch(tex, tx+3)
                                    );

        gl_Position = modelViewProjectionMatrix * transformMatrix * position;
    }
    else
    {
        gl_Position = modelViewProjectionMatrix * position;
    }
    
}

