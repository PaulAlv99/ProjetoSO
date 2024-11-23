#!/bin/bash
for N in {1..250}
do
    ./cliente ./configs/cliente.conf &
done
wait