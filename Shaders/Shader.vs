#version 460 core
    layout (location = 0) in vec3 aPos;

    uniform mat4 model;
    uniform mat4 ortho;


    void main()
    {
        gl_Position = ortho * model * vec4(aPos, 1.0);

    }