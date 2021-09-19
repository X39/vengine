#version 460
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec3 outColor;

layout(set = 0, binding = 0) uniform  CameraBuffer {
    mat4 view;
    mat4 proj;
    mat4 projection_view;
} camera_data;

struct gpu_mesh_data {
    mat4 model;
};

// Object matrices
layout(std140, set = 0, binding = 1) readonly buffer ObjectBuffer {

    gpu_mesh_data mesh_data[];
} mesh_buffer;

void main()
{
    mat4 model_space = mesh_buffer.mesh_data[gl_BaseInstance].model;
    mat4 camera_space = camera_data.projection_view * model_space;
    gl_Position = camera_space * vec4(vPosition, 1.0f);
    outColor = vColor;
}