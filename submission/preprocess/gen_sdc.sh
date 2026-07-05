#!/bin/bash
# Generate complete GCD SDC with input_transition and set_load
cd /mnt/f/reasonix/constraints

{
  cat gcd.sdc
  echo ''
  echo '# === input transitions ==='
  # req_msg[31:0]
  for i in $(seq 0 31); do
    echo "set_input_transition 10 -min -rise [get_ports req_msg[$i]] -clock clk"
    echo "set_input_transition 10 -min -fall [get_ports req_msg[$i]] -clock clk"
    echo "set_input_transition 20 -max -rise [get_ports req_msg[$i]] -clock clk"
    echo "set_input_transition 20 -max -fall [get_ports req_msg[$i]] -clock clk"
  done
  # other inputs
  for p in clk reset req_val resp_rdy; do
    echo "set_input_transition 10 -min -rise [get_ports $p] -clock clk"
    echo "set_input_transition 10 -min -fall [get_ports $p] -clock clk"
    echo "set_input_transition 20 -max -rise [get_ports $p] -clock clk"
    echo "set_input_transition 20 -max -fall [get_ports $p] -clock clk"
  done
  echo ''
  echo '# === output loads ==='
  # resp_msg[15:0]
  for i in $(seq 0 15); do
    echo "set_load -pin_load 4 [get_ports resp_msg[$i]]"
  done
  for p in req_rdy resp_val; do
    echo "set_load -pin_load 4 [get_ports $p]"
  done
} > gcd_complete.sdc

echo "Lines: $(wc -l < gcd_complete.sdc)"
echo "--- tail ---"
tail -10 gcd_complete.sdc
