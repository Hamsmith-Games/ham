<h1 align="center">
<br/>
<img src="logo.png" width="512" />
<br/>
<br/>
The Ham Development Environment
<br/>
<br/>
</h1>

> **NOTE** This is a work in progress and currently has a tumoltuous development cycle with many things changing and breaking constantly. Use at your own peril.

<br/>

This project is intended as a general purpose environment for developing games and other interactive applications.

<br/>
<br/>
<br/>
<br/>

<h2 align="center">
<a href="runtime">Ham Runtime</a>
<br/>
<br/>
</h2>

> LGPLv3+

The Ham runtime is the backbone of all other components in the project.

- Runtime library (`libham`)

<br/>
<br/>
<br/>
<br/>

<h2 align="center">
<a href="engine">Ham World Engine</a>
<br/>
<br/>
</h2>

> GPLv3+

A creative environment for developing games and immersive experiences.

- Runtime library (`libham-engine`)
- Client (`ham-engine-client`)
- Server (`ham-engine-server` and `ham-engine-server-manager`)
- Editor (`ham-engine-editor`)

<br/>
<br/>
<br/>
<br/>

<h2 align="center">
Ham Language
<br/>
<br/>
</h2>

> GPLv3+

The Ham Language is currently a P.O.C and should not be used.

- Interpreter (`hami`)
- Compiler (`hamc`)

<br/>
<br/>
<br/>
<br/>

<h1 align="center">Building the environment</h1>
<p align="center"><b>As easy as 1, 2, 3... probably.</b></p>

<br/>
<br/>

```bash
mkdir build
cd build
cmake ..
cmake --build . -j $(nproc)
```

<br/>
<br/>
<br/>
<br/>

<h1 align="center">Special thanks</h1>
<p align="center"><b>These projects made Ham possible ❤️</b></p>

<br/>
<br/>

<p align="center"><a href="https://www.gnu.org/software/software.html">GNU Software</a></p>
<p align="center"><a href="https://fmt.dev/">{fmt}</a></p>
<p align="center"><a href="https://www.qt.io/">Qt Project</a></p>
<p align="center"><a href="https://www.libsdl.org/">Simple DirectMedia Layer</a></p>
<p align="center"><a href="https://github.com/ValveSoftware/GameNetworkingSockets">GameNetworkingSockets</a></p>
<p align="center"><a href="https://github.com/ibireme/yyjson">yyjson</a></p>
<p align="center"><a href="https://gmplib.org/">GNU MP</a></p>
<p align="center"><a href="https://www.mpfr.org/">GNU MPFR</a></p>
<p align="center"><a href="https://invisible-island.net/ncurses/">Ncurses</a></p>
<p align="center"><a href="https://github.com/martinus/robin-hood-hashing">robin_hood unordered map & set</a></p>
<p align="center"><a href="https://www.vulkan.org/">Vulkan</a></p>
<p align="center"><a href="https://openal-soft.org/">OpenAL Soft</a></p>

<br/>
<br/>
