<h1 align="center">
<br/>
<img src="engine/editor/images/logo.png" width="512" />
<br/>
<br/>
The Hamsmith Development Environment
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
Ham Runtime
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
Ham Language
<br/>
<br/>
</h2>

> GPLv3+

The Ham Language is currently a POC and should not be used.

- Interpreter (`hami`)
- Compiler (`hamc`)

<br/>
<br/>
<br/>
<br/>

<h2 align="center">
Ham World Engine
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

<h1 align="center">Building the environment</h1>

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
