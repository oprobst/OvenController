"""Minimal S-expression reader for KiCad netlists (no KiCad dependency)."""

import re


def parse_sexp(text):
    tokens = re.findall(r'\(|\)|"(?:[^"\\]|\\.)*"|[^\s()]+', text)
    stack = [[]]
    for tok in tokens:
        if tok == "(":
            stack.append([])
        elif tok == ")":
            done = stack.pop()
            stack[-1].append(done)
        else:
            stack[-1].append(tok[1:-1] if tok.startswith('"') else tok)
    return stack[0][0]


def children(node, key):
    return [c for c in node[1:] if isinstance(c, list) and c and c[0] == key]


def value(node, key):
    found = children(node, key)
    return found[0][1] if found and len(found[0]) > 1 else None
