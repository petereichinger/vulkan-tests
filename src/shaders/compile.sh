#!/bin/bash
echo $(pwd)
$VULKAN_SDK/bin/glslangValidator -V ../src/shaders/triangle.vert -o shaders/vert.spv
$VULKAN_SDK/bin/glslangValidator -V ../src/shaders/triangle.frag -o shaders/frag.spv
