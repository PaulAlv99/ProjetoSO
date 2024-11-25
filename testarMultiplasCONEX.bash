#!/bin/bash
for N in {1..4}
do
    ./cliente ./configs/cliente.conf &
done
wait