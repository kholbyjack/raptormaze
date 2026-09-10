import os

with open("/proc/amazeraptor", "r") as proc_raptor:
    raptor_maze = proc_raptor.read()

print(raptor_maze)
