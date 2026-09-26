"""Borland name demangling. Expected values are what Borland's own TDUMP prints
for these runtime-library symbols (except where noted), so they document the
scheme as well as test it."""

import pytest

from zbtools.demangle import demangle, qualified_name


@pytest.mark.parametrize(
    ("mangled", "expected"),
    [
        # constructors, destructors, operators, const methods
        ("@xmsg@$bctr$qrx6string", "xmsg::xmsg(const string&)"),
        ("@xmsg@$bdtr$qv", "xmsg::~xmsg()"),
        ("@xmsg@raise$qv", "xmsg::raise()"),
        ("@xmsg@$basg$qrx4xmsg", "xmsg::operator =(const xmsg&)"),
        ("@$bnew$qui", "operator new(unsigned int)"),
        ("@$bdele$qpv", "operator delete(void*)"),
        ("@istream@$brsh$qrzc", "istream::operator >>(char&)"),
        # back-references (t<n>) and unsigned/pointer arguments
        (
            "@_ThrowException$qpvt1t1t1uiuiuipuc",
            "_ThrowException(void*,void*,void*,void*,unsigned int,unsigned int,"
            "unsigned int,unsigned char*)",
        ),
        ("@strstreambuf@$bctr$qpzcit1", "strstreambuf::strstreambuf(char*,int,char*)"),
        # function pointers
        ("@set_unexpected$qpqv$v", "set_unexpected(void(*)())"),
        (
            "@strstreambuf@$bctr$qpql$pvpqpv$v",
            "strstreambuf::strstreambuf(void*(*)(long),void(*)(void*))",
        ),
        ("@ostream@$blsh$qpqr3ios$r3ios", "ostream::operator <<(ios&(*)(ios&))"),
        # nested types (TDUMP drops the argument after one; the real function has it)
        ("@string@strip$q16string@StripTypec", "string::strip(string::StripType,char)"),
        # RTTI type descriptors (TDUMP adds " const" for classes) and vtables
        ("@$xt$4xmsg", "__tpdsc__[xmsg]"),
        ("@$xt$p6string", "__tpdsc__[string*]"),
        ("@constream@3", "vtable for constream"),
        ("@fstreambase@3$vsn", "vtable for fstreambase"),
        # names that aren't C++-mangled are left alone
        ("@__InitExceptBlock", "__InitExceptBlock"),
        ("_strcpy", "_strcpy"),
        ("__DestructorCountPtr", "__DestructorCountPtr"),
    ],
)
def test_demangle(mangled: str, expected: str) -> None:
    assert demangle(mangled) == expected


def test_unparseable_names_are_returned_unchanged() -> None:
    assert demangle("@foo$q?") == "@foo$q?"


@pytest.mark.parametrize(
    ("symbol", "expected"),
    [
        ("@xmsg@raise$qv", "xmsg::raise"),
        ("@xmsg@$bctr$qrx6string", "xmsg::xmsg"),
        ("@string@c_str$xqv", "string::c_str"),
        ("@ios@$bcall$qv", "ios::operator ()"),
        ("@fn_46be2e$ql", "fn_46be2e"),
        ("_strcpy", "strcpy"),
        ("@__InitExceptBlock", "__InitExceptBlock"),
    ],
)
def test_qualified_name(symbol: str, expected: str) -> None:
    assert qualified_name(symbol) == expected
