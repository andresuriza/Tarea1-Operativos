#!/bin/bash
docker build -t cliente-ubuntu .
docker run --rm \
  --add-host=host.docker.internal:host-gateway \
  -v $(pwd)/imagenes:/data \
  cliente-ubuntu --port 1717
