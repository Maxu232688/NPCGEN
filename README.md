# NPC Placement Tool

A drop-in Unreal Engine 5.7+ editor plugin for placing NPCs in levels via PIE (Play In Editor).

## Features

- Walk/fly around your level as a player character and place NPCs with hotkeys
- Configure NPC types (mesh, animation, scale, etc.) via DataAsset
- Auto-creates default configuration on first use — no manual setup
- PIE session changes are diffed against the original level, with an Apply/Discard dialog
- Top-down mode for box-selecting and batch-operating placed NPCs
- Rotate, undo, and delete placed NPCs in real time

## Supported Engines

- Unreal Engine 5.7+

## Installation

1. Copy the `NPCGEN` folder into your project's `Plugins/` directory
2. Launch your project — the plugin loads automatically

## First-Time Setup

### 1. Create a Character Blueprint

| Step | Value |
|------|-------|
| Path | `Content/DefaultNPCs/` |
| File | `BP_NPCPlacementCharacter` |
| Parent Class | `NPCPlacementCharacter` |
| Mesh → Skeletal Mesh | Any skeletal mesh (e.g. `SKM_Manny`) |
| Mesh → Anim Class | Any animation blueprint (e.g. `ABP_Manny_C`) |

### 2. Create a GameMode Blueprint

| Step | Value |
|------|-------|
| Path | `Content/DefaultNPCs/` |
| File | `BP_NPCGameMode` |
| Parent Class | `Game Mode Base` |
| Default Pawn Class | `BP_NPCPlacementCharacter` |

### 3. Set Default GameMode

**Edit → Project Settings → Maps & Modes → Default GameMode** → `BP_NPCGameMode`

## Usage

Click the **"NPC Placement"** toolbar button. The plugin auto-creates `DA_DefaultNPCConfig` at `/Game/DefaultNPCs/` with a default NPC entry. Edit it to add your own NPC types.

### Controls (in PIE)

| Action | Key |
|--------|-----|
| Move | W / A / S / D |
| Look | Mouse |
| Select NPC type | 1 – 9 |
| Place NPC | Same number key again |
| Toggle UI mode | Alt |
| Undo placement | Z |
| Rotate NPC | Q / E |
| Top-down mode | X |
| Delete selected | Delete |
| Free camera | F |
| Zoom | Scroll wheel |

## Modules

| Module | Type | Platform |
|--------|------|----------|
| `NPCPlacementTool` | Runtime | Game |
| `NPCPlacementToolEditor` | Editor | Editor-only |
