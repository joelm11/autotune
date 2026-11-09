# June Plugin

From the Jaboratory.

Thinking of how to set the key for the auto tune.

- Dropdown to select key
- Dropdown to select octave

# Key selection

- Initially support C, G, D, A, E selectable via dropdown
- Pitch detect needs to find the correct octave
- Pitch detect detects the note
- Pitch correct finds the difference between the note and the nearest note in the scale
- That difference becomes the shift factor

## Synchronization

Synchronization requirements

- UI issues singular change that processor needs to read once.
- Processors are pushing updates constantly to UI which only wants to read the most recent update.
