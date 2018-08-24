#!/bin/bash

mkdir textures 2>/dev/null
mkdir meshes 2>/dev/null

function getTexture {
    if [ ! -f $1 ]; then
        echo "Getting $1 from $2"
        curl -s --output $1 $2
    fi
}

getTexture textures/texture.jpg https://vulkan-tutorial.com/images/texture.jpg
getTexture textures/chalet.jpg https://vulkan-tutorial.com/resources/chalet.jpg

if [ ! -f meshes/chalet.obj ]; then
    echo "Getting meshes/chalet.obj"
    curl -s --output meshes/chalet.obj.zip https://vulkan-tutorial.com/resources/chalet.obj.zip
    cd meshes
    unzip chalet.obj.zip > /dev/null
    rm chalet.obj.zip
fi

if [ ! -f meshes/spot.obj ]; then
    echo "Getting spot"
    rm -rf spot
    curl -s --output spot.zip "http://www.cs.cmu.edu/~kmcrane/Projects/ModelRepository/spot.zip"
    unzip -d spot spot.zip > /dev/null
    cp spot/spot/spot_texture.png textures/spot.png
    cp spot/spot/spot_triangulated.obj meshes/spot.obj
    rm spot.zip
    rm -rf spot
fi
