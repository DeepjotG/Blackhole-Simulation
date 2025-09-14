#version 460 core
    layout (location = 0) in vec3 aPos;

    layout (std430, binding = 1) readonly buffer ssbo {
        mat4 modelMatrices[];
    };

    layout (std430, binding = 2) buffer sbbo1 {
        vec4 positions[];
    };

    uniform mat4 ortho;
    uniform bool isInstanced;
    
    void main()
    {
        if(isInstanced){
            vec4 position = modelMatrices[gl_InstanceID] * vec4(positions[gl_InstanceID].xyz, 1.0);
            gl_Position = ortho * position;
            positions[gl_InstanceID] = position;
        } else {
            gl_Position = ortho * vec4(aPos, 1.0);
        }
        

    }