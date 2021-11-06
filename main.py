import sys
from readchar import readchar # pip install readchar

if len(sys.argv) < 2:
    print(f'Usage: {sys.argv[1]} <file.bf>')
    sys.exit(1)


filename = sys.argv[1]
code = open(filename).read()
stack = []
jumps = {}
for i, c in enumerate(code):
    if c == '[':
        stack.append(i)
    elif c == ']':
        j = stack.pop()
        jumps[i] = j
        jumps[j] = i


pc = 0
mem = [0] * 30_000
ptr = 0
count = 0
print(len(code))
while pc < len(code):
    count += 1
    op = code[pc]
    if   op == '>': ptr += 1
    elif op == '<': ptr -= 1
    elif op == '+': mem[ptr] = (mem[ptr] + 1) % 255
    elif op == '-': mem[ptr] = (mem[ptr] - 1) % 255
    elif op == '.': print(chr(mem[ptr]), end='', flush=True)
    elif op == ',': mem[ptr] = readchar()
    elif op == '[' and mem[ptr] == 0: pc = jumps[pc]
    elif op == ']' and mem[ptr] != 0: pc = jumps[pc]

    pc += 1


print(f'Executed {count} instructions')
