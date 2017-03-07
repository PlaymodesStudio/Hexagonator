#version 410

#pragma include <of_default_vertex_in_attributes.glsl>
#pragma include <of_default_uniforms.glsl>

uniform samplerBuffer texTransform;
uniform samplerBuffer texCubeColors;
uniform int u_numHexags;
uniform vec4 u_color;
uniform int u_useMatrix;
uniform int u_useCubeColors;
flat out vec4 hexColor;

//in vec2         texcoord;
// outputs
out vec2        vTexCoord;
//out vec4 hexColor;

void main()
{
    // outputs
    vTexCoord = texcoord;
    

    
    int numVertsXHexa = 7;
    int numVerts = numVertsXHexa * u_numHexags;
    
    int hexagonId = ((gl_VertexID) / numVertsXHexa);
    int faceOnHexa = int(gl_VertexID)%7;
  
    hexColor = u_color;
    
    //hexColor.r = float(faceOnHexa) / float(6);
    //    hexColor.r = float(hexagonId) / float(u_numHexags); // a red color of 0.0 to 1.0 based on hex id
    //    hexColor.r = float(gl_VertexID) / float(21); // a red color of 0.0 to 1.0 based on hex id
    //hexColor = vec4( hexColor.r, hexColor.r,hexColor.r,hexColor.r);
    
    // make another buffer to be passed to the shader
    // with colors for the cubic sides
    // 1 color (vec4) for each side (3) of each hexagon (N)
    
    
    if(u_useCubeColors==1)
    {
        // COLOR for ISO cube color !! remember about "FLAT" on color definition (vert + frag) ""
        if((faceOnHexa==1)||(faceOnHexa==2))
        {
            // 1 :: lowest side
            hexColor = vec4(1.0,0.0,0.0,1.0);
            hexColor = texelFetch(texCubeColors, (hexagonId*3)+0);
        }
        else if((faceOnHexa==6)||(faceOnHexa==5))
        {
            // 2 :: left side
            hexColor = vec4(0.0,1.0,0.0,1.0);
            hexColor = texelFetch(texCubeColors, (hexagonId*3)+1);
        }
        else if((faceOnHexa==3)||(faceOnHexa==4))
        {
            // 3 :: right side
            hexColor = vec4(0.0,0.0,1.0,1.0);
            hexColor = texelFetch(texCubeColors, (hexagonId*3)+2);
        }
    }
    // to check orders and ids
    //    if(gl_VertexID%7==0) hexColor = vec4(1.0,0.5,0.0,1.0);
    //if(hexagonId==1088) hexColor = vec4(1.0,0.0,0.0,1.0);
    
    
    
    // TRANSFORMATION
    // use transform matrix
//    static const string VIEW_MATRIX_UNIFORM="viewMatrix";
//    static const string MODELVIEW_MATRIX_UNIFORM="modelViewMatrix";
//    static const string PROJECTION_MATRIX_UNIFORM="projectionMatrix";
//    static const string MODELVIEW_PROJECTION_MATRIX_UNIFORM="modelViewProjectionMatrix";
//    static const string TEXTURE_MATRIX_UNIFORM="textureMatrix";
//    static const string COLOR_UNIFORM="globalColor";

    
    if(u_useMatrix==1)
    {
        // use transform matrix that comes in the form of a "texture" : samplerBuffer tex;
        // fetch the mat4x4 = 4 x vec4 and reconstruct the matrix to be multiplied each vertex
        int tx = hexagonId*4;
        mat4 transformMatrix = mat4(
                                    texelFetch(texTransform, tx),
                                    texelFetch(texTransform, tx+1),
                                    texelFetch(texTransform, tx+2),
                                    texelFetch(texTransform, tx+3)
                                    );

        gl_Position = modelViewProjectionMatrix * transformMatrix * position;
    }
    else
    {
        // TODO : Doesn't work !! 
        gl_Position = modelViewProjectionMatrix * position;
    }
    
}

