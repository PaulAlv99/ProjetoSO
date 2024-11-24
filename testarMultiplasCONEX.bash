#!/bin/bash
for N in {1..2}
do
    ./cliente ./configs/cliente.conf &
done
wait