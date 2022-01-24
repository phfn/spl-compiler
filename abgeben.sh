#!/bin/bash
docker build -f docker/Dockerfile -t spl-compiler .
docker run -it spl-compiler
