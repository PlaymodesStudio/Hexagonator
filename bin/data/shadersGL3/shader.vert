#version 400

#pragma include <of_default_vertex_in_attributes.glsl>
#pragma include <of_default_uniforms.glsl>

uniform samplerBuffer tex;
uniform int u_numHexags;
uniform vec4 u_color;
uniform int u_useMatrix;
flat out vec4 hexColor;

//out vec4 hexColor;

void main()
{
    int numVertsXHexa = 7;
    int numVerts = numVertsXHexa * u_numHexags;
    
    int hexagonId = ((gl_VertexID) / numVertsXHexa);
    int hexagonIdInt = gl_VertexID;
    int faceOnHexa = int(hexagonIdInt)%7;
  
    hexColor = u_color;
    hexColor.r = float(faceOnHexa) / float(6);
    
    //    hexColor.r = float(hexagonId) / float(u_numHexags); // a red color of 0.0 to 1.0 based on hex id
//    hexColor.r = float(hexagonIdInt) / float(21); // a red color of 0.0 to 1.0 based on hex id
    hexColor = vec4( hexColor.r, hexColor.r,hexColor.r,hexColor.r);
    
    
// to check orders and ids
//    if(gl_VertexID%7==0) hexColor = vec4(1.0,0.5,0.0,1.0);
    
    // for ISO cube color !! remember about "FLAT" on color definition (vert + frag) "" 
    if((faceOnHexa==6)||(faceOnHexa==5))
    {
      hexColor = vec4(0.0,0.3,0.7,1.0);
      
    }
    else if((faceOnHexa==1)||(faceOnHexa==2))
    {
      hexColor = vec4(0.0,0.1,0.3,1.0);
      
    }
    else if((faceOnHexa==3)||(faceOnHexa==4))
    {
      hexColor = vec4(0.0,0.9,0.9,1.0);
      
    }
    
    
    
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

