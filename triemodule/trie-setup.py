from distutils.core import setup, Extension

setup(name="trie", version="1.0",
      ext_modules=[Extension("trie", ["triemodule.c"])])

