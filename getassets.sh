rm -rf textures 2>/dev/null
rm -rf meshes 2>/dev/null

mkdir textures
mkdir meshes

wget -O textures/texture.jpg https://vulkan-tutorial.com/images/texture.jpg
wget -O textures/chalet.jpg https://vulkan-tutorial.com/resources/chalet.jpg


wget -O meshes/chalet.obj.zip https://vulkan-tutorial.com/resources/chalet.obj.zip
cd meshes
unzip chalet.obj.zip
rm chalet.obj.zip
