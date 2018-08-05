#!/bin/bash

echo HELLO
$VULKAN_SDK/bin/glslangValidator -V triangle.vert
$VULKAN_SDK/bin/glslangValidator -V triangle.frag
