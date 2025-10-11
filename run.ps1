podman machine init
podman machine start
podman load -i ./simple-opengl.tar.gz
# Replace this with actual IP!
podman run -it --rm -e DISPLAY=172.x.x.x:0.0 --device=/dev/dri -w /share simple-opengl:latest nixGLIntel simple-opengl
