# PBR Showcase Sphere Scene

## Purpose

This scene is a visual checkpoint for the current PBR foundation. It is not a new renderer feature branch; it is a compact showcase that exercises the systems already implemented.

Run it with:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes showcase-spheres
```

The capture is written to:

```text
out/pbr_showcase_spheres_verification.ppm
```

## What It Demonstrates

The scene contains two layers:

- A top row of larger PBR spheres using existing repository texture assets.
- A lower material grid showing metallic and roughness variation.

The top row demonstrates:

- Albedo textured dielectric spheres: Earth and Mars textures.
- Texture-set material binding: diffuse, specular-as-metallic probe, roughness, AO, and normal map from `fbx/bag`.
- Metallic low-roughness material response.
- Normal-mapped glossy material response.
- Emissive material plus bloom contribution using the existing sun texture.

The render path demonstrates:

- PBR material binding.
- Tangent-space normal map path.
- Deferred PBR G-buffer.
- Deferred lighting.
- Tiled point-light culling.
- 8-light pressure rig.
- PBR shadow atlas.
- Procedural IBL precompute.
- Verification capture and renderer statistics.

## Verification Contract

The mode is available through:

```text
--verify-pbr-showcase-spheres
```

`tools/verify_pbr.ps1` exposes it as:

```text
-Modes showcase-spheres
```

The verification checks:

- The process exits successfully.
- The capture is a valid PPM.
- At least 6 showcase PBR spheres are present.
- The PBR deferred lighting path draws.
- The tiled light grid is bound and reduces the point-light loop.
- The 8-point-light pressure rig is active.

## Resource Policy

No new external texture download is required for this scene. It uses the existing local assets under:

- `Texture/solar system`
- `Texture/normal`
- `fbx/bag`

This avoids adding external asset licensing risk while still showing the current PBR material and texture pipeline.

