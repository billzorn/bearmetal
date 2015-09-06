# Simple dictionary-based string tries. Can hold anything as values.
# Supports converstion to a C source code string that performs
# lookup literally and executes the values, as long as all the values
# are C source expression strings.

_end = '\x00'

# expects keys in d_source to be strings
def mktrie(d_source):
    root = {}
    for key in d_source:
        d = root
        for k in key:
            d = d.setdefault(k, {})
        d[_end] = d_source[key]
    return root

# for testing
def intrie(trie, key):
    d = trie
    for k in key:
        if k in d:
            d = d[k]
        else:
            return None
    if _end in d:
        return d[_end]
    else:
        return None

# recursive, with default parameters works from the top of a trie
def trie_to_c_src(trie, default, idx = 0, ident = 0, tab = 1):
    # const char *key; // string to look up
    # size_t len;      // length of key, including nul char on the end
    s = ''
    d = trie
    
    s += ' '*ident + 'if (' + str(idx) + 'ul >= len) {' + default + '}\n'
    s += ' '*ident + 'switch (key[' + str(idx) + 'ul]) {\n'
    ident += tab

    for k in d:
        s += ' '*ident + 'case ' + str(ord(k)) + ':\n'
        ident += tab
        if k == _end:
            s += ' '*ident + '{' + d[k] + '}\n'
        else:
            s += trie_to_c_src(d[k], default, idx=idx+1, ident=ident, tab=tab)
        s += ' '*ident + 'break;\n'
        ident -= tab
    
    s += ' '*ident + 'default:\n'
    ident += tab
    s += ' '*ident + '{' + default + '}\n'
    ident -= tab

    ident -= tab
    s += ' '*ident + '}\n'
    return s
