gcc -Iinc -I../inc -Wall -Wextra -std=c11 -g ../src/modbus_slave.c ../src/modbus_utils.c modbus_slave_sim.c -o slave_sim

./slave_sim