#!/bin/bash
for N in {1..3}
do
    ./cliente ./configs/cliente.conf &
done
wait