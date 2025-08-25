gcc -Iinc -I../inc -Wall -Wextra -std=c11 -g ../src/modbus_master.c ../src/modbus_utils.c modbus_master_sim.c -o master_sim

./master_sim
