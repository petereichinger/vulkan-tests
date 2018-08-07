#!/bin/bash
echo $(pwd)
$VULKAN_SDK/bin/glslangValidator -V ../../src/shaders/triangle.vert -o vert.spv
$VULKAN_SDK/bin/glslangValidator -V ../../src/shaders/triangle.frag -o frag.spv
