# Use Ubuntu as base image
FROM ubuntu:22.04

# Avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    python3 \
    ncurses-bin \
    perl \
    && rm -rf /var/lib/apt/lists/*

# Create working directory
WORKDIR /app

# Create testcases directory structure
RUN mkdir -p testcases/valid testcases/invalid

# Copy all necessary files
COPY giftcardreader.c giftcard.h Makefile ./
COPY genanim.py gengift.py ./
COPY runtests.sh ./
COPY testcases/ ./testcases/

# Make sure the test script is executable
RUN chmod +x runtests.sh

# Build the program with all variants (original, asan, ubsan)
RUN make

# Default command to run tests
CMD ["./runtests.sh"]