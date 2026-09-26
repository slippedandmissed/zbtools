"""Demangle Borland C++ 4.x symbol names, e.g.

    @xmsg@$bctr$qrx6string  ->  xmsg::xmsg(const string&)

Hand-written because no Python library knows Borland's scheme (PyPI's
demanglers handle GCC/Clang's Itanium scheme only), and Ghidra has no Borland
demangler either. Checked against the names Borland's own TDUMP prints for the
runtime library. Names it can't parse are returned unchanged.
"""

# Special function names: $b<code>.
_OPERATORS = {
    "new": "operator new", "dele": "operator delete",
    "nwa": "operator new[]", "dla": "operator delete[]",
    "asg": "operator =", "call": "operator ()", "subs": "operator []",
    "arow": "operator ->", "arwm": "operator ->*",
    "add": "operator +", "sub": "operator -", "mul": "operator *", "div": "operator /",
    "mod": "operator %", "inc": "operator ++", "dec": "operator --",
    "and": "operator &", "or": "operator |", "xor": "operator ^", "not": "operator ~",
    "lsh": "operator <<", "rsh": "operator >>",
    "rplu": "operator +=", "rmin": "operator -=", "rmul": "operator *=", "rdiv": "operator /=",
    "rmod": "operator %=", "rand": "operator &=", "ror": "operator |=", "rxor": "operator ^=",
    "rlsh": "operator <<=", "rrsh": "operator >>=",
    "eql": "operator ==", "neq": "operator !=", "lss": "operator <", "gtr": "operator >",
    "leq": "operator <=", "geq": "operator >=",
    "lnot": "operator !", "land": "operator &&", "lor": "operator ||",
    "ind": "operator *", "adr": "operator &", "coma": "operator ,",
}  # fmt: skip

_BUILTINS = {
    "v": "void", "c": "char", "s": "short", "i": "int", "l": "long", "j": "__int64",
    "f": "float", "d": "double", "g": "long double", "o": "bool", "b": "wchar_t",
    "e": "...",
}  # fmt: skip


class _Error(ValueError):
    pass


class _Parser:
    def __init__(self, text: str) -> None:
        self.text, self.pos = text, 0
        self.args: list[str] = []  # for back-references (t<n>)

    def peek(self, n: int = 1) -> str:
        return self.text[self.pos : self.pos + n]

    def take(self) -> str:
        if self.pos >= len(self.text):
            raise _Error("unexpected end")
        self.pos += 1
        return self.text[self.pos - 1]

    def number(self) -> int:
        start = self.pos
        while self.peek().isdigit():
            self.pos += 1
        if start == self.pos:
            raise _Error("expected a number")
        return int(self.text[start : self.pos])

    def class_name(self) -> str:
        """A length-prefixed name; nested names are joined with @."""
        length = self.number()
        name = self.text[self.pos : self.pos + length]
        if len(name) != length:
            raise _Error("truncated name")
        self.pos += length
        return name.replace("@", "::")

    def modifiers(self) -> tuple[bool, bool, str]:
        """Leading const (x), volatile (w) and unsigned (u) markers. z (signed)
        is dropped: Borland's own demangler prints "zc" as plain char."""
        const = volatile = False
        sign = ""
        while self.peek() and self.peek() in "xwuz":
            code = self.take()
            const |= code == "x"
            volatile |= code == "w"
            if code == "u":
                sign = "unsigned "
        return const, volatile, sign

    def type(self) -> str:
        const, volatile, sign = self.modifiers()
        code = self.take()
        if code == "p" and self.peek() == "q":  # pointer to function
            self.pos += 1
            return self.function_pointer()
        if code in ("p", "r"):
            text = self.type() + ("*" if code == "p" else "&")
            qualifiers = (" const" if const else "") + (" volatile" if volatile else "")
            return text + qualifiers
        if code in _BUILTINS:
            text = sign + _BUILTINS[code]
        elif code.isdigit():
            self.pos -= 1
            text = self.class_name()
        elif code == "t":
            ref = self.take()
            index = int(ref) if ref.isdigit() else ord(ref) - ord("a") + 10
            if not 1 <= index <= len(self.args):
                raise _Error("bad back-reference")
            return self.args[index - 1]
        elif code == "a":
            size = self.number()
            if self.take() != "$":
                raise _Error("bad array")
            return f"{self.type()}[{size}]"
        elif code == "q":
            return self.function_pointer()
        else:
            raise _Error(f"unknown type code {code!r}")
        prefix = ("const " if const else "") + ("volatile " if volatile else "")
        return prefix + text

    def function_pointer(self) -> str:
        """After `q`: parameter types, `$`, then the return type."""
        params = self.arguments(until="$")
        self.take()  # $
        return f"{self.type()}(*)({params})"

    def arguments(self, until: str = "") -> str:
        types = []
        while self.pos < len(self.text) and self.peek() != until:
            arg = self.type()
            types.append(arg)
            self.args.append(arg)
        return "" if types == ["void"] else ",".join(types)

    def qualified_name(self) -> tuple[list[str], str]:
        """Enclosing classes, and the (possibly special) final name."""
        scopes: list[str] = []
        while True:
            if self.peek(2) == "$b":
                self.pos += 2
                start = self.pos
                while self.peek() and self.peek() != "$":
                    self.pos += 1
                code = self.text[start : self.pos]
                if code == "ctr":
                    return scopes, scopes[-1] if scopes else "constructor"
                if code == "dtr":
                    return scopes, "~" + (scopes[-1] if scopes else "destructor")
                return scopes, _OPERATORS.get(code, f"operator {code}")
            if self.peek(2) == "$o":
                self.pos += 2
                return scopes, f"operator {self.type()}"
            start = self.pos
            while self.peek() and self.peek() not in ("@", "$"):
                self.pos += 1
            name = self.text[start : self.pos]
            if self.peek() == "@":
                self.pos += 1
                scopes.append(name)
                continue
            return scopes, name


def demangle(symbol: str) -> str:
    """The C++ declaration a Borland symbol names, or the symbol unchanged if it
    isn't mangled (plain C names such as `_strcpy`) or can't be parsed."""
    if not symbol.startswith("@"):
        return symbol
    parser = _Parser(symbol[1:])
    try:
        if parser.peek(4) == "$xt$":  # RTTI type descriptor
            parser.pos += 4
            return f"__tpdsc__[{parser.type()}]"
        scopes, name = parser.qualified_name()
        if name == "3" and scopes:  # @Class@3, sometimes with a suffix such as $vsn
            return f"vtable for {'::'.join(scopes)}"
        qualified = "::".join([*scopes, name])
        if not parser.peek():
            return qualified  # data, e.g. a static member
        parser.take()  # $
        const_method = False
        while parser.peek() in ("x", "w") and parser.peek():
            const_method |= parser.take() == "x"
        if parser.take() != "q":
            raise _Error("expected argument list")
        if parser.peek() == "q":  # calling convention, e.g. qs (__stdcall)
            parser.pos += 2
        params = parser.arguments()
        return f"{qualified}({params})" + (" const" if const_method else "")
    except (_Error, IndexError):
        return symbol


def qualified_name(symbol: str) -> str:
    """The qualified name a symbol declares, without its argument list: e.g.
    "xmsg::raise" for @xmsg@raise$qv, or "strcpy" for the C name _strcpy."""
    text = demangle(symbol)
    if text == symbol:  # C: a leading underscore (cdecl) or @ (fastcall)
        return symbol.removeprefix("_").removeprefix("@")
    body = text.removesuffix(" const")
    if body.endswith(")"):  # cut the argument list at its matching "("
        depth = 0
        for i in range(len(body) - 1, -1, -1):
            depth += {")": 1, "(": -1}.get(body[i], 0)
            if depth == 0:
                return body[:i]
    return text
