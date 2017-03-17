#version 410

#pragma include <of_default_vertex_in_attributes.glsl>
#pragma include <of_default_uniforms.glsl>

//layout(location = 1) in vec4 vertex_color;

uniform samplerBuffer texTransform;
uniform samplerBuffer texCubeColors;
uniform int u_numHexags;
uniform int u_modulo;
uniform vec4 u_colorA;
uniform vec4 u_colorB;
uniform int u_useMatrix;

flat out vec4 hexColor;
uniform int u_textureSource;
uniform int u_source;

out vec2        out_vTexCoord;
out vec4        out_colorMix;
out vec4        out_vertexColor;
out vec4        out_colorA;

//enum sourceTextureType
//{
//    HEX_TEXTURE_IMAGE = 0,
//    HEX_TEXTURE_VIDEO = 1,
//    HEX_TEXTURE_SYPHON = 2,
//    HEX_TEXTURE_SYPHON_MAX = 3,
//    HEX_TEXTURE_CUBES = 4,
//    HEX_TEXTURE_COLOR = 5
//};
//enum sourceType
//{
//    HEX_SOURCE_TEXTURE = 0,
//    HEX_SOURCE_QUADS = 1,
//    HEX_SOURCE_CUCS = 2,
//    HEX_SOURCE_RANDOM = 3
//};



void main()
{
    // outputs texture coordinates to fragmnent shader
    out_vTexCoord = texcoord;
    // calculate hexagon/element id based on which vertex and which modulo
    // this is important to calculate the transformation to element centroid in the transformation matrix
    int hexagonId = ((gl_VertexID) / u_modulo);
    
    if(u_textureSource==4)
    {
        int numVerts = u_modulo * u_numHexags;
        int faceOnHexa = int(gl_VertexID)%7;
              
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
    else if(u_textureSource==5)
    {
        if((gl_VertexID%u_modulo)==0)
        {
            out_colorMix = u_colorA;
    
        }
        else
        {
            out_colorMix = u_colorB;
        }
        
    }
    if(u_source==2)
    {
        if((gl_VertexID%u_modulo)<(u_modulo/8))
        {
            out_colorMix = u_colorA;
        }
        else
        {
            out_colorMix = u_colorB;
        }
        
    }
    out_colorA = u_colorA;
    //out_colorA = color;
    
    // we output to the fragment shader the vertex color taken from vbo buffers
    out_vertexColor = color;

    
    //hexColor.r = float(faceOnHexa) / float(6);
    //    hexColor.r = float(hexagonId) / float(u_numHexags); // a red color of 0.0 to 1.0 based on hex id
    //    hexColor.r = float(gl_VertexID) / float(21); // a red color of 0.0 to 1.0 based on hex id
    //hexColor = vec4( hexColor.r, hexColor.r,hexColor.r,hexColor.r);
    
    // make another buffer to be passed to the shader
    // with colors for the cubic sides
    // 1 color (vec4) for each side (3) of each hexagon (N)
    
    
    // to check orders and ids
    //    if(gl_VertexID%7==0) hexColor = vec4(1.0,0.5,0.0,1.0);
    //if(hexagonId==1088) hexColor = vec4(1.0,0.0,0.0,1.0);
    
    
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

