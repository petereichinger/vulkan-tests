#!/bin/bash
mkdir shaders
echo $(pwd)
$VULKAN_SDK/bin/glslangValidator -V ../shaders/triangle.vert -o shaders/vert.spv
$VULKAN_SDK/bin/glslangValidator -V ../shaders/triangle.frag -o shaders/frag.spv
