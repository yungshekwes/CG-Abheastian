#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the input locations of attributes
layout(location = 0) in vec3 vertCoordinates_in;
layout(location = 1) in vec3 vertColor_in;

// Specify the Uniforms of the vertex shader
// uniform mat4 modelTransform; for example
uniform mat4 modelTransform;
uniform mat4 projectionTransform;

// Specify the output of the vertex stage
out vec3 vertColor;

void main() {
  // gl_Position is the output (a vec4) of the vertex shader
  // Currently without any transformation

  gl_Position = projectionTransform * modelTransform * vec4(vertCoordinates_in, 1.0F);
  vertColor = vertColor_in;
}
