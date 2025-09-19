#version 460 core
    layout (location = 0) in vec3 aPos;
    layout (std430, binding = 1) buffer sbbo1 {
        vec4 positions[];
    };

    uniform mat4 ortho;
    uniform bool isInstanced;
    
    void main()
    {
        if(isInstanced){
            gl_Position = ortho * vec4(positions[gl_InstanceID].xyz, 1.0);
        } else {
            gl_Position = ortho * vec4(aPos, 1.0);
        }
        

    }