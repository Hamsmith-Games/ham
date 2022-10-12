# TODO

List of stuff that needs to be done. Should probably be GitHub issues ¯\\\_(ツ)\_/¯

## Runtime

- [ ] Add `pkg-config` file
- [ ] Consolidate `ham_net_socket` and `ham_net_connection` or make `ham_net_socket` listen only
- [ ] Create shape functions `ham_shape_calculate_bounding_box`, `ham_shape_calculate_area` and `ham_shape_calculate_volume`
- [ ] Create ray type (`ham_ray`) and ray-shape intersection functions

## Plugins

- [x] Create `ham-renderer-gl` and/or `ham-renderer-gles` (Created `ham-renderer-gl`)

### `ham-net-gns`

- [ ] Use the source from `ham-net-steam`

### `ham-net-steam`

- [ ] Make source compatible with GameNetworkingSockets where applicable
- [ ] Add more Steamworks interfaces and rename to `ham-steam`

### `ham-renderer-vulkan`

- [ ] Wire up uniform buffers
- [ ] Add render passes for GBO, lighting and post-processing
- [ ] Render something more exciting than a quad displaying UVs

## Engine

### Runtime

- [ ] Add `pkg-config` file
- [ ] Create physics plugin interface (`ham_physics`)

- [ ] Make `ham_engine` fully self-contained and remove "engine plugin" concept
  - [ ] Use `ham_engine_options` or similarly named struct in `ham_engine_create` instead of `argc` and `argv`
  - [ ] Make `ham_engine_subsystem` the primary way to construct the engine

- [ ] Have `ham_engine` create a listen `ham_net_socket` on creation

- [ ] Have `ham_world` do octree space-partitioning (see `ham/octree.h`)
  - [ ] Have `ham_world` track the positions of objects as they move and shift their position in the tree

### Editor

- [ ] Add basic main window layout

- [ ] Integrate CMake as a library or externally called process

- [ ] Finish project template `CMakeLists.txt`
  - [ ] Generate build directory and `compile_commands.json`

- [ ] Create a project settings window

- [ ] Create an editor/engine settings window
