#!/bin/bash

mkdir textures 2>/dev/null
mkdir meshes 2>/dev/null

function getTexture {
    #if [ ! -f $1 ]; then
        echo "Getting $1 from $2"
        wget -4 -O $1 $2 > /dev/null
    #fi
}

getTexture textures/texture.jpg https://vulkan-tutorial.com/images/texture.jpg
getTexture textures/chalet.jpg https://vulkan-tutorial.com/resources/chalet.jpg

if [ ! -f meshes/chalet.obj ]; then

    echo "Getting chalet.obj"
    wget -4 -O meshes/chalet.obj.zip https://vulkan-tutorial.com/resources/chalet.obj.zip > /dev/null
    cd meshes
    unzip chalet.obj.zip > /dev/null
    rm chalet.obj.zip
fi
