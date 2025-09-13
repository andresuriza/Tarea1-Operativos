FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential pkg-config iputils-ping netcat-traditional curl \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copia respetando la nueva estructura
COPY include/ include/
COPY src/ src/
COPY Makefile ./

# Fuerza recompilación limpia y verbo para ver qué linkea
RUN make clean && make -B client

# Arranque (igual que antes)
ENTRYPOINT ["./client", "--ip", "host.docker.internal"]
