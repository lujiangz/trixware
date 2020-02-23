#!/usr/bin/python3

from pathlib import Path
import random
import string

junk_head = """
/* junk */
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

"""

def gen_string(minlen=1, maxlen=255):
    return "".join(random.choices(string.ascii_letters, k=random.randint(minlen, maxlen)))

class cpp_var:
    def __init__(self):
        self.name = gen_string(5, 16)
        self.type = "void"

    def __str__(self):
        return str(self.value)

    @property
    def declaration(self):
        return f"{self.type} {self.name}"

    @property
    def definition(self):
        return f"{self.type} {self.name} = {self}"

    def gen_comparison(self, other):
        return f"{self.name} {random.choice(self.cmp_ops)} {other}"

    def gen_manip(self, other):
        return f"{self.name} {random.choice(self.manip_ops)} {other.name}"

class cpp_string(cpp_var):
    def __init__(self):
        super().__init__()
        self.value = gen_string()
        self.type = "string"
        self.cmp_ops = ['==', '!=', '<', '<=', '>', '>=']
        self.manip_ops = ['=', '+=']

    def __str__(self):
        return f'string("{self.value}")'

class cpp_bool(cpp_var):
    def __init__(self):
        super().__init__()
        self.value = random.choice(["true", "false"])
        self.type = "bool"
        self.cmp_ops = ["==", "!="]
        self.manip_ops = ["=", "= !"]

class cpp_int(cpp_var):
    def __init__(self, minval=-2**31, maxval=2**31-1):
        super().__init__()
        self.value = random.randint(minval, maxval)
        self.type = "int"
        self.cmp_ops = ['==', '!=', '<', '<=', '>', '>=']
        self.manip_ops = ['=', '+=', '-=', '*=', '/=']

class cpp_double(cpp_int):
    def __init__(self):
        super().__init__(-2**20, 2**20)
        self.type = "double"
        self.value += random.random()

cpp_types = [cpp_string, cpp_bool, cpp_int, cpp_double]

def gen_for(varlist):
    counter = cpp_int(1)
    stmts = [a.gen_manip(b) for a, b in varlist if type(a) == type(b)]
    if len(stmts) > 0:
        stmts = "\n" + ";\n".join(stmts) + ";\n"
    else:
        stmts = "\ncontinue;\n"

    return f"\nfor ({counter.definition}; {counter.name} > 0; {counter.name}--) {{{stmts}}}"

def gen_condition(a, b, code):
    return f"\nif ({a.gen_comparison(b)}) {{\n{code.strip()}\n}}"

def randompairs(it, minpairs=1, maxpairs=10):
    numpairs = random.randint(minpairs, maxpairs)
    return zip(random.choices(it, k=numpairs), random.choices(it, k=numpairs))

class cpp_func:
    def __init__(self):
        self.return_type = random.choice(cpp_types + [cpp_var])()
        self.args = [v() for v in random.choices(cpp_types, k=random.randint(0, 5))]
        self.local =[v() for v in random.choices(cpp_types, k=random.randint(1, 10))]
        self.variables = self.args + self.local

    @property
    def declaration(self):
        return f"{self.return_type.declaration}({', '.join([arg.declaration for arg in self.args])})"

    @property
    def call(self):
        return f"this->{self.return_type.name}({', '.join([str(arg) for arg in self.args])})"

    def generate(self, classname):
        s = [v.definition + ";" for v in self.local]

        for a, b in randompairs(self.variables):
            if random.choice([True, False]):
                code = gen_for(randompairs(self.variables))
            else:
                code = ""

            if code != "":
                if a.type == b.type:
                    s.append(gen_condition(a, b, code))
                else:
                    s.append(code)

        if self.return_type.type != "void":
            ret = f"\nreturn {self.return_type};"

            for v in self.variables:
                if v.type == self.return_type.type:
                    ret = f"\nreturn {v.name};"

            s.append(ret)

        code = "\n".join(s)

        return f"{self.return_type.type} {classname}::{self.return_type.name}({', '.join([arg.declaration for arg in self.args])})\n{{\n{code}\n}}"

def genclass():
    name = gen_string(5, 16)
    s = []
    funcs = []

    for section in ["public", "protected", "private"]:
        s.append(section + ":")

        s.extend([v().declaration + ";" for v in random.choices(cpp_types, k=random.randint(1, 6))])

        s.append("")

        if section == "public":
            s.append(f"{name}();")

        for i in range(random.randint(0, 8)):
            f = cpp_func()
            funcs.append(f)
            s.append(f.declaration + ";")

    s = "\n".join(s)
    decl = f"class {name}\n{{\n{s}\n}};\n\n"

    definition = []

    for func in funcs:
        definition.append(func.generate(name))

    calls = "\n".join([f.call + ";" for f in funcs])
    definition.append(f"{name}::{name}()\n{{\n{calls}\n}}")

    cls = []
    indent = 0

    for line in (decl +  "\n\n".join(definition)).split("\n"):
        if "}" in line:
            indent -= 1

        if not ":" in line:
            line = indent * 4 * " " + line

        if "{" in line:
            indent += 1

        cls.append(line.rstrip())

    return "\n".join(cls)

for p in Path(".").glob("**/*.cpp"):
    with open(p, "a") as f:
        print(junk_head + genclass(), file=f)
