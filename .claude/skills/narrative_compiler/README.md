# Narrative Compiler Skill (Three-Layer Architecture)

A Claude Code skill for converting natural language narrative descriptions into a three-layer structured format: Semantic → Constraint → Timing.

## Architecture

This skill implements a three-layer architecture to avoid confusion:

1. **Semantic Layer**: What happens (goal, participants, atoms)
2. **Constraint Layer**: Relationships between actions (ordering, sync, interrupt)
3. **Timing Layer**: When and how long (duration sources, tempo modifiers)

## Installation

This skill is automatically loaded when you open the project in Claude Code.

## Usage

Start your message with `我是导演策划：` followed by your narrative description:

```
我是导演策划：Hanako 请 V 坐下，同时自顾自喝一口酒
```

The skill will automatically:
1. Parse your narrative description
2. Generate three-layer structured JSON
3. Save the JSON to `Narratives/narrative_{timestamp}.json`

## Output Format

```json
{
  "semantic": {
    "goal": "narrative_goal",
    "participants": ["character_list"],
    "atoms": [...]
  },
  "constraints": {
    "ordering_constraints": [...],
    "sync_constraints": [...],
    "interrupt_constraints": [...]
  },
  "timing": {
    "duration_sources": [...],
    "tempo_modifiers": {...}
  }
}
```

## Key Features

### Semantic Layer
- **goal**: Narrative objective (short English phrase)
- **participants**: All characters and objects involved
- **atoms**: Atomic actions (Speak, MoveTo, Gesture, LookAt, Wait, Signal)

### Constraint Layer
- **ordering_constraints**: Sequential relationships
  - `finish_to_start`: A finishes before B starts
  - `start_to_start`: B can start after A starts
  - `join`: C starts after both A and B finish
  - `join_after_stable`: C starts after A and B are stable
  - `parallel`: A and B execute in parallel

- **sync_constraints**: Synchronization conditions
  - `wait_all`: Wait for all atoms to complete
  - `wait_stable`: Wait for stable state

- **interrupt_constraints**: Interruption handling

### Timing Layer
- **duration_sources**: Duration estimation for each atom
  - `resource`: From resource (e.g., voice file duration)
  - `animation`: From animation
  - `estimated`: Estimated duration
  - `fixed`: Fixed duration

- **tempo_modifiers**: Rhythm and pacing
  - `style`: fast, medium, slow, slow_deliberate, urgent
  - `pause_scale`: Pause duration multiplier (0.5-2.0)
  - `overlap_preference`: high, medium, low, none

## Examples

See [examples.md](examples.md) for detailed test cases.

## Files

- `skill.md` - Main skill instructions (three-layer architecture)
- `skill.json` - Skill metadata and configuration
- `examples.md` - Test cases with three-layer examples
- `README.md` - This file

## Version

2.0.0 (Three-Layer Architecture)

## Changes from v1.0.0

- Separated semantic, constraint, and timing layers
- Added explicit constraint types (no more ambiguous "then/while")
- Added duration source modes
- Added tempo modifiers with detailed parameters
- Improved clarity and executability
