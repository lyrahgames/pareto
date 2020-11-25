cxx.std = latest
using cxx

hxx{*}: extension = hpp
cxx{*}: extension = cpp

exe{main}: {hxx cxx}{**}

cxx.poptions =+ "-I$src_base"