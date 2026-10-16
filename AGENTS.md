# ColumbusEngine agent instructions

These rules apply to new and modified engine code, including shaders. Explicit user instructions take precedence. Keep changes within the requested scope; do not use a feature or cleanup task to reformat unrelated code or repair unrelated systems. Existing code is inconsistent in places: follow the rules below rather than copying an old exception to them.

## Readability and formatting

- Optimize for clarity, logical grouping, and the visual shape of the code. There is **no fixed 80-, 100-, or 120-character line limit**. Prefer a readable long line over an awkward wrap. Do not run a broad formatter or alignment script over existing files.
- Use Allman braces for functions and control flow. Use braces for new control-flow blocks, especially long conditions and nested logic. Write `if (...)` and `for (...)`, with spaces around operators and after commas.
- Match the surrounding indentation: C++ generally uses tabs; shader code may use spaces. Preserve existing line endings and avoid whitespace churn.
- Use descriptive PascalCase names for types, functions, members, parameters, and ordinary locals, following the nearby subsystem. Use lowercase `i`, `j`, etc. for simple loop indices, never capital `I`. Descriptive indices such as `ViewIndex` are also appropriate.
- Separate declarations, calculations, control flow, and side effects into meaningful blocks with blank lines. Put blank lines around an `if` or loop mixed into a calculation block. Keep shader attributes such as `[unroll]` and `[loop]` immediately above their loop, with the loop block visually separated from surrounding work.
- Align variable-name starts and equals signs **within small logical groups** when this improves readability. Apply the same judgment to assignments, designated initializers, reflected field macros, and new shader code. Do not pad a whole function or struct to its longest name, or combine unrelated groups just to align them. Blank lines are often better than more spaces.

```cpp
const Vector3 Center = Bounds.CalcCenter();
const Vector3 Extent = Bounds.CalcSize() * 0.5f;

Vector3 CenterWS = (InverseView * Vector4(Center, 1)).XYZ();
Vector3 CenterLS = (LightRotation * Vector4(CenterWS, 1)).XYZ();

const float TexelSize = 2.0f * Radius / Resolution;
```

For mixed types in one logical group, align both the name and initializer where useful:

```cpp
u32   Resolution = 1024;
float DepthBias  = 0.0001f;
float NormalBias = 0.01f;
```

Reflection fields should also form readable columns within their logical group:

```cpp
CREFLECT_STRUCT_FIELD(bool,  RayTracedShadows, "")
CREFLECT_STRUCT_FIELD(int,   ShadowResolution, "")
CREFLECT_STRUCT_FIELD(float, ShadowDepthBias,  "")
```

- Keep straightforward initializers, calls, and ternaries on one line, even when long. This includes simple `std::span` selections and calls such as `PushConstantsGraphics`. Do not put only the variable and `=` on one line and spread a short ternary across the next two. Use an explicit `if` when a selection needs explanation or several steps.
- When a call genuinely benefits from multiple lines, break at logical argument boundaries and put the closing `);` on its own line. Keep parallel expressions symmetrical: for example, each component of a transformed extent should occupy one complete row; do not wrap only the last component.
- Give important operations such as `Graph.AddPass` a blank line before them. Keep lambda bodies readable and extract substantial work into named helpers.

## Functions and placement

- Keep functions focused. Separate orchestration from allocation, view/matrix preparation, culling, upload, and drawing. Extract meaningful helpers instead of leaving long, deeply nested functions. Do not create trivial forwarding helpers or arbitrary abstractions just to meet a line count.
- Prefer early returns and named `const bool` conditions for compound decisions. Preserve short-circuit safety: check that an iterator or pointer is valid before deriving conditions from its contents.
- Separate functions with blank lines. In a file containing several responsibilities, group related helpers and entry points into named sections using the established separator style:

```cpp
///////////////////////////////////////////////////////////////////////////////////////////////
// Shadow view preparation
```

- Search for an existing implementation before adding a utility. For example, reuse `Math::UpperPowerOf2` from `Math/MathUtil.h` instead of introducing another power-of-two helper. Check its supported inputs and semantics first.
- Put behavior on the type that owns it when appropriate: validity and transformed/deformed bounds belong on `Box` (`IsValid`, `CalcTransformedBox`, `CalcDeformedBox`). Cross-type geometry operations such as box/sphere intersection belong in `Math/Geometry.h/.cpp`. Extend the existing `Frustum` rather than adding a competing shadow-specific frustum implementation.
- Keep renderer-specific allocators and infrastructure under `Graphics`/`Graphics/Core`, not engine `Core`. Keep a helper local to its consumer when it has no useful shared role.
- Use `struct` for records and public state aggregates; use `class` where encapsulation and an invariant-bearing interface are useful. Avoid unnecessary layers, duplicate representations, and separate GPU records/buffers without a concrete ownership or indexing need.

## Types, errors, and ownership

- Use engine integer aliases from `Core/Types.h`, such as `u32`, `u64`, and `i32`, for engine data. Use `size_t` where appropriate for container sizes. Use fixed-width types in CPU/GPU interfaces and make narrowing conversions intentional.
- **Do not introduce `throw`, `try`, or `catch`. The engine does not use exceptions.** Use `Log` for diagnostics and `COLUMBUS_ASSERT` / `COLUMBUS_ASSERT_MESSAGE` for programmer invariants. Add a safe return or cleanup path where execution could continue; an assertion does not justify a later invalid dereference or out-of-range write.
- Prefer `const` for values that do not change, initialize new state explicitly, and include the headers that provide the types/functions being used. Avoid relying on accidental transitive includes or adding unused includes.
- Follow the subsystem's ownership model and existing `UPtr`/`SPtr` aliases where appropriate. A graphics wrapper's lifetime does not necessarily destroy the underlying GPU resource: follow the existing device destruction and synchronization path.
- Keep persistent resources with their owning scene/context/system. Make resize, scene replacement, removal, and teardown paths explicit. Balance profiling memory/count increments and decrements with actual resource lifetime.
- Use stable handles, including their generation, for identity retained across frames. Dense array indices are frame-local references and can change after removal or compaction.

## Renderer and shader work

- Use the render graph and existing shader parameter/binding abstractions. Declare actual resource reads/writes and pipeline stages so barriers and lifetimes remain correct. Do not hide resources used by a pass from its dependencies.
- Capture deferred pass data with a lifetime that extends through execution. Do not capture references to temporary preparation data. Ensure names passed from stack buffers are copied by the receiving API before the buffer expires.
- Keep C++ and HLSL representations synchronized: field order, widths, alignment/padding, flags, descriptor capacities, and array indexing must agree. Use size/layout assertions where useful. Provide valid fallback bindings where the descriptor interface requires them, including frames with zero active items.
- Preserve the engine's conventions documented in `Math/Matrix.h`: row-major matrices, column vectors, `Matrix * Vector`, and `Projection * View * Model`; Y up and Z backwards; reverse depth in `[0, 1]` with near at 1, sky at 0, and `GEqual`; screen UV origin at top left. Apply projection/viewport adjustments at the established boundary rather than scattering extra flips or transposes.
- Keep authored settings separate from per-frame render proxies. Runtime overrides should alter the proxy, not rewrite serialized authoring state.
- For new author-facing properties, update defaults, reflection, editor UI/metadata, serialization, and render-proxy upload together as applicable. Preserve sensible defaults for older assets and validate ranges at the appropriate boundary. Check the existing `CREFLECT` machinery before adding custom serialization or UI code.
- Profile substantial new work: CPU preparation, GPU passes, and persistent GPU memory allocations should have appropriate markers/counters. Prefer existing facilities in `Profiling/Profiling.h` and render-graph GPU markers over ad hoc timing.
- Avoid string concatenation and `std::to_string` in per-frame/hot loops. Format short labels with `snprintf` into a suitably sized fixed buffer. Reuse temporary storage and reserve known capacities when useful; do not add speculative caches or performance machinery outside the task.
- Support performance claims with measurements. Distinguish CPU preparation, GPU pass time, and total frame time, and compare equivalent scenes/settings. Keep rendering correctness checks alongside optimization measurements.

## Build and verification

- Inspect the actual CMake configuration and output paths before building. This engine is also built as a library by a containing game/editor project; do not assume `ProjectFiles` lives inside the engine repository.
- Build the editor and shaders together using the configured CMake targets. On the current Windows game workspace, the relevant commands run from the game project, for example:

```powershell
cmake --build ProjectFiles --config RelWithDebInfo --target TestGameEditor ShadowMapTests -- /m
Binaries/RelWithDebInfo/ShadowMapTests.exe
```

- Build configurations sequentially when they share shader output/deployment paths. Do not run competing Debug and RelWithDebInfo shader builds against the same output.
- Run checks appropriate to the change. Reuse existing tests; extend them for meaningful invariants and regressions rather than adding tests that merely mirror implementation. Documentation-only or whitespace-only edits do not require a full rendering suite.
- Use the editor CLI for repeatable smoke tests, render captures, and profiling when available. In the current sibling `Game_1` project, `Tools/TestShadowMapsCLI.py` and `Tools/TestRenderingCLI.py` exercise the relevant integration paths; inspect their arguments and use the executable built with the matching shaders.
- For renderer refactors, compare representative output, including masked and animated geometry when affected. For optimizations, check parity and report the measured result without treating noise as an improvement.
- Keep generated builds, captures, temporary scripts, and profiling output out of source changes. Check the final diff for whitespace errors, unrelated changes, stale includes/callers, and accidental changes to CPU/GPU contracts. Preserve other work already present in the checkout.
